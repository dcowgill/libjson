// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef __INCLUDED_LIBJSON_STR__
#define __INCLUDED_LIBJSON_STR__

typedef struct Str Str;

Str* str_create(int size_hint);
Str* str_create_from_cstr(const char* s);
void str_destroy(Str* s);
char* str_destroy_and_copy(Str* s);
void str_append_cstr(Str** s, const char* t);
void str_append_char(Str** s, char c);
const char* str_cstr(const Str* s);
int str_length(const Str* s);

#endif
