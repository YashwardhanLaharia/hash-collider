#ifndef ATTACK_PARALLEL_H
#define ATTACK_PARALLEL_H

#include "collider_types.h"

#include <stddef.h>

int birthday_attack_parallel(const unsigned char *file_a, size_t length_a,
                             const unsigned char *file_b, size_t length_b,
                             int thread_count, collision_solution *solution);

#endif
