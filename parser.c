// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "parser.h"
#include "json.h"
#include "lexer.h"
#include "utilities.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// JSON LL(1) grammar (terminals are UPPERCASE, non-terminals are lowercase):
//
//     value    : STRING | NUMBER | object | array | TRUE | FALSE | NULL
//     object   : '{' members '}'
//     members  : STRING ':' value | members ',' STRING ':' value
//     array    : '[' elements ']'
//     elements : value | elements ',' value

typedef struct Parser Parser;
struct Parser {
    Lexer* lexer;
    Json_parse_error error;
};

// Constructs a new parser error.
static Json_parse_error _create_parse_error()
{
    Json_parse_error e;
    e.code = json_parse_error_success;
    e.line = 0;
    e.column = 0;
    e.expected_token_types[0] = json_token_type_error;
    e.actual_token_type = json_token_type_error;
    return e;
}

// Returns the Json_parse_error_code that corresponds to a Lexer_error.
static Json_parse_error_code _translate_lexer_error(Lexer_error le)
{
    switch (le) {
    case lexer_error_success:
        return json_parse_error_internal;
    case lexer_error_input_exhausted:
        return json_parse_error_unexpected_end_of_input;
    case lexer_error_illegal_escape_sequence:
        return json_parse_error_illegal_escape_sequence;
    case lexer_error_runaway_string:
        return json_parse_error_runaway_string;
    case lexer_error_runaway_comment:
        return json_parse_error_runaway_comment;
    case lexer_error_numeric_overflow:
    case lexer_error_numeric_underflow:
    case lexer_error_invalid_number:
        return json_parse_error_invalid_number;
    case lexer_error_unexpected_input:
        return json_parse_error_unexpected_input;
    }
    assert(0);  // not reached
}

// Sets the given parser's error code and lexical position. Does not touch the
// expected and actual token types.
static void _set_parse_error(Parser* parser, Json_parse_error_code code)
{
    parser->error.code = code;
    parser->error.line = lexer_line(parser->lexer);
    parser->error.column = lexer_column(parser->lexer);
}

// Inspect next token in sequence; return 0 if not of the expected type.
// Expects exactly `n` token types in var_args position.
static int _expect(Parser* parser, int n, ...)
{
    Json_token_type actual = lexer_token(parser->lexer).type;

    if (actual == json_token_type_error) {   // error in lexical analyzer
        int code = _translate_lexer_error(lexer_error(parser->lexer));
        _set_parse_error(parser, code);
        return 0;
    }

    va_list ap;
    int found = 0;
    int i;

    va_start(ap, n);
    assert(n <= json_max_expected_tokens);
    for (i = 0; i < n; i++) {
        Json_token_type expected = va_arg(ap, Json_token_type);
        if (actual == expected) {
            found = 1;
            break;
        }
        parser->error.expected_token_types[i] = expected;
    }
    parser->error.expected_token_types[i] = json_token_type_error;
    va_end(ap);

    if (!found) {
        _set_parse_error(parser, json_parse_error_unexpected_input);
        parser->error.actual_token_type = actual;
        return 0;
    }

    return 1;
}

// Discard next token in sequence; panic if not of the expected type.
static Token _consume(Parser* parser, Json_token_type expected)
{
    Token token = lexer_token(parser->lexer);
    lexer_advance(parser->lexer);
    if (token.type != expected) {
        JSON_PANIC(("Expected token type %s; got %s.",
                    token_type_to_string(expected),
                    token_type_to_string(token.type)));
    }
    return token;
}

// Similar to _consume, but returns 1 if expected token was eaten, else 0.
static int _consume_if_match(Parser* parser, Json_token_type expected)
{
    if (lexer_token(parser->lexer).type == expected) {
        lexer_advance(parser->lexer);  // discard token
        return 1;
    }
    return 0;
}

// This forward declaration is necessary because the parse functions may be
// mutually recursive.
static Json_value* _parse_value(Parser* parser);

static Json_value* _parse_null(Parser* parser)
{
    _consume(parser, json_token_type_null);
    return json_value_new_null();
}

static Json_value* _parse_string(Parser* parser)
{
    return json_value_from_cstr(
        _consume(parser, json_token_type_string).value.string);
}

static Json_value* _parse_number(Parser* parser)
{
    return json_value_from_double(
        _consume(parser, json_token_type_number).value.number);
}

static Json_value* _parse_true(Parser* parser)
{
    _consume(parser, json_token_type_true);
    return json_value_from_bool(1);
}

static Json_value* _parse_false(Parser* parser)
{
    _consume(parser, json_token_type_false);
    return json_value_from_bool(0);
}

