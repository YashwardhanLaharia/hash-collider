#include "attack_parallel.h"

#include <stdio.h>

int birthday_attack_parallel(const unsigned char *file_a, size_t length_a,
                             const unsigned char *file_b, size_t length_b,
                             int thread_count, collision_solution *solution)
{
    (void) file_a;
    (void) length_a;
    (void) file_b;
    (void) length_b;
    (void) thread_count;
    (void) solution;

    fprintf(stderr, "birthday_attack_parallel() is not implemented yet\n");
    return 0;
}
