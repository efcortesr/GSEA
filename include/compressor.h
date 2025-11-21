#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include <stddef.h>
#include <stdint.h>

/*
 * RLE2 (PackBits + threshold + RAW/RLE block)
 * RLE1 has been completely removed.
 */

int rle2_compress_stream(int fd_in, int fd_out);
int rle2_decompress_stream(int fd_in, int fd_out);

/* Tunables */
#ifndef RLE2_BLOCK_SIZE
#define RLE2_BLOCK_SIZE (64 * 1024) /* 64 KiB blocks */
#endif

#ifndef RLE2_RUN_THRESHOLD
#define RLE2_RUN_THRESHOLD 3 /* min run length to emit RUN */
#endif

#endif /* COMPRESSOR_H */
