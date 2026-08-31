#include "pdf_io.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char *load_file(const char *path, size_t *length)
{
    FILE *file;
    unsigned char *data;
    long file_size;
    size_t bytes_read;

    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open '%s': %s\n", path, strerror(errno));
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0 || (file_size = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Could not determine the size of '%s'\n", path);
        fclose(file);
        return NULL;
    }

    data = malloc((size_t) file_size);
    if (data == NULL && file_size != 0) {
        fprintf(stderr, "Could not allocate %ld bytes for '%s'\n", file_size,
                path);
        fclose(file);
        return NULL;
    }
    bytes_read = fread(data, 1, (size_t) file_size, file);
    if (bytes_read != (size_t) file_size || ferror(file)) {
        fprintf(stderr, "Could not read '%s'\n", path);
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *length = (size_t) file_size;
    return data;
}

int write_file(const char *path, const unsigned char *data, size_t length)
{
    FILE *file = fopen(path, "wb");

    if (file == NULL) {
        fprintf(stderr, "Could not create '%s': %s\n", path, strerror(errno));
        return 0;
    }
    if (fwrite(data, 1, length, file) != length || fclose(file) != 0) {
        fprintf(stderr, "Could not write '%s'\n", path);
        return 0;
    }
    return 1;
}

int valid_pdf_layout(const unsigned char *pdf, size_t length)
{
    static const char prefix[] = "%PDF-1.4\n%NONCE=";
    static const char student_marker[] = "%STUDENT_ID=";

    return length >= MIN_PDF_HEADER_LENGTH &&
           memcmp(pdf, prefix, sizeof(prefix) - 1) == 0 &&
           memcmp(pdf + 33, student_marker, sizeof(student_marker) - 1) == 0;
}

void set_nonce(unsigned char *pdf, uint64_t nonce)
{
    static const char hex[] = "0123456789abcdef";
    int i;

    for (i = NONCE_LENGTH - 1; i >= 0; --i) {
        pdf[NONCE_OFFSET + i] = (unsigned char) hex[nonce & 0xfU];
        nonce >>= 4;
    }
}

void set_student_number(unsigned char *pdf, const char *student_id)
{
    memcpy(pdf + STUDENT_ID_OFFSET, student_id, STUDENT_ID_LENGTH);
}
