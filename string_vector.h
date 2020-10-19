#ifndef STRING_VECTOR_H
#define STRING_VECTOR_H
#include "string.h"

typedef struct string_vector {
	string* data;
	size_t size;
	size_t capacity;
} string_vector;

void string_vector_default_construct(void* str_vec);

void string_vector_emplace_back(string_vector*, char const* data, size_t len);

void string_vector_push_back_move(string_vector*, string* src);

void string_vector_push_back(string_vector*, string const* src);

void string_vector_destruct(string_vector*);

void string_vector_clear(string_vector*);

void string_vector_pop_back(string_vector*);

void string_vector_reserve(string_vector*, size_t cap);

// give me references...
string const* string_vector_cback(string_vector const*);

string* string_vector_back(string_vector*);
#endif