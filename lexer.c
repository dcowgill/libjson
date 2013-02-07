// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "lexer.h"
#include "str.h"
#include "utilities.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

struct Lexer {
    const char* start;              // start of input
    const char* curr;               // current position in input
    const char* end;                // one past last character in input
    int line;                       // current line number
    const char* curr_line_start;    // for computing column# within line
    char* tok_str;                  // storage for string tokens
    int tok_str_size;               // capacity of token_string
    Token token;                    // current token in stream
    Lexer_error error;              // stores the error state
};

//
// +--------------------+
// | internal functions |
// +--------------------+
//

// Copies the first `n` bytes starting at `p` into `lex->tok_str`.
static const char* _copy_cstr_into_lexer(Lexer* lex, const char* p, int n)
{
    const int size = n + 1;
    if (lex->tok_str_size < size) {
        lex->tok_str = (char*) erealloc(lex->tok_str, size);
        lex->tok_str_size = size;
    }
    memcpy(lex->tok_str, p, n);
    lex->tok_str[n] = '\0';
    return lex->tok_str;
}

// Copies contents `s` into `lex->tok_str`.
static const char* _copy_str_into_lexer(Lexer* lex, const Str* s)
{
    return _copy_cstr_into_lexer(lex, str_cstr(s), str_length(s));
}

// Stores the current token and advances the lexer `length` characters.
static void _store_token(Lexer* lex, Json_token_type type, int length)
{
    lex->token.type = type;
    lex->curr += length;
}

// Case-insentive keyword match.
static int _match_keyword(const Lexer* lex, const char* keyword)
{
    return strncasecmp(lex->curr, keyword, strlen(keyword)) == 0;
}

// Tests whether an identifier begins at current position.
static int _match_identifier(const Lexer* lex)
{
    char c = lex->curr[0];
    return isalpha(c) || c == '_';
}

// Increments the lexer line number.
static void _next_line(Lexer* lex)
{
    lex->line++;
    lex->curr_line_start = lex->curr;
}

// If there is a newline at the current lexer position, advances past it,
// increments the line number, and returns 1. Otherwise, returns 0.
static int _skip_newline(Lexer* lex)
{
    char c = lex->curr[0];

    if (c == '\n' || c == '\r') {   // deal with line endings
        if (c == '\r' && lex->curr[1] == '\n') {
            lex->curr++;            // special handling for CRLFs
        }
        lex->curr++;
        _next_line(lex);
        return 1;
    }
    return 0;
}

// Advances lexer past whitespace; returns non-zero if input remains.
static int _skip_ws(Lexer* lex)
{
    char c;
    while (isspace(c = lex->curr[0])) {
        if (!_skip_newline(lex)) {
            lex->curr++;
        }
    }
    return lex->curr < lex->end;
}

// Advances the lexer past the c++-style comment at the current position.
static void _skip_slash_slash_comment(Lexer* lex)
{
    assert(lex->curr[0] == '/' && lex->curr[1] == '/');
    lex->curr += 2;
    while (lex->curr < lex->end && !_skip_newline(lex)) {
        lex->curr++;
    }
}

// Advances the lexer past the c-style comment at the current position.
static void _skip_slash_star_comment(Lexer* lex)
{
    assert(lex->curr[0] == '/' && lex->curr[1] == '*');
    lex->curr += 2;
    while (lex->curr < lex->end) {
        if (lex->curr[0] == '*' && lex->curr[1] == '/') {
            lex->curr += 2;
            return;
        }
        if (!_skip_newline(lex)) {
            lex->curr++;
        }
    }
    lex->error = lexer_error_runaway_comment;
}

// If a comment begins at the current position, advances the lexer past it and
// returns 1. Otherwise, returns 0.
static int _skip_comment(Lexer* lex)
{
    if (lex->curr[0] == '/') {
        if (lex->curr[1] == '/') {
            _skip_slash_slash_comment(lex);
            return 1;
        }
        if (lex->curr[1] == '*') {
            _skip_slash_star_comment(lex);
            return 1;
        }
    }
    return 0;
}

// Parses a double-quoted string token.
static int _parse_string(Lexer* lex)
{
    // FIXME: convert unicode character sequences

    if (lex->curr[0] != '"' && lex->curr[0] != '\'') {
        JSON_PANIC(("Current token must begin with \" or '."));
    }

    Str* answer = str_create(16);
    const char* p = lex->curr + 1;
    const char terminator = lex->curr[0];

    for (; p < lex->end; p++) {
        if (*p == terminator) {
            lex->token.type = json_token_type_string;
            lex->token.value.string = _copy_str_into_lexer(lex, answer);
            str_destroy(answer);
            lex->curr = p + 1;
            return 1;
        } else if (*p == '\\') {
            if (p[1] == terminator) {
                str_append_char(&answer, *++p);
                continue;
            }
            switch (p[1]) {
            case '\\': case '/':
                str_append_char(&answer, p[1]);
                break;
            case 'b':
                str_append_char(&answer, '\b');
                break;
            case 'f':
                str_append_char(&answer, '\f');
                break;
            case 'n':
                str_append_char(&answer, '\n');
                break;
            case 'r':
                str_append_char(&answer, '\r');
                break;
            case 't':
                str_append_char(&answer, '\t');
                break;
            case 'u':
                JSON_PANIC(("unicode not supported yet"));
                break;
            default:
                // illegal escape sequence
                lex->error = lexer_error_illegal_escape_sequence;
                return 0;
            }
            p++;
        } else if (*p == '\n' || *p == '\r') {
            if (*p == '\r' && *(p+1) == '\n') {
                str_append_char(&answer, *p++);
            }
            str_append_char(&answer, *p);
            _next_line(lex);
        } else {
            str_append_char(&answer, *p);
        }
    }

    // runaway string
    lex->error = lexer_error_runaway_string;
    str_destroy(answer);
    return 0;
}

