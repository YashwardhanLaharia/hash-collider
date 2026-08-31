#include "table.h"

#include <stdlib.h>

static size_t table_index(const collision_table *table, uint64_t hash)
{
    /* Multiplication spreads the 48-bit hash before power-of-two masking. */
    return (size_t) ((hash *  UINT64_C(11400714819323198485)) &
                     (table->capacity - 1));
}

int collision_table_init(collision_table *table, size_t capacity)
{
    if (capacity == 0 || (capacity & (capacity - 1)) != 0) {
        return 0;
    }
    table->entries = calloc(capacity, sizeof(*table->entries));
    if (table->entries == NULL) {
        return 0;
    }
    table->capacity = capacity;
    table->count = 0;
    return 1;
}

void collision_table_destroy(collision_table *table)
{
    free(table->entries);
    table->entries = NULL;
    table->capacity = 0;
    table->count = 0;
}

int collision_table_insert(collision_table *table, uint64_t hash, uint64_t nonce)
{
    size_t index;

    if (table->capacity == 0 || table->count == table->capacity) {
        return 0;
    }
    index = table_index(table, hash);
    while (table->entries[index].occupied && table->entries[index].hash != hash) {
        index = (index + 1) & (table->capacity - 1);
    }
    if (!table->entries[index].occupied) {
        table->entries[index].occupied = 1;
        table->entries[index].hash = hash;
        table->count++;
    }
    table->entries[index].nonce = nonce;
    return 1;
}

int collision_table_find(const collision_table *table, uint64_t hash,
                         uint64_t *nonce)
{
    size_t index;
    size_t probes = 0;

    if (table->capacity == 0) {
        return 0;
    }
    index = table_index(table, hash);
    while (probes++ < table->capacity && table->entries[index].occupied) {
        if (table->entries[index].hash == hash) {
            *nonce = table->entries[index].nonce;
            return 1;
        }
        index = (index + 1) & (table->capacity - 1);
    }
    return 0;
}
