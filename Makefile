CC      = gcc
CFLAGS  = -O3 -std=c11 -Wall -Wextra -pedantic -fopenmp
LDFLAGS = -fopenmp

TARGET = collider
OBJECTS = src/main.o src/pdf_io.o src/table.o src/attack_serial.o \
	 src/attack_parallel.o src/toy_hash.o

.PHONY: all clean

all: $(TARGET)


$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

src/main.o: src/main.c src/attack_parallel.h src/attack_serial.h \
	src/pdf_io.h src/toy_hash.h src/collider_types.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/pdf_io.o: src/pdf_io.c src/pdf_io.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/table.o: src/table.c src/table.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/attack_serial.o: src/attack_serial.c src/attack_serial.h \
	src/collider_types.h src/table.h src/toy_hash.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/attack_parallel.o: src/attack_parallel.c src/attack_parallel.h \
	src/collider_types.h src/table.h src/toy_hash.h
	$(CC) $(CFLAGS) -c -o $@ $<

src/toy_hash.o: src/toy_hash.c src/toy_hash.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJECTS)
