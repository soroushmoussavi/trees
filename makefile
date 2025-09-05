CC = gcc
CFLAGS = -std=c99 -Wall
LDFLAGS = -ledit -lm
SRC = src/trees.c src/mpc.c src/utils.c
OBJ = $(SRC:.c=.o)
TARGET = trees

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ) $(TARGET)