#include "compressor.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* =======================
 *  Shared small helpers
 * ======================= */

static int write_all(int fd, const uint8_t *buf, size_t n)
{
    size_t off = 0;
    while (off < n)
    {
        ssize_t w = write(fd, buf + off, n - off);
        if (w < 0)
        {
            perror("write");
            return -1;
        }
        off += (size_t)w;
    }
    return 0;
}

static int read_all(int fd, uint8_t *buf, size_t n)
{
    size_t off = 0;
    while (off < n)
    {
        ssize_t r = read(fd, buf + off, n - off);
        if (r < 0)
        {
            perror("read");
            return -1;
        }
        if (r == 0)
            return 1; /* EOF before n bytes */
        off += (size_t)r;
    }
    return 0;
}

static void u32le_write(uint8_t out[4], uint32_t v)
{
    out[0] = (uint8_t)(v & 0xFF);
    out[1] = (uint8_t)((v >> 8) & 0xFF);
    out[2] = (uint8_t)((v >> 16) & 0xFF);
    out[3] = (uint8_t)((v >> 24) & 0xFF);
}

static uint32_t u32le_read(const uint8_t in[4])
{
    return (uint32_t)in[0] | ((uint32_t)in[1] << 8) | ((uint32_t)in[2] << 16) | ((uint32_t)in[3] << 24);
}

/* =======================
 *  RLE2
 *  Header: "RLE2\0\0\0\0"
 *  Stream: repeated blocks
 *    tag: 1 byte (0x00 RAW, 0x01 RLE)
 *    len: 4 bytes (LE) -> payload length
 *    payload: 'len' bytes
 *  RLE payload uses PackBits-like:
 *    control 0..127  -> (control+1) literals follow
 *    control 128..255-> ((control&0x7F)+1) repeats, followed by 1 value byte
 *  Run threshold: only emit RUN if run_len >= RLE2_RUN_THRESHOLD
 * ======================= */

static const uint8_t RLE2_MAGIC[8] = {'R', 'L', 'E', '2', 0, 0, 0, 0};

static size_t packbits_encode_threshold(const uint8_t *in, size_t n, uint8_t *out, int k_min_run)
{
    size_t i = 0, o = 0;

    while (i < n)
    {
        /* Try to find a run starting at i */
        size_t run = 1;
        while (i + run < n && in[i + run] == in[i] && run < 128)
            run++;

        if (run >= (size_t)k_min_run)
        {
            /* flush this run as RUN blocks of up to 128 */
            while (run > 0)
            {
                size_t chunk = (run > 128) ? 128 : run;
                uint8_t ctrl = 0x80 | (uint8_t)(chunk - 1); /* MSB=1, length-1 */
                out[o++] = ctrl;
                out[o++] = in[i];
                i += chunk;
                run -= chunk;

                /* If more of the same value continues, recompute for next loop */
                if (run == 0)
                {
                    size_t more = 0;
                    while (i + more < n && in[i + more] == in[i] && more < 128)
                        more++;
                    if (more >= (size_t)k_min_run)
                    {
                        run = more;
                    }
                }
            }
            continue;
        }

        /* Otherwise, accumulate a LITERAL packet up to 128 bytes,
           but stop before a long enough run would start. */
        size_t lit_start = i;
        size_t lit_len = 1; /* at least in[i] */

        while (i + lit_len < n && lit_len < 128)
        {
            /* peek if a run would start at i+lit_len */
            size_t r = 1;
            while (i + lit_len + r < n &&
                   in[i + lit_len + r] == in[i + lit_len] &&
                   r < 128)
            {
                r++;
            }
            if (r >= (size_t)k_min_run)
                break; /* stop literal before the run */
            lit_len++;
        }

        /* emit LITERAL packet: ctrl = (len-1) with MSB=0 */
        uint8_t ctrl = (uint8_t)(lit_len - 1);
        out[o++] = ctrl;
        memcpy(out + o, in + lit_start, lit_len);
        o += lit_len;
        i += lit_len;
    }
    return o;
}

static int packbits_decode(const uint8_t *in, size_t n, uint8_t *out, size_t *out_len)
{
    size_t i = 0, o = 0;
    while (i < n)
    {
        uint8_t ctrl = in[i++];
        if ((ctrl & 0x80) == 0)
        {
            /* LITERAL: len = ctrl+1 */
            size_t len = (size_t)ctrl + 1;
            if (i + len > n)
                return 1; /* truncated */
            memcpy(out + o, in + i, len);
            o += len;
            i += len;
        }
        else
        {
            /* RUN: len = (ctrl&0x7F)+1, then one value */
            size_t len = (size_t)(ctrl & 0x7F) + 1;
            if (i >= n)
                return 1; /* missing value */
            uint8_t val = in[i++];
            memset(out + o, val, len);
            o += len;
        }
    }
    *out_len = o;
    return 0;
}

