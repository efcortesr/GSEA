# GSEA — Guía rápida de uso

## Introducción

Este repositorio contiene `gsea`, una utilidad para comprimir (RLE2) y encriptar (Vigenère) archivos y directorios.

## Principales funciones

- `-c` / `-d` — Comprimir / Descomprimir
- `-e` / `-u` — Encriptar / Desencriptar
- `-ce` — Comprimir y encriptar en un solo paso
- `-ud` — Desencriptar y descomprimir en un solo paso

---

## Comandos básicos

### Compilación

```bash
make clean && make
```

---

## Operaciones de compresión

### Comprimir un único archivo

```bash
./gsea -c -i examples/input.txt -o examples/input.rle
```

### Descomprimir un único archivo

```bash
./gsea -d -i examples/input.rle -o examples/input_restored.txt
```

### Comprimir un directorio

```bash
./gsea -c -i examples/multi -o examples/multi_out
```

### Descomprimir un directorio

```bash
./gsea -d -i examples/multi_out -o examples/multi_restored
```

---

## Operaciones de encriptación

### Encriptar un archivo

```bash
./gsea -e -i examples/input.txt -o examples/input.enc -k "miclave"
```

### Desencriptar un archivo

```bash
./gsea -u -i examples/input.enc -o examples/input.dec -k "miclave"
```

### Encriptar un directorio

```bash
./gsea -e -i examples/test_crypto -o examples/test_crypto_enc -k "miclave"
```

### Desencriptar un directorio

```bash
./gsea -u -i examples/test_crypto_enc -o examples/test_crypto_dec -k "miclave"
```

---

## Operaciones combinadas

### Comprimir y encriptar en un solo paso (archivo)

```bash
./gsea -ce -i examples/audio.mp3 -o examples/audio.enc -k "miclave"
```

### Desencriptar y descomprimir en un solo paso (archivo)

```bash
./gsea -ud -i examples/input.enc -o examples/input.dec -k "miclave"
```

### Comprimir y encriptar en un solo paso (directorio)

```bash
./gsea -ce -i examples/pruebas -o examples/pruebas_enc -k "miclave"
```

### Desencriptar y descomprimir en un solo paso (directorio)

```bash
./gsea -ud -i examples/pruebas_enc -o examples/pruebas_dec -k "miclave"
```

---

## Verificación y utilidades

### Comparar archivos

```bash
diff examples/input.txt examples/input_restored.txt
```

### Comparar directorios completos

```bash
diff -r examples/test_crypto examples/test_crypto_dec
diff -r examples/pruebas examples/pruebas_dec
```

### Medir tiempo de ejecución

```bash
time ./gsea -c -i examples/input.txt -o examples/input.rle
time ./gsea -c -i examples/multi -o examples/multi_out
time ./gsea -e -i examples/test_crypto -o examples/test_crypto_enc -k "miclave"
time ./gsea -u -i examples/test_crypto_enc -o examples/test_crypto_dec -k "miclave"
```

### Limpiar archivos temporales

```bash
rm -f examples/*.rle examples/*.enc examples/*.dec
rm -rf examples/*_enc examples/*_dec
```

---

## Procesamiento de archivos grandes

### Comprimir, luego encriptar

```bash
./gsea -c -i examples/pruebas/eficiente1.bin -o examples/tmp.rle
./gsea -e -i examples/tmp.rle -o examples/eficiente1.enc -k "miclave"
```

### Desencriptar, luego descomprimir

```bash
./gsea -u -i examples/eficiente1.enc -o examples/tmp.rle -k "miclave"
./gsea -d -i examples/tmp.rle -o examples/eficiente1.dec
```

### Procesar varios archivos binarios en un directorio

```bash
./gsea -ce -i examples/pruebas -o examples/pruebas_enc -k "miclave"
./gsea -ud -i examples/pruebas_enc -o examples/pruebas_dec -k "miclave"
diff -r examples/pruebas examples/pruebas_dec
```

---

## Suite de pruebas

### Prueba 1: Directorio de pruebas general

```bash
./gsea -ce -i examples/pruebas -o examples/pruebas_enc -k "miclave"
./gsea -ud -i examples/pruebas_enc -o examples/pruebas_dec -k "miclave"
```

### Prueba 2: Directorio de Archivos de 2GB

```bash
./gsea -ce -i examples/2gb_files -o examples/2gb_enc -k "miclave"
./gsea -ud -i examples/2gb_enc -o examples/2gb_dec -k "miclave"
```

### Prueba 3: Archivo binario de 1GB

```bash
./gsea -ce -i examples/1gb.bin -o examples/1gb.rle -k "miclave"
./gsea -ud -i examples/1gb.rle -o examples/1gb_restored.bin -k "miclave"
```

### Prueba 4: Archivo óptimo RLE de 1GB

```bash
./gsea -ce -i examples/1gb_rle_optimal.bin -o examples/1gb_optimal.rle -k "miclave"
./gsea -ud -i examples/1gb_optimal.rle -o examples/1gb_optimal_restored.bin -k "miclave"
```

### Prueba 5: Directorio de pruebas

```bash
./gsea -ce -i examples/pruebas -o examples/pruebas_enc -k "miclave"
./gsea -ud -i examples/pruebas_enc -o examples/pruebas_dec -k "miclave"
```

### Prueba 6: Imagen BMP

```bash
./gsea -ce -i examples/leon.bmp -o examples/leon.rle -k "miclave"
./gsea -ud -i examples/leon.rle -o examples/leon_restored.bmp -k "miclave"
```

### Prueba 7: Archivo de audio MP3

```bash
./gsea -ce -i examples/audio.mp3 -o examples/audio.rle -k "miclave"
./gsea -ud -i examples/audio.rle -o examples/audio_restored.mp3 -k "miclave"
```
