// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef __INCLUDED_LIBJSON_UTILITIES_
#define __INCLUDED_LIBJSON_UTILITIES_

#include <stdlib.h>

// This macro provides a simple printf-and-abort facility. It's similar to the
// standard assert macro, except this one takes printf-style arguments instead
// of a boolean expression. Note that the macro arguments must be doubly
// parenthesized, e.g. JSON_PANIC(("Hello, %s!", "world"));
#define JSON_PANIC(args)                                    \
    json_panic(__FILE__, __LINE__, __PRETTY_FUNCTION__) args

// This function is used by the JSON_PANIC macro; never call it directly.
void (*json_panic(char const*,int,char const*))(char const*, ...);

// These functions work exactly like their standard library counterparts,
// except they abort if memory allocation fails.
void* emalloc(size_t n);
void* erealloc(void* vp, size_t n);
char* estrdup(const char* s);

#endif
