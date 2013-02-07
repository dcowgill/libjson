// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "json.h"
#include "parser.h"
#include "str.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    // Read json code from stdin into `input`.
    Str* input = str_create(0);
    char buf[100];
    while (fgets(buf, sizeof(buf), stdin)) {
        str_append_cstr(&input, buf);
    }

    // Parse `input` and abort if an error occurs.
    Json_parse_error error;
    Json_value* value = json_parse(str_cstr(input), &error);
    free(input);
    if (value == NULL) {
        json_parse_error_print(stdout, error);
        fprintf(stdout, "\n");
        return 2;
    }

    // Convert `value` to a string and write to stdout.
    char* output = json_stringify(value);
    json_value_destroy(value);
    printf("%s\n", output);
    free(output);

    return 0;
}
