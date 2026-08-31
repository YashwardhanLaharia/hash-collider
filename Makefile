CC      = gcc
CFLAGS  = -O3 -std=c11 -Wall -Wextra -pedantic -fopenmp
LDFLAGS = -fopenmp

TARGET = collider
OBJECTS = src/collider.o src/toy_hash.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

src/collider.o: src/collider.c src/toy_hash.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/toy_hash.o: src/toy_hash.c src/toy_hash.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)
