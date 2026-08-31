/*
 * Hash collision search skeleton.
 *
 * Build with:
 *     make
 *
 * Run with:
 *     ./collider file_a.pdf file_b.pdf [options]
 */

#include "toy_hash.h"

#include <errno.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NONCE_OFFSET 16
#define NONCE_LENGTH 16
#define STUDENT_ID_OFFSET 45
#define STUDENT_ID_LENGTH 8
#define MIN_PDF_HEADER_LENGTH (STUDENT_ID_OFFSET + STUDENT_ID_LENGTH + 1)

typedef struct {
    uint64_t nonce_a;
    uint64_t nonce_b;
} collision_solution;

static void print_usage(const char *program)
{
    fprintf(stderr,
            "Usage: %s <file-a.pdf> <file-b.pdf> [options]\n"
            "Options:\n"
            "  --student-id ID    eight-character student number\n"
            "  --threads N        number of OpenMP threads (default: 1)\n"
            "  -o FILE_A FILE_B   output paths for solved files\n",
            program);
}

static int parse_positive_int(const char *text, int *value)
{
    char *end = NULL;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed < 1 ||
        parsed > 2147483647L) {
        return 0;
    }

    *value = (int) parsed;
    return 1;
}

static int valid_student_id(const char *student_id)
{
    size_t i;

    if (strlen(student_id) != STUDENT_ID_LENGTH) {
        return 0;
    }

    for (i = 0; i < STUDENT_ID_LENGTH; ++i) {
        if (student_id[i] < '0' || student_id[i] > '9') {
            return 0;
        }
    }
    return 1;
}

static void set_nonce(unsigned char *pdf, uint64_t nonce)
{
    static const char hex[] = "0123456789abcdef";
    int i;

    for (i = NONCE_LENGTH - 1; i >= 0; --i) {
        pdf[NONCE_OFFSET + i] = (unsigned char) hex[nonce & 0xfU];
        nonce >>= 4;
    }
}

static void set_student_number(unsigned char *pdf, const char *student_id)
{
    memcpy(pdf + STUDENT_ID_OFFSET, student_id, STUDENT_ID_LENGTH);
}

static unsigned char *load_file(const char *path, size_t *length)
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

static int write_file(const char *path, const unsigned char *data, size_t length)
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

static int valid_pdf_layout(const unsigned char *pdf, size_t length)
{
    static const char expected_prefix[] = "%PDF-1.4\n%NONCE=";
    static const char expected_student_marker[] = "%STUDENT_ID=";

    return length >= MIN_PDF_HEADER_LENGTH &&
           memcmp(pdf, expected_prefix, sizeof(expected_prefix) - 1) == 0 &&
           memcmp(pdf + 33, expected_student_marker,
                  sizeof(expected_student_marker) - 1) == 0;
}

static int verify_collision(const unsigned char *file_a, size_t length_a,
                            const unsigned char *file_b, size_t length_b,
                            const char *student_id,
                            const collision_solution *solution,
                            uint64_t *hash_a, uint64_t *hash_b)
{
    unsigned char *copy_a = malloc(length_a);
    unsigned char *copy_b = malloc(length_b);
    int matches = 0;

    if ((copy_a == NULL && length_a != 0) || (copy_b == NULL && length_b != 0)) {
        fprintf(stderr, "Could not allocate buffers for collision verification\n");
        free(copy_a);
        free(copy_b);
        return 0;
    }

    memcpy(copy_a, file_a, length_a);
    memcpy(copy_b, file_b, length_b);
    set_nonce(copy_a, solution->nonce_a);
    set_nonce(copy_b, solution->nonce_b);
    set_student_number(copy_a, student_id);
    set_student_number(copy_b, student_id);

    *hash_a = toy_hash(copy_a, length_a);
    *hash_b = toy_hash(copy_b, length_b);
    matches = *hash_a == *hash_b;

    free(copy_a);
    free(copy_b);
    return matches;
}

