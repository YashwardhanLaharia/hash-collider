#ifndef ATTACK_SERIAL_H
#define ATTACK_SERIAL_H

#include "collider_types.h"

#include <stddef.h>

int birthday_attack_serial(const unsigned char *file_a, size_t length_a,
                           const unsigned char *file_b, size_t length_b,
                           collision_solution *solution);

#endif
