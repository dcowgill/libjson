// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#include "json.h"
#include "str.h"
#include "table.h"
#include "utilities.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

struct Json_value {
    Json_type type;
    union {
        char* string;       // json_type_string
        double number;      // json_type_number
        int bool;           // json_type_bool
        Table* table;       // json_type_object or json_type_array
    } variant;
};

struct Json_iterator {
    Json_value* value;      // value being iterated over
    int pos;                // current position in iteration
};

//
// +--------------------+
// | internal functions |
// +--------------------+
//

static Json_value* _json_new_value(Json_type type)
{
    Json_value* v = (Json_value*) emalloc(sizeof(Json_value));
    v->type = type;
    return v;
}

static Json_value* _json_copy_array(const Json_value* v)
{
    assert(json_value_has_type(v, json_type_array));

    Json_value* new_array =
        json_value_new_array(json_value_count_members(v));
    Json_iterator* i = json_iterator_create(v);
    for (; json_iterator_is_valid(i); json_iterator_advance(i)) {
        Json_value* value_copy = json_value_copy(json_iterator_curr_value(i));
        json_value_append(new_array, value_copy);
    }
    json_iterator_destroy(i);
    return new_array;
}

static Json_value* _json_copy_object(const Json_value* v)
{
    assert(json_value_has_type(v, json_type_object));

    Json_value* new_object =
        json_value_new_object(json_value_count_members(v));
    Json_iterator* i = json_iterator_create(v);
    for (; json_iterator_is_valid(i); json_iterator_advance(i)) {
        Json_value* value_copy = json_value_copy(json_iterator_curr_value(i));
        json_value_set_key(new_object, json_iterator_curr_key(i), value_copy);
    }
    json_iterator_destroy(i);
    return new_object;
}

//
// +------------+
// | Json_value |
// +------------+
//

void json_value_destroy(Json_value* v)
{
    if (!v) return;

    switch (json_value_get_type(v)) {
    case json_type_string:
        free(v->variant.string);
        break;
    case json_type_object:
    case json_type_array:
        table_destroy(v->variant.table, (void (*)(void*)) json_value_destroy);
        break;
    default:
        // no cleanup necessary for other types
        break;
    }
    free(v);
}

Json_value* json_value_copy(const Json_value* v)
{
    switch (json_value_get_type(v)) {
    case json_type_null:
        return json_value_new_null();
    case json_type_string:
        return json_value_from_cstr(json_value_get_cstr(v));
    case json_type_number:
        return json_value_from_double(json_value_get_double(v));
    case json_type_bool:
        return json_value_from_bool(json_value_get_bool(v));
    case json_type_object:
        return _json_copy_object(v);
    case json_type_array:
        return _json_copy_array(v);
    }
    assert(0);  // should not be reached
}

Json_value* json_value_new_null()
{
    return _json_new_value(json_type_null);
}

Json_value* json_value_from_cstr(const char* s)
{
    Json_value* v = _json_new_value(json_type_string);
    v->variant.string = estrdup(s);
    return v;
}

Json_value* json_value_from_double(double n)
{
    Json_value* v = _json_new_value(json_type_number);
    v->variant.number = n;
    return v;
}

Json_value* json_value_from_bool(int b)
{
    Json_value* v = _json_new_value(json_type_bool);
    v->variant.bool = b ? 1 : 0;  // coerce to [0,1]
    return v;
}

Json_value* json_value_new_array(int size_hint)
{
    Json_value* v = _json_new_value(json_type_array);
    v->variant.table = table_create(size_hint);
    return v;
}

Json_value* json_value_new_object(int size_hint)
{
    Json_value* v = _json_new_value(json_type_object);
    v->variant.table = table_create(size_hint);
    return v;
}

void json_value_append(Json_value* array, Json_value* v)
{
    assert(json_value_has_type(array, json_type_array));
    table_set_key(&array->variant.table, NULL, v);
}

void json_value_set_key(Json_value* object, const char* k, Json_value* v)
{
    assert(json_value_has_type(object, json_type_object));
    json_value_destroy(table_set_key(&object->variant.table, k, v));
}

Json_type json_value_get_type(const Json_value* v)
{
    assert(v);
    return v->type;
}

int json_value_has_type(const Json_value* v, Json_type type)
{
    return json_value_get_type(v) == type;
}

const char* json_value_get_cstr(const Json_value* v)
{
    assert(json_value_has_type(v, json_type_string));
    return v->variant.string;
}

double json_value_get_double(const Json_value* v)
{
    assert(json_value_has_type(v, json_type_number));
    return v->variant.number;
}

int json_value_get_bool(const Json_value* v)
{
    assert(json_value_has_type(v, json_type_bool));
    return v->variant.bool;
}

int json_value_count_members(const Json_value* v)
{
    assert(json_value_has_type(v, json_type_array) ||
           json_value_has_type(v, json_type_object));

    return table_get_size(v->variant.table);
}

//
// +---------------+
// | Json_iterator |
// +---------------+
//

