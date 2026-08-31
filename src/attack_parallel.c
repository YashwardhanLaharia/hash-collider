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
/* Progress output is opt-in so benchmark runs remain quiet by default. */
#define PROGRESS_INTERVAL (UINT64_C(1) << 18)
#define B_BATCH_SIZE (UINT64_C(8) * TRIAL_COUNT)

static size_t partition_for_hash(uint64_t hash, int partition_count)
{
    return (size_t) (hash % (uint64_t) partition_count);
}

int birthday_attack_parallel(const unsigned char *file_a, size_t length_a,
                             const unsigned char *file_b, size_t length_b,
                             int thread_count, collision_solution *solution,
                             int show_progress)
{
    collision_table *tables;
    omp_lock_t *locks;
    atomic_int failed = 0;
    atomic_int found = 0;
    uint64_t completed = 0;
    uint64_t batch_start = 0;
    double start_time;
    int team_size = 0;
    int initialized_locks = 0;
    int exhausted = 0;

    tables = calloc((size_t) thread_count, sizeof(*tables));
    locks = calloc((size_t) thread_count, sizeof(*locks));
    if (tables == NULL || locks == NULL) {
        free(tables);
        free(locks);
        return 0;
    }

    start_time = omp_get_wtime();

    /* Hash partitions are locked during insertion and become read-only after
       the phase barrier, so parallel lookups require no table locks. */
#pragma omp parallel num_threads(thread_count) shared(team_size, completed)
    {
        unsigned char *candidate_a = malloc(length_a);
        unsigned char *candidate_b = malloc(length_b);
        uint64_t trials_per_thread;
        size_t table_capacity = 1;
        uint64_t nonce;
        uint64_t matching_nonce_a;
        uint64_t hash;

#pragma omp single
        {
            int partition;

            team_size = omp_get_num_threads();
            trials_per_thread = (TRIAL_COUNT + (uint64_t) team_size - 1) /
                                (uint64_t) team_size;
            while ((uint64_t) table_capacity < 2 * trials_per_thread) {
                table_capacity <<= 1;
            }
            for (partition = 0; partition < team_size; ++partition) {
                omp_init_lock(&locks[partition]);
                ++initialized_locks;
                if (!collision_table_init(&tables[partition], table_capacity)) {
                    atomic_store(&failed, 1);
                }
            }
        }

#pragma omp barrier
        if ((candidate_a == NULL && length_a != 0) ||
            (candidate_b == NULL && length_b != 0)) {
            atomic_store(&failed, 1);
        }
#pragma omp barrier
        if (!atomic_load(&failed)) {
            memcpy(candidate_a, file_a, length_a);
            memcpy(candidate_b, file_b, length_b);
            set_student_number(candidate_a, STUDENT_ID);
            set_student_number(candidate_b, STUDENT_ID);
#pragma omp for schedule(static)
            for (nonce = 0; nonce < TRIAL_COUNT; ++nonce) {
                uint64_t progress;
                size_t partition;

                set_nonce(candidate_a, nonce);
                hash = toy_hash(candidate_a, length_a);
                partition = partition_for_hash(hash, team_size);
                omp_set_lock(&locks[partition]);
                if (!collision_table_insert(&tables[partition], hash, nonce)) {
                    atomic_store(&failed, 1);
                }
                omp_unset_lock(&locks[partition]);

                if (show_progress && (nonce + 1) % PROGRESS_INTERVAL == 0) {
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
                              ? (double) (TRIAL_COUNT + B_BATCH_SIZE - progress) /
                                    rate
                              : 0.0;
#pragma omp critical(progress_output)
                    {
                        fprintf(stderr,
                                "\rparallel attack: phase A, %6.2f%%, ETA %.1fs",
                                100.0 * (double) progress /
                                    (double) (TRIAL_COUNT + B_BATCH_SIZE),
                                eta);
                        fflush(stderr);
                    }
                }
            }
        }

#pragma omp barrier
        if (!atomic_load(&failed)) {
#pragma omp single
            {
                completed = 0;
                start_time = omp_get_wtime();
            }
            while (!atomic_load(&found) && !exhausted) {
                uint64_t offset_b;

#pragma omp for schedule(static)
                for (offset_b = 0; offset_b < B_BATCH_SIZE; ++offset_b) {
                    size_t partition;
                    uint64_t progress;

                    if (atomic_load(&found)) {
                        continue;
                    }

                    nonce = batch_start + offset_b;
                    set_nonce(candidate_b, nonce);
                    hash = toy_hash(candidate_b, length_b);
                    partition = partition_for_hash(hash, team_size);
                    if (collision_table_find(&tables[partition], hash,
                                             &matching_nonce_a)) {
                        int expected = 0;

                        if (atomic_compare_exchange_strong(&found, &expected, 1)) {
                            solution->nonce_a = matching_nonce_a;
                            solution->nonce_b = nonce;
                        }
                    }

                    if (show_progress &&
                        (offset_b + 1) % PROGRESS_INTERVAL == 0) {
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
                                  ? (double) (B_BATCH_SIZE - progress) / rate
                                  : 0.0;
#pragma omp critical(progress_output)
                        {
                            fprintf(stderr,
                                    "\rparallel attack: phase B from 0x%016llx, %6.2f%%, ETA %.1fs",
                                    (unsigned long long) batch_start,
                                    100.0 * (double) progress /
                                        (double) B_BATCH_SIZE,
                                    eta);
                            fflush(stderr);
                        }
                    }
                }

#pragma omp single
                {
                    if (!atomic_load(&found)) {
                        if (batch_start > UINT64_MAX - B_BATCH_SIZE) {
                            exhausted = 1;
                        } else {
                            batch_start += B_BATCH_SIZE;
                            completed = 0;
                            start_time = omp_get_wtime();
                        }
                    }
                }
            }
        }

        free(candidate_a);
        free(candidate_b);
    }

    if (completed != 0) {
        fputc('\n', stderr);
    }
    for (int partition = 0; partition < team_size; ++partition) {
        collision_table_destroy(&tables[partition]);
        if (partition < initialized_locks) {
            omp_destroy_lock(&locks[partition]);
        }
    }
    free(tables);
    free(locks);
    return !atomic_load(&failed) && atomic_load(&found);
}
