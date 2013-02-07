// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef __INCLUDED_LIBJSON_TABLE__
#define __INCLUDED_LIBJSON_TABLE__

// Simple O(N)-lookup table for (string, void*) pairs.

typedef struct Table Table;
typedef struct Table_iterator Table_iterator;

// Creates an empty table. If `size_hint` is non-positive it will be ignored;
// otherwise, it hints at the maximum number of key-value pairs the table will
// contain.
Table* table_create(int size_hint);

// Destroys a table created by table_create. `destroy_value` will be called
// for each value stored in the table.
void table_destroy(Table* t, void (*destroy_value)(void*));

// Sets the given key-value pair in the table `t`. If the key is null, the
// value will be appended without a key; otherwise, the key-value pair will be
// inserted (or overwritten if the key is already in the table). Returns the
// old value if the key was overwritten, null otherwise.
void* table_set_key(Table** t, const char* key, void* value);

// Returns the number of entries in the given table.
int table_get_size(const Table *t);

// Table_iterator API
Table_iterator* table_iterator_create(const Table* t);
void table_iterator_destroy(Table_iterator* iter);
void table_iterator_advance(Table_iterator* iter);
int table_iterator_is_valid(const Table_iterator* iter);
const char* table_iterator_curr_key(const Table_iterator* iter);
const void* table_iterator_curr_value(const Table_iterator* iter);

#endif