static Json_value* _parse_object(Parser* parser)
{
    lexer_advance(parser->lexer);  // advance past left curly

    Json_value* answer = json_value_new_object(0);
    char* key = NULL;

    for (;;) {
        // If the next token is a "}", we're done with this object.
        if (_consume_if_match(parser, json_token_type_right_curly)) {
            goto success;
        }

        if (!_expect(parser,
                     2,
                     json_token_type_identifier,
                     json_token_type_string))
        {
            goto error;
        }

        key = estrdup(lexer_token(parser->lexer).value.string);
        lexer_advance(parser->lexer);

        if (!_expect(parser, 1, json_token_type_colon)) {
            goto error;
        }
        lexer_advance(parser->lexer);

        Json_value* value = _parse_value(parser);
        if (value == NULL) {
            goto error;
        }
        json_value_set_key(answer, key, value);
        free(key);
        key = NULL;

        // Make sure the next token is either a "," or a "}".
        if (!_expect(parser,
                     2,
                     json_token_type_comma,
                     json_token_type_right_curly))
        {
            goto error;
        }

        // If the next token is a ",", discard it.
        _consume_if_match(parser, json_token_type_comma);
    }

error:
    json_value_destroy(answer);
    answer = NULL;

success:
    free(key);

    return answer;
}

static Json_value* _parse_array(Parser* parser)
{
    Json_value* answer;

    lexer_advance(parser->lexer);  // advance past left bracket
    answer = json_value_new_array(0);

    for (;;) {
        // If the next token is a "]", we're done with this object.
        if (_consume_if_match(parser, json_token_type_right_bracket)) {
            goto success;
        }

        Json_value* value = _parse_value(parser);
        if (value == NULL) {
            goto error;
        }
        json_value_append(answer, value);

        // If the next token is a comma, discard it.
        _consume_if_match(parser, json_token_type_comma);
    }

error:
    json_value_destroy(answer);
    answer = NULL;

success:
    return answer;
}

static Json_value* _parse_value(Parser* parser)
{
    Json_token_type token_type;
    int i;

    if (lexer_error(parser->lexer) != lexer_error_success) {
        _set_parse_error(parser,
                         _translate_lexer_error(lexer_error(parser->lexer)));
        return NULL;
    }

    token_type = lexer_token(parser->lexer).type;
    switch (token_type) {
    case json_token_type_null:          return _parse_null(parser);
    case json_token_type_string:        return _parse_string(parser);
    case json_token_type_number:        return _parse_number(parser);
    case json_token_type_true:          return _parse_true(parser);
    case json_token_type_false:         return _parse_false(parser);
    case json_token_type_left_bracket:  return _parse_array(parser);
    case json_token_type_left_curly:    return _parse_object(parser);
    default:                            break; // error
    }

    _set_parse_error(parser, json_parse_error_unexpected_input);

    parser->error.expected_token_types[i=0] = json_token_type_null;
    parser->error.expected_token_types[i++] = json_token_type_string;
    parser->error.expected_token_types[i++] = json_token_type_number;
    parser->error.expected_token_types[i++] = json_token_type_true;
    parser->error.expected_token_types[i++] = json_token_type_false;
    parser->error.expected_token_types[i++] = json_token_type_left_bracket;
    parser->error.expected_token_types[i++] = json_token_type_left_curly;
    parser->error.expected_token_types[i++] = json_token_type_error;

    parser->error.actual_token_type = token_type;
    return NULL;
}

//
// +------------+
// | public API |
// +------------+
//

Json_value* json_parse(const char* input, Json_parse_error* errorp)
{
    Parser parser;
    Json_value* v;

    parser.lexer = lexer_create(input);
    parser.error = _create_parse_error();

    lexer_advance(parser.lexer);
    v = _parse_value(&parser);

    // If the parse succeeded, it's an error for there to be more input.
    if (v && lexer_error(parser.lexer) != lexer_error_input_exhausted) {
        _set_parse_error(&parser, json_parse_error_extraneous_input);
        json_value_destroy(v);
        v = NULL;
    }

    lexer_destroy(parser.lexer);

    // If an error occurred and the caller wants it to be returned via errorp,
    // set errorp; otherwise, destroy the error object we created.

    if (errorp && v == NULL)
        *errorp = parser.error;

    return v;
}

void json_parse_error_print(FILE* fp, const Json_parse_error e)
{
    fprintf(fp, "%s at line %d, column %d",
            json_parse_error_code_to_string(e.code),
            e.line + 1, e.column);

    if (e.expected_token_types[0] != json_token_type_error) {
        int i;
        fprintf(fp, " (expected:");
        for (i = 0; e.expected_token_types[i] != json_token_type_error; i++) {
            Json_token_type token_type = e.expected_token_types[i];
            fputc(' ', fp);
            fputs(token_type_to_string(token_type), fp);
        }
        fputc(')', fp);
    }
    if (e.actual_token_type != json_token_type_error) {
        fprintf(fp, " (got: %s)", token_type_to_string(e.actual_token_type));
    }
}

const char* json_parse_error_code_to_string(Json_parse_error_code e)
{
    switch (e) {
    case json_parse_error_success:
        return "success";
    case json_parse_error_internal:
        return "internal";
    case json_parse_error_unexpected_input:
        return "unexpected input";
    case json_parse_error_unexpected_end_of_input:
        return "unexpected end of input";
    case json_parse_error_illegal_escape_sequence:
        return "illegal escape sequence";
    case json_parse_error_runaway_string:
        return "runaway string";
    case json_parse_error_runaway_comment:
        return "runaway comment";
    case json_parse_error_invalid_number:
        return "invalid number";
    case json_parse_error_extraneous_input:
        return "extraneous input";
    }
    JSON_PANIC(("invalid parse error code: %d", e));
    return 0;
}
