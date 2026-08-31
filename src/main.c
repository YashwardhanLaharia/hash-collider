#include "attack_parallel.h"
#include "attack_serial.h"
#include "pdf_io.h"
#include "toy_hash.h"

#include <errno.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define STUDENT_ID "24295462"

static void print_usage(const char *program)
{
    fprintf(stderr,
            "Usage: %s <file-a.pdf> <file-b.pdf> [options]\n"
            "Options:\n"
            "  --threads N        number of OpenMP threads (default: 1)\n"
            "  --progress         show attack progress (disabled by default)\n",
            program);
}

static char *make_solved_path(const char *input)
{
    const char *base = strrchr(input, '/');
    size_t length;
    char *path;

    base = (base == NULL) ? input : base + 1;
    length = strlen(base) + sizeof("solved/solved_");
    path = malloc(length);
    if (path == NULL) {
        return NULL;
    }
    snprintf(path, length, "solved/solved_%s", base);
    return path;
}

static int ensure_solved_directory(void)
{
    if (mkdir("solved", 0755) != 0 && errno != EEXIST) {
        fprintf(stderr, "Could not create solved output directory: %s\n",
                strerror(errno));
        return 0;
    }
    return 1;
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

static int verify_collision(const unsigned char *file_a, size_t length_a,
                            const unsigned char *file_b, size_t length_b,
                            const collision_solution *solution,
                            uint64_t *hash_a, uint64_t *hash_b)
{
    unsigned char *copy_a = malloc(length_a);
    unsigned char *copy_b = malloc(length_b);
    int matches;

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
    set_student_number(copy_a, STUDENT_ID);
    set_student_number(copy_b, STUDENT_ID);
    *hash_a = toy_hash(copy_a, length_a);
    *hash_b = toy_hash(copy_b, length_b);
    matches = *hash_a == *hash_b;
    free(copy_a);
    free(copy_b);
    return matches;
}

int main(int argc, char **argv)
{
    char *output_a;
    char *output_b;
    unsigned char *file_a;
    unsigned char *file_b;
    size_t length_a;
    size_t length_b;
    int thread_count = 1;
    int i;
    int found;
    int show_progress = 0;
    double start_time;
    double elapsed;
    collision_solution solution;
    uint64_t hash_a;
    uint64_t hash_b;

    if (argc < 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    for (i = 3; i < argc; ++i) {
        if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
            if (!parse_positive_int(argv[++i], &thread_count)) {
                fprintf(stderr, "Invalid thread count\n");
                return EXIT_FAILURE;
            }
        } else if (strcmp(argv[i], "--progress") == 0) {
            show_progress = 1;
        } else {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    file_a = load_file(argv[1], &length_a);
    file_b = load_file(argv[2], &length_b);
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
    if (thread_count == 1) {
        found = birthday_attack_serial(file_a, length_a, file_b, length_b,
                                       &solution, show_progress);
    } else {
        found = birthday_attack_parallel(file_a, length_a, file_b, length_b,
                                         thread_count, &solution, show_progress);
    }
    elapsed = omp_get_wtime() - start_time;
    if (!found) {
        fprintf(stderr, "No collision found. Search time: %.6f seconds\n", elapsed);
        free(file_a);
        free(file_b);
        return EXIT_FAILURE;
    }
    if (!verify_collision(file_a, length_a, file_b, length_b,
                          &solution, &hash_a, &hash_b)) {
        fprintf(stderr, "Candidate collision failed final verification\n");
        free(file_a);
        free(file_b);
        return EXIT_FAILURE;
    }

    set_nonce(file_a, solution.nonce_a);
    set_nonce(file_b, solution.nonce_b);
    set_student_number(file_a, STUDENT_ID);
    set_student_number(file_b, STUDENT_ID);

    if (!ensure_solved_directory()) {
        free(file_a);
        free(file_b);
        return EXIT_FAILURE;
    }
    output_a = make_solved_path(argv[1]);
    output_b = make_solved_path(argv[2]);
    if (output_a == NULL || output_b == NULL) {
        fprintf(stderr, "Could not allocate output paths\n");
        free(output_a);
        free(output_b);
        free(file_a);
        free(file_b);
        return EXIT_FAILURE;
    }
    if (!write_file(output_a, file_a, length_a) ||
        !write_file(output_b, file_b, length_b)) {
        free(output_a);
        free(output_b);
        free(file_a);
        free(file_b);
        return EXIT_FAILURE;
    }
    printf("nonce A: 0x%016llx\n", (unsigned long long) solution.nonce_a);
    printf("nonce B: 0x%016llx\n", (unsigned long long) solution.nonce_b);
    printf("hash A:  %012llx\n", (unsigned long long) hash_a);
    printf("hash B:  %012llx\n", (unsigned long long) hash_b);
    printf("solved A: %s\n", output_a);
    printf("solved B: %s\n", output_b);
    printf("search time: %.6f seconds\n", elapsed);
    free(output_a);
    free(output_b);
    free(file_a);
    free(file_b);
    return EXIT_SUCCESS;
}
