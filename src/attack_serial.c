#include "attack_serial.h"

#include <stdio.h>

int birthday_attack_serial(const unsigned char *file_a, size_t length_a,
                           const unsigned char *file_b, size_t length_b,
                           collision_solution *solution)
{
    (void) file_a;
    (void) length_a;
    (void) file_b;
    (void) length_b;
    (void) solution;

    fprintf(stderr, "birthday_attack_serial() is not implemented yet\n");
    return 0;
}