int rle2_compress_stream(int fd_in, int fd_out)
{
    if (write_all(fd_out, RLE2_MAGIC, sizeof(RLE2_MAGIC)) != 0)
        return 1;

    uint8_t *inbuf = (uint8_t *)malloc(RLE2_BLOCK_SIZE);
    /* En el peor de los casos PackBits se expande ≈1/128, pero por seguridad asignamos 2× */
    uint8_t *rlebuf = (uint8_t *)malloc(RLE2_BLOCK_SIZE * 2);
    if (!inbuf || !rlebuf)
    {
        fprintf(stderr, "malloc failed\n");
        free(inbuf);
        free(rlebuf);
        return 1;
    }

    for (;;)
    {
        ssize_t r = read(fd_in, inbuf, RLE2_BLOCK_SIZE);
        if (r < 0)
        {
            perror("read");
            free(inbuf);
            free(rlebuf);
            return 2;
        }
        if (r == 0)
            break;

        size_t in_n = (size_t)r;

        /* Codificar usando PackBits con umbral */
        size_t enc_n = packbits_encode_threshold(inbuf, in_n, rlebuf, RLE2_RUN_THRESHOLD);

        /* Decidir bloque RAW o RLE */
        uint8_t tag;
        const uint8_t *payload;
        uint32_t paylen;
        if (enc_n >= in_n)
        {
            tag = 0x00;
            payload = inbuf;
            paylen = (uint32_t)in_n;
        }
        else
        {
            tag = 0x01;
            payload = rlebuf;
            paylen = (uint32_t)enc_n;
        }

        uint8_t header[5];
        header[0] = tag;
        u32le_write(header + 1, paylen);

        if (write_all(fd_out, header, sizeof(header)) != 0)
        {
            free(inbuf);
            free(rlebuf);
            return 3;
        }
        if (write_all(fd_out, payload, paylen) != 0)
        {
            free(inbuf);
            free(rlebuf);
            return 4;
        }
    }

    free(inbuf);
    free(rlebuf);
    return 0;
}

int rle2_decompress_stream(int fd_in, int fd_out)
{
    uint8_t hdr[8];
    if (read_all(fd_in, hdr, sizeof(hdr)) != 0)
    {
        fprintf(stderr, "Invalid or short header for RLE2.\n");
        return 1;
    }
    if (memcmp(hdr, RLE2_MAGIC, sizeof(hdr)) != 0)
    {
        fprintf(stderr, "Not an RLE2 file.\n");
        return 1;
    }

    /* Buffers for a block */
    uint8_t *inbuf = (uint8_t *)malloc(RLE2_BLOCK_SIZE * 2);  /* payload can be RLE, choose 2x */
    uint8_t *outbuf = (uint8_t *)malloc(RLE2_BLOCK_SIZE * 4); /* decompressed may expand; be generous */
    if (!inbuf || !outbuf)
    {
        fprintf(stderr, "malloc failed\n");
        free(inbuf);
        free(outbuf);
        return 1;
    }

    for (;;)
    {
        uint8_t blk_hdr[5];
        int rc = read_all(fd_in, blk_hdr, sizeof(blk_hdr));
        if (rc != 0)
        {
            if (rc == 1)
            { /* EOF cleanly */
                break;
            }
            /* error already printed */
            free(inbuf);
            free(outbuf);
            return 2;
        }

        uint8_t tag = blk_hdr[0];
        uint32_t paylen = u32le_read(&blk_hdr[1]);

        if (paylen == 0)
            continue; /* empty block */
        if (paylen > RLE2_BLOCK_SIZE * 2)
        {
            /* sanity guard; adjust if you expect bigger payloads */
            inbuf = (uint8_t *)realloc(inbuf, paylen);
            if (!inbuf)
            {
                fprintf(stderr, "realloc failed\n");
                free(outbuf);
                return 3;
            }
        }

        if (read_all(fd_in, inbuf, paylen) != 0)
        {
            free(inbuf);
            free(outbuf);
            return 4;
        }

        if (tag == 0x00)
        {
            /* RAW */
            if (write_all(fd_out, inbuf, paylen) != 0)
            {
                free(inbuf);
                free(outbuf);
                return 5;
            }
        }
        else if (tag == 0x01)
        {
            /* RLE payload */
            size_t out_len = 0;
            if (packbits_decode(inbuf, paylen, outbuf, &out_len) != 0)
            {
                fprintf(stderr, "Corrupted RLE2 block payload.\n");
                free(inbuf);
                free(outbuf);
                return 6;
            }
            if (write_all(fd_out, outbuf, out_len) != 0)
            {
                free(inbuf);
                free(outbuf);
                return 7;
            }
        }
        else
        {
            fprintf(stderr, "Unknown block tag: 0x%02X\n", tag);
            free(inbuf);
            free(outbuf);
            return 8;
        }
    }

    free(inbuf);
    free(outbuf);
    return 0;
}
