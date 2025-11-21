GSEA — Guía Rápida de Uso
Introducción

Este repositorio contiene gsea, una utilidad diseñada para comprimir (RLE2) y encriptar (Vigenère) archivos y directorios.

Funciones principales
Función	Descripción
-c / -d	Comprimir / Descomprimir
-e / -u	Encriptar / Desencriptar
-ce	Comprimir y encriptar en un solo paso
-ud	Desencriptar y descomprimir en un solo paso
Compilación
make clean && make

Ejemplos de uso
Comprimir y descomprimir archivos
# Comprimir archivo
./gsea -c -i examples/input.txt -o examples/input.rle

# Descomprimir archivo
./gsea -d -i examples/input.rle -o examples/input_restored.txt

Comprimir y descomprimir directorios
# Comprimir directorio
./gsea -c -i examples/multi -o examples/multi_out

# Descomprimir directorio
./gsea -d -i examples/multi_out -o examples/multi_restored

Comparar archivos tras el proceso
diff examples/input.txt examples/input_restored.txt
diff -r examples/test_crypto examples/test_crypto_dec

Medir tiempos de ejecución
time ./gsea -c -i examples/input.txt -o examples/input.rle
time ./gsea -c -i examples/multi -o examples/multi_out
time ./gsea -e -i examples/test_crypto -o examples/test_crypto_enc -k "miclave"
time ./gsea -u -i examples/test_crypto_enc -o examples/test_crypto_dec -k "miclave"

Encriptación y desencriptación
Archivos individuales
# Encriptar
./gsea -e -i examples/input.txt -o examples/input.enc -k "miclave"

# Desencriptar
./gsea -u -i examples/input.enc -o examples/input.dec -k "miclave"

Procesos combinados
# Comprimir + Encriptar
./gsea -ce -i examples/audio.mp3 -o examples/audio.enc -k "miclave"

# Desencriptar + Descomprimir
./gsea -ud -i examples/audio.enc -o examples/audio.dec -k "miclave"

Encriptar/Desencriptar directorios
# Encriptar directorio
./gsea -e -i examples/test_crypto -o examples/test_crypto_enc -k "miclave"

# Desencriptar directorio
./gsea -u -i examples/test_crypto_enc -o examples/test_crypto_dec -k "miclave"

Procesamiento de archivos grandes
# Comprimir luego encriptar
./gsea -c -i examples/pruebas/eficiente1.bin -o examples/tmp.rle
./gsea -e -i examples/tmp.rle -o examples/eficiente1.enc -k "miclave"

# Desencriptar luego descomprimir
./gsea -u -i examples/eficiente1.enc -o examples/tmp.rle -k "miclave"
./gsea -d -i examples/tmp.rle -o examples/eficiente1.dec

Procesar múltiples archivos en un directorio
./gsea -ce -i examples/pruebas -o examples/pruebas_enc -k "miclave"
./gsea -ud -i examples/pruebas_enc -o examples/pruebas_dec -k "miclave"

# Verificar diferencias
diff -r examples/pruebas examples/pruebas_dec

Limpieza de archivos temporales
rm -f examples/*.rle examples/*.enc examples/*.dec
rm -rf examples/*_enc examples/*_dec

Pruebas
1. Directorio completo
./gsea -ce -i examples/pruebas -o examples/pruebas_enc -k "miclave"
./gsea -ud -i examples/pruebas_enc -o examples/pruebas_dec -k "miclave"

2. Directorio de archivos de 2 GB
./gsea -ce -i examples/2gb_files -o examples/2gb_enc -k "miclave"
./gsea -ud -i examples/2gb_enc -o examples/2gb_dec -k "miclave"

3. Archivo de 1 GB
./gsea -ce -i examples/1gb.bin -o examples/1gb.rle -k "miclave"
./gsea -ud -i examples/1gb.rle -o examples/1gb_restored.bin -k "miclave"

4. Archivo RLE optimizado
./gsea -ce -i examples/1gb_rle_optimal.bin -o examples/1gb_optimal.rle -k "miclave"
./gsea -ud -i examples/1gb_optimal.rle -o examples/1gb_optimal_restored.bin -k "miclave"

5. Directorio pequeño
./gsea -ce -i examples/pruebas -o examples/pruebas_enc -k "miclave"
./gsea -ud -i examples/pruebas_enc -o examples/pruebas_dec -k "miclave"

6. Imagen BMP
./gsea -ce -i examples/leon.bmp -o examples/leon.rle -k "miclave"
./gsea -ud -i examples/leon.rle -o examples/leon_restored.bmp -k "miclave"

7. Audio MP3
./gsea -ce -i examples/audio.mp3 -o examples/audio.rle -k "miclave"
./gsea -ud -i examples/audio.rle -o examples/audio_restored.mp3 -k "miclave"
