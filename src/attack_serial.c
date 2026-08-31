#include "attack_serial.h"

#include "pdf_io.h"
#include "table.h"
#include "toy_hash.h"

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRIAL_COUNT (UINT64_C(1) << 24)
#define B_BATCH_SIZE (UINT64_C(8) * TRIAL_COUNT)
#define TABLE_CAPACITY (UINT64_C(1) << 25)
#define STUDENT_ID "24295462"
/* Progress output is opt-in so benchmark runs remain quiet by default. */
#define PROGRESS_INTERVAL (UINT64_C(1) << 20)

int birthday_attack_serial(const unsigned char *file_a, size_t length_a,
                           const unsigned char *file_b, size_t length_b,
                           collision_solution *solution, int show_progress)
{
    unsigned char *candidate_a = NULL;
    unsigned char *candidate_b = NULL;
    collision_table table = {0};
    uint64_t nonce_a;
    uint64_t nonce_b;
    uint64_t batch_start = 0;
    uint64_t offset_b;
    uint64_t matching_nonce_a;
    uint64_t hash;
    uint64_t completed;
    double start_time;
    double elapsed;
    double rate;
    double eta;
    int progress_printed = 0;
    int found = 0;

    candidate_a = malloc(length_a);
    candidate_b = malloc(length_b);
    if ((candidate_a == NULL && length_a != 0) ||
        (candidate_b == NULL && length_b != 0)) {
        goto cleanup;
    }
    memcpy(candidate_a, file_a, length_a);
    memcpy(candidate_b, file_b, length_b);
    set_student_number(candidate_a, STUDENT_ID);
    set_student_number(candidate_b, STUDENT_ID);

    if (!collision_table_init(&table, (size_t) TABLE_CAPACITY)) {
        goto cleanup;
    }

    start_time = omp_get_wtime();
    for (nonce_a = 0; nonce_a < TRIAL_COUNT; ++nonce_a) {
        set_nonce(candidate_a, nonce_a);
        hash = toy_hash(candidate_a, length_a);
        if (!collision_table_insert(&table, hash, nonce_a)) {
            goto cleanup;
        }
        if (show_progress && (nonce_a + 1) % PROGRESS_INTERVAL == 0) {
            completed = nonce_a + 1;
            elapsed = omp_get_wtime() - start_time;
            rate = (elapsed > 0.0) ? (double) completed / elapsed : 0.0;
            eta = (rate > 0.0)
                       ? (double) (TRIAL_COUNT + B_BATCH_SIZE - completed) / rate
                      : 0.0;
            fprintf(stderr, "\rserial attack: phase A, %6.2f%%, ETA %.1fs",
                    100.0 * (double) completed /
                        (double) (TRIAL_COUNT + B_BATCH_SIZE),
                    eta);
            fflush(stderr);
            progress_printed = 1;
        }
    }

    while (!found) {
        start_time = omp_get_wtime();
        for (offset_b = 0; offset_b < B_BATCH_SIZE; ++offset_b) {
            nonce_b = batch_start + offset_b;
            set_nonce(candidate_b, nonce_b);
            hash = toy_hash(candidate_b, length_b);
            if (collision_table_find(&table, hash, &matching_nonce_a)) {
                solution->nonce_a = matching_nonce_a;
                solution->nonce_b = nonce_b;
                found = 1;
                break;
            }
            if (show_progress && (offset_b + 1) % PROGRESS_INTERVAL == 0) {
                completed = offset_b + 1;
                elapsed = omp_get_wtime() - start_time;
                rate = (elapsed > 0.0) ? (double) completed / elapsed : 0.0;
                eta = (rate > 0.0)
                          ? (double) (B_BATCH_SIZE - completed) / rate
                          : 0.0;
                fprintf(stderr,
                        "\rserial attack: phase B from 0x%016llx, %6.2f%%, ETA %.1fs",
                        (unsigned long long) batch_start,
                        100.0 * (double) completed / (double) B_BATCH_SIZE,
                        eta);
                fflush(stderr);
                progress_printed = 1;
            }
        }
        if (!found) {
            if (batch_start > UINT64_MAX - B_BATCH_SIZE) {
                goto cleanup;
            }
            batch_start += B_BATCH_SIZE;
        }
    }

cleanup:
    if (progress_printed) {
        fputc('\n', stderr);
    }
    collision_table_destroy(&table);
    free(candidate_a);
    free(candidate_b);
    return found;
}
