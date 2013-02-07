// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "str.h"
#include "munit.h"
#include <stdlib.h>
#include <string.h>

static void test_new_str()
{
    Str* s = str_create(99);
    mu_assert(str_length(s)==0);
    str_destroy(s);
}

static void test_append_cstr()
{
    Str* s = str_create(1);
    Str* t = str_create(1);
    str_append_cstr(&s, "Hello");
    mu_assert(strcmp(str_cstr(s), "Hello") == 0);
    str_append_cstr(&t, ", world!");
    mu_assert(strcmp(str_cstr(t), ", world!") == 0);
    str_append_cstr(&s, str_cstr(t));
    mu_assert(strcmp(str_cstr(s), "Hello, world!") == 0);
    mu_assert(str_length(s) == strlen("Hello, world!"));
    str_destroy(s);
    str_destroy(t);
}

static void test_append_char()
{
    Str* s = str_create(1);
    str_append_char(&s, '1');
    str_append_char(&s, '2');
    str_append_char(&s, '3');
    str_append_char(&s, '4');
    str_append_char(&s, '5');
    mu_assert(strcmp(str_cstr(s), "12345") == 0);
    mu_assert(str_length(s) == 5);
    str_destroy(s);
}

static void test_destroy_and_copy()
{
    Str* s = str_create(1);
    str_append_cstr(&s, "The quick brown fox...");
    char* t = str_destroy_and_copy(s);
    mu_assert(strcmp(t, "The quick brown fox...") == 0);
    free(t);
}

static void run_all_tests()
{
    mu_run_test(test_new_str);
    mu_run_test(test_append_cstr);
    mu_run_test(test_append_char);
    mu_run_test(test_destroy_and_copy);
}

int main()
{
    run_all_tests();
    return mu_summarize();
}
