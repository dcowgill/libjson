// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "str.h"
#include "utilities.h"
#include <assert.h>
#include <string.h>

struct Str {
    int length;
    int size;
    char data[1];
};

static void _str_insure_capacity(Str** s, int extra_chars)
{
    Str* t = *s;  // to reduce dereferencing
    const int min_size = t->length + extra_chars + 1;
    if (t->size < min_size) {
        do { t->size *= 2; } while (t->size < min_size);
        Str* new_str = str_create(t->size);
        new_str->length = t->length;
        memcpy(new_str->data, t->data, t->length + 1);
        str_destroy(t);
        *s = new_str;
    }
}

Str* str_create(int size_hint)
{
    const int size = size_hint > 0 ? size_hint : 16;
    Str* s = (Str*) emalloc(sizeof(Str) + size*sizeof(char));
    s->length = 0;
    s->size = size;
    s->data[0] = '\0';
    return s;
}

Str* str_create_from_cstr(const char* t)
{
    const int size = strlen(t) + 1;
    Str* s = (Str*) emalloc(sizeof(Str) + size);
    s->length = size - 1;
    s->size = size;
    memcpy(s->data, t, size);
    return s;
}

void str_destroy(Str* s)
{
    free(s);
}

char* str_destroy_and_copy(Str* s)
{
    char* copy = estrdup(str_cstr(s));
    str_destroy(s);
    return copy;
}


void str_append_cstr(Str** s, const char* t)
{
    const int t_len = strlen(t);
    _str_insure_capacity(s, t_len);
    memcpy((*s)->data + (*s)->length, t, t_len + 1);
    (*s)->length += t_len;
}

void str_append_char(Str** s, char c)
{
    _str_insure_capacity(s, 1);
    (*s)->data[(*s)->length++] = c;
    (*s)->data[(*s)->length] = '\0';
}

const char* str_cstr(const Str* s)
{
    return s->data;
}

int str_length(const Str* s)
{
    return s->length;
}
