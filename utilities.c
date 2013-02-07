// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "utilities.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void _panic_printf(char const* fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

	if (fmt[0] != '\0' && fmt[strlen(fmt)-1] == ':') {
		fprintf(stderr, " %s", strerror(errno));
    }
    fputc('\n', stderr);

    abort();
}

void (*json_panic(char const* file,
                  int line,
                  char const* func))(char const*, ...)
{
    fprintf(stderr, "PANIC: %s:%d: %s: ", file, line, func);
    return _panic_printf;
}

void* emalloc(size_t n)
{
    void* p = malloc(n);
    if (p == NULL) {
        JSON_PANIC(("malloc of %u bytes failed:", n));
    }
    return p;
}

void* erealloc(void* vp, size_t n)
{
	void *p = realloc(vp, n);
	if (p == NULL) {
		JSON_PANIC(("realloc of %u bytes failed:", n));
    }
	return p;
}

char* estrdup(const char* s)
{
    if (s == NULL) {
        return NULL;
    }
    const size_t n = strlen(s) + 1;
    char* t = (char*) malloc(n);
    if (t == NULL) {
        JSON_PANIC(("estrdup(\"%.20s\") failed:", s));
    }
    memcpy(t, s, n);
    return t;
}