Json_iterator* json_iterator_create(const Json_value* v)
{
    assert(json_value_has_type(v, json_type_array) ||
           json_value_has_type(v, json_type_object));

    return (Json_iterator*) table_iterator_create(v->variant.table);
}

void json_iterator_destroy(Json_iterator* iter)
{
    table_iterator_destroy((Table_iterator*) iter);
}

void json_iterator_advance(Json_iterator* iter)
{
    table_iterator_advance((Table_iterator*) iter);
}

int json_iterator_is_valid(const Json_iterator* iter)
{
    return table_iterator_is_valid((Table_iterator*) iter);
}

const char* json_iterator_curr_key(const Json_iterator* iter)
{
    return table_iterator_curr_key((Table_iterator*) iter);
}

const Json_value* json_iterator_curr_value(const Json_iterator* iter)
{
    return (const Json_value*)
        table_iterator_curr_value((Table_iterator*) iter);
}

//
// +-----------------+
// | stringification |
// +-----------------+
//

// Forward declarations are needed because our stringification routines are
// mutually recursive.
static void _json_stringify_array (Str** dst, const Json_value* v);
static void _json_stringify_object(Str** dst, const Json_value* v);

// Escape control characters (and various other characters, such as double
// quotes) in the given string. Returns a newly allocated string.
static char* _escape_string(const char* s)
{
    Str* t = str_create(strlen(s));
    const char* p;

#define ESC_SEQ(c) { str_append_char(&t, '\\'); str_append_char(&t, c); }

    for (p = s; *p != '\0'; p++) {
        switch (*p) {
        case '\b':
            ESC_SEQ('b');
            break;
        case '\f':
            ESC_SEQ('f');
            break;
        case '\n':
            ESC_SEQ('n');
            break;
        case '\r':
            ESC_SEQ('r');
            break;
        case '\t':
            ESC_SEQ('t');
            break;
        case '"':
            ESC_SEQ('"');
            break;
        default:
            str_append_char(&t, *p);
            break;
        }
    }

#undef ESC_SEQ

    return str_destroy_and_copy(t);
}

// Converts a double to text, returning a newly allocated string.
static char* _double_to_string(double d)
{
    int size = 30;      // guess at maximum size
    char* p = (char*) emalloc(size);

    for (;;) {
        int n = snprintf(p, size, "%g", d);

        if (n >= 0 && n < size) {
            break;      // success
        }

        if (n >= 0) {
            size = n+1; // we know the exact size
        } else {
            size *= 2;  // obsolete snprintf, have to guess
        }

        p = (char*) erealloc(p, size);
    }
    return p;
}

// Converts a json value to text, appending it to `dst`.
static void _json_stringify(Str** dst, const Json_value* v)
{
    char* s;

    switch (json_value_get_type(v)) {
    case json_type_null:
        str_append_cstr(dst, "null");
        break;
    case json_type_string:
        str_append_char(dst, '"');
        s = _escape_string(json_value_get_cstr(v));
        str_append_cstr(dst, s);
        free(s);
        str_append_char(dst, '"');
        break;
    case json_type_number: {
        char* s = _double_to_string(json_value_get_double(v));
        str_append_cstr(dst, s);
        free(s);
        break;
    }
    case json_type_bool:
        str_append_cstr(dst, json_value_get_bool(v) ? "true" : "false");
        break;
    case json_type_object:
        _json_stringify_object(dst, v);
        break;
    case json_type_array:
        _json_stringify_array(dst, v);
        break;
    }
}

// Converts a json array to text, appending it to `dst`.
static void _json_stringify_array(Str** dst, const Json_value* v)
{
    str_append_char(dst, '[');
    int first = 1;
    Json_iterator* i = json_iterator_create(v);
    for (; json_iterator_is_valid(i); json_iterator_advance(i)) {
        if (first) {
            first = 0;
        } else {
            str_append_cstr(dst, ", ");
        }
        _json_stringify(dst, json_iterator_curr_value(i));
    }
    json_iterator_destroy(i);
    str_append_char(dst, ']');
}

// Converts a json object to text, appending it to `dst`.
static void _json_stringify_object(Str** dst, const Json_value* v)
{
    str_append_char(dst, '{');
    Json_iterator* i = json_iterator_create(v);
    int first = 1;
    for (; json_iterator_is_valid(i); json_iterator_advance(i)) {
        if (first) {
            first = 0;
        } else {
            str_append_cstr(dst, ", ");
        }
        str_append_char(dst, '"');
        str_append_cstr(dst, json_iterator_curr_key(i));
        str_append_char(dst, '"');
        str_append_char(dst, ':');
        _json_stringify(dst, json_iterator_curr_value(i));
    }
    json_iterator_destroy(i);
    str_append_char(dst, '}');
}

char* json_stringify(const Json_value* v)
{
    Str* s = str_create(100);
    _json_stringify(&s, v);
    char* answer = estrdup(str_cstr(s));
    str_destroy(s);
    return answer;
}
