CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -pthread
LDFLAGS = -pthread
SRC = src/main.c src/cli.c src/file_manager.c src/compressor.c src/encryptor.c
OBJ = $(SRC:.c=.o)
TARGET = gsea

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)