/*
 * TODO: implement the serial and OpenMP birthday attack here.
 *
 * A typical implementation should:
 *   1. Generate nonce trials for file A and store hash -> nonce entries in an
 *      open-addressing table. Hashes are 48-bit values, so use uint64_t keys.
 *   2. Generate nonce trials for file B and probe the table for a matching
 *      hash. Return the two nonces as soon as a match is found.
 *   3. For the parallel version, avoid unsynchronised writes to a shared
 *      table. Possible designs include thread-local tables followed by a
 *      merge, or a partitioned table with synchronised insertion.
 *   4. Coordinate termination with an OpenMP shared flag and verify the
 *      candidate in the caller before writing output files.
 *
 * The expected birthday-attack scale is approximately sqrt(2^48) = 2^24
 * trials. The table size and trial limit should be made configurable once the
 * collision table is implemented.
 */
static int birthday_attack(const unsigned char *file_a, size_t length_a,
                           const unsigned char *file_b, size_t length_b,
                           int thread_count, collision_solution *solution)
{
    (void) file_a;
    (void) length_a;
    (void) file_b;
    (void) length_b;
    (void) thread_count;
    (void) solution;

    fprintf(stderr, "birthday_attack() is not implemented yet\n");
    return 0;
}

int main(int argc, char **argv)
{
    const char *input_a;
    const char *input_b;
    const char *student_id = "00000000";
    const char *output_a = "solved_a.pdf";
    const char *output_b = "solved_b.pdf";
    unsigned char *file_a;
    unsigned char *file_b;
    size_t length_a;
    size_t length_b;
    int thread_count = 1;
    int i;
    int found;
    double start_time;
    double elapsed;
    collision_solution solution;
    uint64_t hash_a;
    uint64_t hash_b;

    if (argc < 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    input_a = argv[1];
    input_b = argv[2];
    for (i = 3; i < argc; ++i) {
        if (strcmp(argv[i], "--student-id") == 0 && i + 1 < argc) {
            student_id = argv[++i];
        } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
            if (!parse_positive_int(argv[++i], &thread_count)) {
                fprintf(stderr, "Invalid thread count\n");
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "-o") == 0 && i + 2 < argc) {
            output_a = argv[++i];
            output_b = argv[++i];
        } else {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (!valid_student_id(student_id)) {
        fprintf(stderr, "Student ID must contain exactly eight digits\n");
        return EXIT_FAILURE;
    }

    file_a = load_file(input_a, &length_a);
    file_b = load_file(input_b, &length_b);
    if (file_a == NULL || file_b == NULL) {
        free(file_a);
        free(file_b);
        return EXIT_FAILURE;
    }
    if (!valid_pdf_layout(file_a, length_a) || !valid_pdf_layout(file_b, length_b)) {
        fprintf(stderr, "Input files do not have the expected PDF header layout\n");
        free(file_a);
        free(file_b);
        return EXIT_FAILURE;
    }

    omp_set_num_threads(thread_count);
    start_time = omp_get_wtime();
    found = birthday_attack(file_a, length_a, file_b, length_b, thread_count,
                            &solution);
    elapsed = omp_get_wtime() - start_time;

    if (!found) {
        fprintf(stderr, "No collision found. Search time: %.6f seconds\n", elapsed);
        free(file_a);
        free(file_b);
        return EXIT_FAILURE;
    }

    if (!verify_collision(file_a, length_a, file_b, length_b, student_id,
                          &solution, &hash_a, &hash_b)) {
        fprintf(stderr, "Candidate collision failed final verification\n");
        free(file_a);
        free(file_b);
        return EXIT_FAILURE;
    }

    set_nonce(file_a, solution.nonce_a);
    set_nonce(file_b, solution.nonce_b);
    set_student_number(file_a, student_id);
    set_student_number(file_b, student_id);
    if (!write_file(output_a, file_a, length_a) ||
        !write_file(output_b, file_b, length_b)) {
        free(file_a);
        free(file_b);
        return EXIT_FAILURE;
    }

    printf("nonce A: 0x%016llx\n", (unsigned long long) solution.nonce_a);
    printf("nonce B: 0x%016llx\n", (unsigned long long) solution.nonce_b);
    printf("hash A:  %012llx\n", (unsigned long long) hash_a);
    printf("hash B:  %012llx\n", (unsigned long long) hash_b);
    printf("search time: %.6f seconds\n", elapsed);

    free(file_a);
    free(file_b);
    return EXIT_SUCCESS;
}
