// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef __INCLUDED_LIBJSON_PARSER__
#define __INCLUDED_LIBJSON_PARSER__

#include "json.h"
#include "tokens.h"
#include <stdio.h>

typedef enum Json_parse_error_code Json_parse_error_code;
enum Json_parse_error_code {
    json_parse_error_success = 0,
    json_parse_error_internal,
    json_parse_error_unexpected_input,
    json_parse_error_unexpected_end_of_input,
    json_parse_error_illegal_escape_sequence,
    json_parse_error_runaway_string,
    json_parse_error_runaway_comment,
    json_parse_error_invalid_number,
    json_parse_error_extraneous_input,
};

enum { json_max_expected_tokens = 10 };

typedef struct Json_parse_error Json_parse_error;
struct Json_parse_error {
    Json_parse_error_code code; // describes the error
    int line;                   // line on which error was encountered
    int column;                 // column within the line of the error

    // Array of tokens we were expecting to see, but didn't. Terminated by an
    // instance of json_token_type_error (see lexer.h).
    Json_token_type expected_token_types[json_max_expected_tokens + 1];

    // The token type we encountered but didn't expect. If not relevant to
    // this particular parse error, will be set to json_token_type_error.
    Json_token_type actual_token_type;
};

// Parses a JSON input string, returning a Json_value or NULL on error.
Json_value* json_parse(const char* input, Json_parse_error* errorp);

void json_parse_error_print(FILE* fp, const Json_parse_error e);
const char* json_parse_error_code_to_string(Json_parse_error_code e);

#endif
