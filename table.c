// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "table.h"
#include "utilities.h"
#include <assert.h>
#include <string.h>

typedef struct Pair Pair;
struct Pair {
    char* key;
    void* value;
};

struct Table {
    int capacity;   // physical length of the `pairs` array
    int size;       // logical length (number of entries in table
    Pair pairs[1];  // table contents: array of key-value pairs
};

static void _extend_table(Table** t, int new_capacity)
{
    assert((*t)->capacity < new_capacity);
    Table* new_table = table_create(new_capacity);
    new_table->size = (*t)->size;
    memcpy(new_table->pairs, (*t)->pairs, (*t)->size*sizeof(Pair));
    free(*t);
    *t = new_table;
}

static int _keys_are_equal(const char* s, const char* t)
{
    // Null keys can never equal other keys, not even other null keys.
    return s && t && strcmp(s, t) == 0;
}

// +-----------+
// | Table API |
// +-----------+

Table* table_create(int size_hint)
{
    const int capacity = size_hint > 0 ? size_hint : 4;
    Table* t = (Table*) emalloc(2*sizeof(int) + capacity*sizeof(Pair));
    t->capacity = capacity;
    t->size = 0;
    return t;
}

void table_destroy(Table* t, void (*destroy_value)(void*))
{
    assert(t);
    int i;
    for (i = 0; i < t->size; i++) {
        free(t->pairs[i].key);
        if (destroy_value != NULL)
            destroy_value(t->pairs[i].value);
    }
    free(t);
}

void* table_set_key(Table** t, const char* key, void* value)
{
    // If a key was given, search for it and overwrite its value if found.
    if (key != NULL) {
        int i;
        for (i = 0; i < (*t)->size; i++) {
            if (_keys_are_equal((*t)->pairs[i].key, key)) {
                void* old_value = (*t)->pairs[i].value;
                (*t)->pairs[i].value = value;
                return old_value;
            }
        }
    }

    // If the table is full, double its capacity.
    if ((*t)->capacity <= (*t)->size)
        _extend_table(t, (*t)->capacity * 2);

    // Add the key-value pair to the end of the table.
    (*t)->pairs[(*t)->size].key = estrdup(key);
    (*t)->pairs[(*t)->size].value = value;
    (*t)->size++;

    return NULL;
}

int table_get_size(const Table *t)
{
    return t->size;
}

// +--------------------+
// | Table_iterator API |
// +--------------------+

struct Table_iterator {
    const Table* table; // table over which we are iterating
    int pos;            // current position in iteration
};

Table_iterator* table_iterator_create(const Table* t)
{
    Table_iterator* iter = (Table_iterator*) emalloc(sizeof(Table_iterator));
    iter->table = t;
    iter->pos = 0;
    return iter;
}

void table_iterator_destroy(Table_iterator* iter)
{
    free(iter);
}

void table_iterator_advance(Table_iterator* iter)
{
    iter->pos++;
}

int table_iterator_is_valid(const Table_iterator* iter)
{
    return iter->pos < table_get_size(iter->table);
}

const char* table_iterator_curr_key(const Table_iterator* iter)
{
    assert(table_iterator_is_valid(iter));
    return iter->table->pairs[iter->pos].key;
}

const void* table_iterator_curr_value(const Table_iterator* iter)
{
    assert(table_iterator_is_valid(iter));
    return iter->table->pairs[iter->pos].value;
}