// Parses a numeric token.
static int _parse_number(Lexer* lex)
{
    errno = 0;

    char* endptr;
    const double d = strtod(lex->curr, &endptr);

    if (errno == ERANGE) {      // underflow if d is zero, else overflow
        lex->error = (d == 0.0)
            ? lexer_error_numeric_underflow
            : lexer_error_numeric_overflow;
        return 0;
    }
    if (lex->curr == endptr) {  // no conversion was performed
        lex->error = lexer_error_invalid_number;
        return 0;
    }

    lex->token.type = json_token_type_number;
    lex->token.value.number = d;
    lex->curr = endptr;
    return 1;
}

// Parses an identifier ([a-z][a-z0-9_]*).
static int _parse_identifier(Lexer* lex)
{
    const char* p;
    int len;

    if (!_match_identifier(lex)) {
        JSON_PANIC(("Expected an identifier at current position."));
    }

    for (p = lex->curr + 1; p != lex->end; p++) {
        if (!isalnum(*p) && *p != '_') {
            break;
        }
    }

    len = p - lex->curr;
    lex->token.value.string = _copy_cstr_into_lexer(lex, lex->curr, len);
    _store_token(lex, json_token_type_identifier, len);
    return 1;
}

//
// +------------------+
// | Lexer public API |
// +------------------+
//

Lexer* lexer_create(const char* input)
{
    Lexer* lex = (Lexer*) emalloc(sizeof(Lexer));
    lex->curr = lex->curr_line_start = lex->start = input;
    lex->end = lex->start + strlen(lex->start);
    lex->line = 0;
    lex->tok_str_size = 50;
    lex->tok_str = (char*) emalloc(lex->tok_str_size);
    lex->error = lexer_error_success;
    return lex;
}

void lexer_destroy(Lexer* lex)
{
    free(lex->tok_str);
    free(lex);
}

Lexer_error lexer_error(const Lexer* lex)
{
    return lex->error;
}

int lexer_has_error(const Lexer* lex)
{
    return lexer_error(lex) != lexer_error_success;
}

Token lexer_token(Lexer* lex)
{
    return lex->token;
}

int lexer_advance(Lexer* lex)
{
    // If the lexer is an error state, do not advance.
    if (lexer_has_error(lex))
        return 0;

    while (_skip_ws(lex)) {
        switch (lex->curr[0]) {
        case '[':
            _store_token(lex, json_token_type_left_bracket, 1);
            return 1;
        case ']':
            _store_token(lex, json_token_type_right_bracket, 1);
            return 1;
        case '{':
            _store_token(lex, json_token_type_left_curly, 1);
            return 1;
        case '}':
            _store_token(lex, json_token_type_right_curly, 1);
            return 1;
        case ',':
            _store_token(lex, json_token_type_comma, 1);
            return 1;
        case ':':
            _store_token(lex, json_token_type_colon, 1);
            return 1;

        case '"': case '\'':
            return _parse_string(lex);

        case '+': case '-': case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7': case '8': case '9':
            return _parse_number(lex);

        case 't':
            if (_match_keyword(lex, "true")) {
                _store_token(lex, json_token_type_true, strlen("true"));
                return 1;
            }
            break;

        case 'f':
            if (_match_keyword(lex, "false")) {
                _store_token(lex, json_token_type_false, strlen("false"));
                return 1;
            }
            break;

        case 'n':
            if (_match_keyword(lex, "null")) {
                _store_token(lex, json_token_type_null, strlen("null"));
                return 1;
            }
            break;

        case '/':
            if (_skip_comment(lex)) {
                if (lexer_has_error(lex))
                    return 0;
                continue;
            }
        }

        if (_match_identifier(lex)) {
            return _parse_identifier(lex);
        }

        lex->error = lexer_error_unexpected_input;
        return 0;
    }

    lex->error = lexer_error_input_exhausted;
    return 0;
}

int lexer_line(const Lexer* lex)
{
    return lex->line;
}

int lexer_column(const Lexer* lex)
{
    return lex->curr - lex->curr_line_start;
}

const char* token_type_to_string(Json_token_type type)
{
    switch (type) {
    case json_token_type_error:
        return "error";
    case json_token_type_null:
        return "null";
    case json_token_type_string:
        return "string";
    case json_token_type_number:
        return "number";
    case json_token_type_true:
        return "true";
    case json_token_type_false:
        return "false";
    case json_token_type_left_bracket:
        return "left_bracket";
    case json_token_type_right_bracket:
        return "right_bracket";
    case json_token_type_left_curly:
        return "left_curly";
    case json_token_type_right_curly:
        return "right_curly";
    case json_token_type_comma:
        return "comma";
    case json_token_type_colon:
        return "colon";
    case json_token_type_identifier:
        return "identifier";
    }
    JSON_PANIC(("Invalid token type: %d", type));
    return NULL; // not reached
}

void fprintf_token(FILE* out, Token tok)
{
    fprintf(out, "token = { %s, ", token_type_to_string(tok.type));
    if (tok.type == json_token_type_string) {
        fprintf(out, "'%s'", tok.value.string);
    } else if (tok.type == json_token_type_number) {
        fprintf(out, "%g", tok.value.number);
    } else {
        fprintf(out, "n/a");
        fprintf(out, " };\n");
    }
}
