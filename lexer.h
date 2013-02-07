// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef __INCLUDED_LIBJSON_LEXER__
#define __INCLUDED_LIBJSON_LEXER__

// Zero-lookahead lexical analyzer for JSON syntax.

#include "tokens.h"
#include <stdio.h>

typedef enum Lexer_error Lexer_error;
enum Lexer_error {
    lexer_error_success = 0,
    lexer_error_input_exhausted,
    lexer_error_illegal_escape_sequence,
    lexer_error_runaway_string,
    lexer_error_runaway_comment,
    lexer_error_numeric_overflow,
    lexer_error_numeric_underflow,
    lexer_error_invalid_number,
    lexer_error_unexpected_input,
};

typedef struct Token Token;
struct Token {
    Json_token_type type; // kind of token
    union {
        const char* string;
        double number;
    } value;        // set only when the value isn't implied by type
};

typedef struct Lexer Lexer;

Lexer* lexer_create(const char* input);
void lexer_destroy(Lexer* lex);
Lexer_error lexer_error(const Lexer* lex);
int lexer_has_error(const Lexer* lex);
Token lexer_token(Lexer* lex);
int lexer_advance(Lexer* lex);
int lexer_line(const Lexer* lex);
int lexer_column(const Lexer* lex);
const char* token_type_to_string(Json_token_type type);
void fprintf_token(FILE* out, Token tok);

#endif
