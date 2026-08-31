#include "attack_serial.h"

#include "pdf_io.h"
#include "table.h"
#include "toy_hash.h"

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRIAL_COUNT (UINT64_C(1) << 24)
#define TABLE_CAPACITY (UINT64_C(1) << 25)
#define STUDENT_ID "24295462"
/* TEMPORARY LOCAL DEBUG OUTPUT: remove this and the progress blocks below before submission. */
#define PROGRESS_INTERVAL (UINT64_C(1) << 20)

int birthday_attack_serial(const unsigned char *file_a, size_t length_a,
                           const unsigned char *file_b, size_t length_b,
                           collision_solution *solution)
{
    unsigned char *candidate_a = NULL;
    unsigned char *candidate_b = NULL;
    collision_table table = {0};
    uint64_t nonce_a;
    uint64_t nonce_b;
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

    /* TEMPORARY LOCAL DEBUG OUTPUT: remove before submission. */
    start_time = omp_get_wtime();
    for (nonce_a = 0; nonce_a < TRIAL_COUNT; ++nonce_a) {
        set_nonce(candidate_a, nonce_a);
        hash = toy_hash(candidate_a, length_a);
        if (!collision_table_insert(&table, hash, nonce_a)) {
            goto cleanup;
        }
        if ((nonce_a + 1) % PROGRESS_INTERVAL == 0) {
            completed = nonce_a + 1;
            elapsed = omp_get_wtime() - start_time;
            rate = (elapsed > 0.0) ? (double) completed / elapsed : 0.0;
            eta = (rate > 0.0) ? (double) (2 * TRIAL_COUNT - completed) / rate : 0.0;
            fprintf(stderr, "\rserial attack: phase A, %6.2f%%, ETA %.1fs",
                    100.0 * (double) completed / (double) (2 * TRIAL_COUNT), eta);
            fflush(stderr);
            progress_printed = 1;
        }
    }

    for (nonce_b = 0; nonce_b < TRIAL_COUNT; ++nonce_b) {
        set_nonce(candidate_b, nonce_b);
        hash = toy_hash(candidate_b, length_b);
        if (collision_table_find(&table, hash, &matching_nonce_a)) {
            solution->nonce_a = matching_nonce_a;
            solution->nonce_b = nonce_b;
            found = 1;
            break;
        }
        if ((nonce_b + 1) % PROGRESS_INTERVAL == 0) {
            completed = TRIAL_COUNT + nonce_b + 1;
            elapsed = omp_get_wtime() - start_time;
            rate = (elapsed > 0.0) ? (double) completed / elapsed : 0.0;
            eta = (rate > 0.0) ? (double) (2 * TRIAL_COUNT - completed) / rate : 0.0;
            fprintf(stderr, "\rserial attack: phase B, %6.2f%%, ETA %.1fs",
                    100.0 * (double) completed / (double) (2 * TRIAL_COUNT), eta);
            fflush(stderr);
            progress_printed = 1;
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
