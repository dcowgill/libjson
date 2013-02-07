// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "table.h"
#include "munit.h"
#include "utilities.h"
#include <stdlib.h>
#include <string.h>

static void populate_table(Table** t, const char* pairs[], int n)
{
    int i;
    for (i = 0; i < n; i += 2)
        table_set_key(t, pairs[i], estrdup(pairs[i+1]));
}

static void test_new_table()
{
    Table* t = table_create(0);
    mu_assert(table_get_size(t) == 0);
    table_destroy(t, NULL);
}

static void test_add_one_pair()
{
    Table* t = table_create(0);
    table_set_key(&t, "foo", estrdup("bar"));
    mu_assert(table_get_size(t) == 1);
    Table_iterator* i = table_iterator_create(t);
    mu_assert(table_iterator_is_valid(i));
    mu_assert(strcmp(table_iterator_curr_key(i), "foo") == 0);
    mu_assert(strcmp((char*) table_iterator_curr_value(i), "bar") == 0);
    table_iterator_advance(i);
    mu_assert(!table_iterator_is_valid(i));
    table_iterator_destroy(i);
    table_destroy(t, free);
}

static void test_add_several_pairs()
{
    const char* pairs[] = {
        "a", "bb",
        "cc", "ddd",
        "eee", "ffff",
        "ggggg", "hhhhhh",
        "iiiiiii", "jjjjjjjj",
    };

    Table* t = table_create(0);
    int i;

    populate_table(&t, pairs, sizeof(pairs)/sizeof(pairs[0]));
    Table_iterator* it = table_iterator_create(t);
    for (i = 0; i < sizeof(pairs)/sizeof(pairs[0]); i += 2) {
        mu_assert(table_iterator_is_valid(it));
        mu_assert(strcmp(table_iterator_curr_key(it), pairs[i])==0);
        mu_assert(strcmp((char*) table_iterator_curr_value(it), pairs[i+1])==0);
        table_iterator_advance(it);
    }
    mu_assert(!table_iterator_is_valid(it));
    table_iterator_destroy(it);
    table_destroy(t, free);
}

static void test_null_key()
{
    Table* t = table_create(0);
    table_set_key(&t, NULL, estrdup("null key"));
    Table_iterator* it = table_iterator_create(t);
    mu_assert(table_iterator_is_valid(it));
    mu_assert(table_iterator_curr_key(it) == NULL);
    mu_assert(strcmp((char*) table_iterator_curr_value(it), "null key") == 0);
    table_iterator_advance(it);
    mu_assert(!table_iterator_is_valid(it));
    table_iterator_destroy(it);
    table_destroy(t, free);
}

static void test_mixed_keys()
{
    const char* pairs[] = {
        NULL,   "null key (1)",
        "k1",   "non-null key (1)",
        NULL,   "null key (2)",
        "k2",   "non-null key (2)",
    };

    Table* t = table_create(0);
    int i;

    populate_table(&t, pairs, sizeof(pairs)/sizeof(pairs[0]));
    Table_iterator* it = table_iterator_create(t);
    for (i = 0; i < sizeof(pairs)/sizeof(pairs[0]); i += 2) {
        mu_assert(table_iterator_is_valid(it));
        mu_assert((table_iterator_curr_key(it)==NULL && pairs[i]==NULL) ||
                  strcmp(table_iterator_curr_key(it), pairs[i])==0);
        mu_assert(strcmp((char*) table_iterator_curr_value(it), pairs[i+1])==0);
        table_iterator_advance(it);
    }
    mu_assert(!table_iterator_is_valid(it));
    table_iterator_destroy(it);
    table_destroy(t, free);
}

static void test_overwrite_key()
{
    const char key[] = "the key";

    Table* t = table_create(0);

    table_set_key(&t, key, estrdup("first value"));
    mu_assert(table_get_size(t) == 1);

    char* a = table_set_key(&t, key, estrdup("second value"));
    mu_assert(table_get_size(t) == 1);
    mu_assert(strcmp(a, "first value") == 0);
    free(a);

    char* b = table_set_key(&t, key, estrdup("third value"));
    mu_assert(table_get_size(t) == 1);
    mu_assert(strcmp(b, "second value") == 0);
    free(b);

    table_destroy(t, free);
}

static void run_all_tests()
{
    mu_run_test(test_new_table);
    mu_run_test(test_add_one_pair);
    mu_run_test(test_add_several_pairs);
    mu_run_test(test_null_key);
    mu_run_test(test_mixed_keys);
    mu_run_test(test_overwrite_key);
}

int main()
{
    run_all_tests();
    return mu_summarize();
}
