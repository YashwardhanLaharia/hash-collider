#ifndef PDF_IO_H
#define PDF_IO_H

#include <stddef.h>
#include <stdint.h>

#define NONCE_OFFSET 16
#define NONCE_LENGTH 16
#define STUDENT_ID_OFFSET 45
#define STUDENT_ID_LENGTH 8
#define MIN_PDF_HEADER_LENGTH (STUDENT_ID_OFFSET + STUDENT_ID_LENGTH + 1)

unsigned char *load_file(const char *path, size_t *length);
int write_file(const char *path, const unsigned char *data, size_t length);
int valid_pdf_layout(const unsigned char *pdf, size_t length);
void set_nonce(unsigned char *pdf, uint64_t nonce);
void set_student_number(unsigned char *pdf, const char *student_id);

#endif
