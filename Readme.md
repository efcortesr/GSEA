# GSEA — Guía rápida de uso

Breve introducción
-------------------

Este repositorio contiene `gsea`, una utilidad para comprimir (RLE2) y encriptar (Vigenère) archivos y directorios.

Principales funciones:

- `-c` / `-d`: comprimir / descomprimir
- `-e` / `-u`: encriptar / desencriptar
- `-ce`: comprimir y encriptar en un solo paso
- `-ud`: desencriptar y descomprimir en un solo paso

Ejemplos rápidos y comandos de prueba:

Comandos para pruebas:

// limpiar y compilar
make clean && make

// comprimir un unico archivo
./gsea -c -i examples/input.txt -o examples/input.rle

// descomprimir un unico archivo
./gsea -d -i examples/input.rle -o examples/input_restored.txt

// comprimir un directorio de multiples archivos
./gsea -c -i examples/multi -o examples/multi_out

// descomprimir un directorio de multiples archivos
./gsea -d -i examples/multi_out -o examples/multi_restored

// comparar 2 archivos
diff examples/input.txt examples/input_restored.txt

// medir tiempo
time ./gsea -c -i examples/input.txt -o examples/input.rle

time ./gsea -c -i examples/multi -o examples/multi_out

// encriptar un archivo
./gsea -e -i examples/input.txt -o examples/input.enc -k "miclave"

// desencriptar un archivo
./gsea -u -i examples/input.enc -o examples/input.dec -k "miclave"

// comprimir y encriptar en un solo paso
./gsea -ce -i examples/audio.mp3 -o examples/audio.enc -k "miclave"

// desencriptar y descomprimir en un solo paso
./gsea -ud -i examples/input.enc -o examples/input.dec -k "miclave"

// encriptar un directorio
./gsea -e -i examples/test_crypto -o examples/test_crypto_enc -k "miclave"

// desencriptar un directorio
./gsea -u -i examples/test_crypto_enc -o examples/test_crypto_dec -k "miclave"

// Verificar que los archivos son idénticos
diff -r examples/test_crypto examples/test_crypto_dec

// Comprimir y encriptar un directorio en un solo paso
./gsea -ce -i examples/pruebas -o examples/pruebas_enc -k "miclave"

// Desencriptar y descomprimir un directorio en un solo paso
./gsea -ud -i examples/pruebas_enc -o examples/pruebas_dec -k "miclave"

// Medir el tiempo de encriptación de un directorio
time ./gsea -e -i examples/test_crypto -o examples/test_crypto_enc -k "miclave"

// Medir el tiempo de desencriptación de un directorio
time ./gsea -u -i examples/test_crypto_enc -o examples/test_crypto_dec -k "miclave"

// Comprimir, luego encriptar archivos grandes
./gsea -c -i examples/pruebas/eficiente1.bin -o examples/tmp.rle
./gsea -e -i examples/tmp.rle -o examples/eficiente1.enc -k "miclave"

// Desencriptar, luego descomprimir archivos grandes
./gsea -u -i examples/eficiente1.enc -o examples/tmp.rle -k "miclave"
./gsea -d -i examples/tmp.rle -o examples/eficiente1.dec

// Procesar varios archivos binarios en un directorio
./gsea -ce -i examples/pruebas -o examples/pruebas_enc -k "miclave"
./gsea -ud -i examples/pruebas_enc -o examples/pruebas_dec -k "miclave"

// Verificar diferencias después de procesar
diff -r examples/pruebas examples/pruebas_dec

// Limpiar archivos temporales después de procesar
rm -f examples/*.rle examples/*.enc examples/*.dec
rm -rf examples/*_enc examples/*_dec

// PRUEBAS
// 1.
./gsea -ce -i examples/pruebas -o examples/pruebas_enc -k "miclave"
./gsea -ud -i examples/pruebas_enc -o examples/pruebas_dec -k "miclave"

// 2.
./gsea -ce -i examples/2gb_files -o examples/2gb_enc -k "miclave"
./gsea -ud -i examples/2gb_enc -o examples/2gb_dec -k "miclave"

// 3.
./gsea -ce -i examples/1gb.bin -o examples/1gb.rle -k "miclave"
./gsea -ud -i examples/1gb.rle -o examples/1gb_restored.bin -k "miclave"

// 4.
./gsea -ce -i examples/1gb_rle_optimal.bin -o examples/1gb_optimal.rle -k "miclave"
./gsea -ud -i examples/1gb_optimal.rle -o examples/1gb_optimal_restored.bin -k "miclave"

//  5.
./gsea -ce -i examples/pruebas -o examples/pruebas_enc -k "miclave"
./gsea -ud -i examples/pruebas_enc -o examples/pruebas_dec -k "miclave"

// 6.

./gsea -ce -i examples/leon.bmp -o examples/leon.rle -k "miclave"
./gsea -ud -i examples/leon.rle -o examples/leon_restored.bmp -k "miclave"

// 7.

./gsea -ce -i examples/audio.mp3 -o examples/audio.rle -k "miclave"
./gsea -ud -i examples/audio.rle -o examples/audio_restored.mp3 -k "miclave"
