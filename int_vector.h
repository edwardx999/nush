#pragma once
#ifndef INT_VECTOR_H
#define INT_VECTOR_H
#include <stddef.h>
#include <stdlib.h>

typedef struct int_vector {
	int* data;
	size_t size;
	size_t capacity;
} int_vector;

typedef struct size_t_vector {
	size_t* data;
	size_t size;
	size_t capacity;
} size_t_vector;

void int_vector_default_construct(void* int_vec);

void size_t_vector_default_construct(void* str_vec);

void int_vector_push_back(int_vector*, int val);

void size_t_vector_push_back(size_t_vector*, size_t val);

static void int_vector_destruct(int_vector* iv)
{
	free(iv->data);
}

static void int_vector_clear(int_vector* iv)
{
	iv->size = 0;
}

static void int_vector_pop_back(int_vector* iv)
{
	--iv->size;
}

static int int_vector_back(int_vector const* iv)
{
	return iv->data[iv->size - 1];
}

static void size_t_vector_destruct(size_t_vector* iv)
{
	free(iv->data);
}

static void size_t_vector_clear(size_t_vector* iv)
{
	iv->size = 0;
}

static void size_t_vector_pop_back(size_t_vector* iv)
{
	--iv->size;
}

static size_t size_t_vector_back(size_t_vector const* iv)
{
	return iv->data[iv->size - 1];
}


#endif