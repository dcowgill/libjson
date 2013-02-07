// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "json.h"
#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

void print_value(Json_value* a)
{
    Json_value* v = json_value_copy(a);
    char* s = json_stringify(v);
    putchar('\n');
    puts(s);
    putchar('\n');
    free(s);
    json_value_destroy(v);
}

int main()
{
    Json_value* array = json_value_new_array(0);
    json_value_append(array, json_value_from_cstr("Hello, world!"));
    json_value_append(array, json_value_from_double(12345));
    json_value_append(array, json_value_from_bool(0));
    json_value_append(array, json_value_new_null());

    Json_value* object = json_value_new_object(1);
    json_value_set_key(object, "foo", json_value_from_cstr("blah blah 1"));
    json_value_set_key(object, "bar", array);
    json_value_set_key(object, "baz", json_value_from_bool(1));
    json_value_set_key(object, "foo", json_value_from_cstr("blah blah 2"));

    print_value(object);

    {
        const char example_json[] =
            "{\"foo\":\n"
            "\"blah blah 2\", \"bar\"\n"
            ":[\"Hello,\tworld!\"\n"
            ",\n"
            "\n"
            "\n"
            "-3.14E100, false, null\n"
            "], \"baz\":true ,\n"
            "\n"
            "\n"
            "\n"
            "}   ";

        {
            Lexer* ll=lexer_create(example_json);
            while (lexer_advance(ll))
                fprintf_token(stdout, lexer_token(ll));
            if (lexer_error(ll) == lexer_error_input_exhausted)
                printf("Input exhausted.\n");
            else
                printf("Encountered error: %d\n", lexer_error(ll));
            lexer_destroy(ll);
        }

        Json_parse_error error;
        Json_value* v = json_parse(example_json, &error);

        if (v) {
            print_value(v);
            json_value_destroy(v);
        } else {
            fprintf(stderr, "Parse error on line %d, column %d: %d\n",
                    error.line, error.column, error.code);

        }
    }

    {
        char* s = json_stringify(object);
        Json_value* object2 = json_parse(s, NULL);
        char* t = json_stringify(object2);
        putchar('\n');
        puts(t);
        free(t);
        putchar('\n');
        puts(s);
        free(s);
        putchar('\n');
        json_value_destroy(object2);
    }


//     json_value_destroy(array);
    json_value_destroy(object);

    return 0;
}
