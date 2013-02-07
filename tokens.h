// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef __INCLUDED_LIBJSON_TOKENS__
#define __INCLUDED_LIBJSON_TOKENS__

typedef enum Json_token_type Json_token_type;
enum Json_token_type {
    json_token_type_error = 0,
    json_token_type_null,
    json_token_type_string,
    json_token_type_number,
    json_token_type_true,
    json_token_type_false,
    json_token_type_left_bracket,
    json_token_type_right_bracket,
    json_token_type_left_curly,
    json_token_type_right_curly,
    json_token_type_comma,
    json_token_type_colon,
    json_token_type_identifier,
};

#endif
