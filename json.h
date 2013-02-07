// Copyright (C) 2003 Daniel Cowgill
//
// Usage of the works is permitted provided that this
// instrument is retained with the works, so that any entity
// that uses the works is notified of this instrument.
//
// DISCLAIMER: THE WORKS ARE WITHOUT WARRANTY.

#ifndef __INCLUDED_LIBJSON_JSON__
#define __INCLUDED_LIBJSON_JSON__

// JSON data types
typedef enum Json_type Json_type;
enum Json_type {
    json_type_null,
    json_type_string,
    json_type_number,
    json_type_bool,
    json_type_object,
    json_type_array,
};

typedef struct Json_value Json_value;
typedef void* Json_iterator;

// Json_value
void json_value_destroy(Json_value* v);
Json_value* json_value_copy(const Json_value* v);
Json_value* json_value_from_cstr(const char* s);
Json_value* json_value_from_double(double n);
Json_value* json_value_from_bool(int b);
Json_value* json_value_new_null();
Json_value* json_value_new_array(int size_hint);
Json_value* json_value_new_object(int size_hint);
void json_value_append(Json_value* array, Json_value* v);
void json_value_set_key(Json_value* object, const char* k, Json_value* v);
Json_type json_value_get_type(const Json_value* v);
int json_value_has_type(const Json_value* v, Json_type type);
const char* json_value_get_cstr(const Json_value* v);
double json_value_get_double(const Json_value* v);
int json_value_get_bool(const Json_value* v);
int json_value_count_members(const Json_value* v);

// Json_iterator
Json_iterator* json_iterator_create(const Json_value* v);
void json_iterator_destroy(Json_iterator* iter);
void json_iterator_advance(Json_iterator* iter);
int json_iterator_is_valid(const Json_iterator* iter);
const char* json_iterator_curr_key(const Json_iterator* iter);
const Json_value* json_iterator_curr_value(const Json_iterator* iter);

// stringification
char* json_stringify(const Json_value* v);

#endif
