#include "attack_parallel.h"

#include "pdf_io.h"
#include "table.h"
#include "toy_hash.h"

#include <omp.h>
#include <stdio.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#define TRIAL_COUNT (UINT64_C(1) << 24)
#define STUDENT_ID "24295462"
/* TEMPORARY LOCAL DEBUG OUTPUT: remove this and the progress blocks below before submission. */
#define PROGRESS_INTERVAL (UINT64_C(1) << 18)

int birthday_attack_parallel(const unsigned char *file_a, size_t length_a,
                             const unsigned char *file_b, size_t length_b,
                             int thread_count, collision_solution *solution)
{
    collision_table *tables;
    atomic_int failed = 0;
    atomic_int found = 0;
    uint64_t completed = 0;
    double start_time;
    int team_size = 0;

    tables = calloc((size_t) thread_count, sizeof(*tables));
    if (tables == NULL) {
        return 0;
    }

    /* TEMPORARY LOCAL DEBUG OUTPUT: remove before submission. */
    start_time = omp_get_wtime();

    /* Each thread owns one table while inserting; all tables are read-only
       after the phase barrier, so parallel lookups require no table locks. */
#pragma omp parallel num_threads(thread_count) shared(team_size, completed)
    {
        int thread_id = omp_get_thread_num();
        unsigned char *candidate_a = malloc(length_a);
        unsigned char *candidate_b = malloc(length_b);
        uint64_t trials_per_thread;
        size_t table_capacity = 1;
        uint64_t nonce;
        uint64_t matching_nonce_a;
        uint64_t hash;

#pragma omp single
        team_size = omp_get_num_threads();

#pragma omp barrier
        trials_per_thread = (TRIAL_COUNT + (uint64_t) team_size - 1) /
                            (uint64_t) team_size;
        while ((uint64_t) table_capacity < 2 * trials_per_thread) {
            table_capacity <<= 1;
        }

        if ((candidate_a == NULL && length_a != 0) ||
            (candidate_b == NULL && length_b != 0) ||
            !collision_table_init(&tables[thread_id], table_capacity)) {
            atomic_store(&failed, 1);
        } else {
            memcpy(candidate_a, file_a, length_a);
            memcpy(candidate_b, file_b, length_b);
            set_student_number(candidate_a, STUDENT_ID);
            set_student_number(candidate_b, STUDENT_ID);
        }

#pragma omp barrier
        if (!atomic_load(&failed)) {
#pragma omp for schedule(static)
            for (nonce = 0; nonce < TRIAL_COUNT; ++nonce) {
                uint64_t progress;

                set_nonce(candidate_a, nonce);
                hash = toy_hash(candidate_a, length_a);
                if (!collision_table_insert(&tables[thread_id], hash, nonce)) {
                    atomic_store(&failed, 1);
                }

                /* TEMPORARY LOCAL DEBUG OUTPUT: remove before submission. */
                if ((nonce + 1) % PROGRESS_INTERVAL == 0) {
                    double elapsed;
                    double rate;
                    double eta;

#pragma omp atomic capture
                    {
                        completed += PROGRESS_INTERVAL;
                        progress = completed;
                    }
                    elapsed = omp_get_wtime() - start_time;
                    rate = (elapsed > 0.0) ? (double) progress / elapsed : 0.0;
                    eta = (rate > 0.0)
                              ? (double) (2 * TRIAL_COUNT - progress) / rate
                              : 0.0;
#pragma omp critical(progress_output)
                    {
                        fprintf(stderr,
                                "\rparallel attack: phase A, %6.2f%%, ETA %.1fs",
                                100.0 * (double) progress /
                                    (double) (2 * TRIAL_COUNT),
                                eta);
                        fflush(stderr);
                    }
                }
            }
        }

#pragma omp barrier
        if (!atomic_load(&failed)) {
#pragma omp for schedule(static)
            for (nonce = 0; nonce < TRIAL_COUNT; ++nonce) {
                int table_index;
                uint64_t progress;

                if (atomic_load(&found)) {
                    continue;
                }

                set_nonce(candidate_b, nonce);
                hash = toy_hash(candidate_b, length_b);
                for (table_index = 0; table_index < team_size; ++table_index) {
                    if (collision_table_find(&tables[table_index], hash,
                                             &matching_nonce_a)) {
                        int expected = 0;

                        if (atomic_compare_exchange_strong(&found, &expected, 1)) {
                            solution->nonce_a = matching_nonce_a;
                            solution->nonce_b = nonce;
                        }
                        break;
                    }
                }

                /* TEMPORARY LOCAL DEBUG OUTPUT: remove before submission. */
                if ((nonce + 1) % PROGRESS_INTERVAL == 0) {
                    double elapsed;
                    double rate;
                    double eta;

#pragma omp atomic capture
                    {
                        completed += PROGRESS_INTERVAL;
                        progress = completed;
                    }
                    elapsed = omp_get_wtime() - start_time;
                    rate = (elapsed > 0.0) ? (double) progress / elapsed : 0.0;
                    eta = (rate > 0.0)
                              ? (double) (2 * TRIAL_COUNT - progress) / rate
                              : 0.0;
#pragma omp critical(progress_output)
                    {
                        fprintf(stderr,
                                "\rparallel attack: phase B, %6.2f%%, ETA %.1fs",
                                100.0 * (double) progress /
                                    (double) (2 * TRIAL_COUNT),
                                eta);
                        fflush(stderr);
                    }
                }
            }
        }

        collision_table_destroy(&tables[thread_id]);
        free(candidate_a);
        free(candidate_b);
    }

    if (completed != 0) {
        fputc('\n', stderr);
    }
    free(tables);
    return !atomic_load(&failed) && atomic_load(&found);
}
