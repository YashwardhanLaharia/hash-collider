#ifndef COLLISION_TABLE_H
#define COLLISION_TABLE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint64_t hash;
    uint64_t nonce;
    int occupied;
} collision_entry;

typedef struct {
    collision_entry *entries;
    size_t capacity;
    size_t count;
} collision_table;

int collision_table_init(collision_table *table, size_t capacity);
void collision_table_destroy(collision_table *table);
int collision_table_insert(collision_table *table, uint64_t hash, uint64_t nonce);
int collision_table_find(const collision_table *table, uint64_t hash,
                         uint64_t *nonce);

#endif
