#include "int_vector.h"
#include <string.h>

void int_vector_default_construct(void* _int_vec)
{
	int_vector* int_vec=_int_vec;
	int_vec->data=0;
	int_vec->size=0;
	int_vec->capacity=0;
}

void size_t_vector_default_construct(void* st_vec)
{
	int_vector_default_construct(st_vec);
}

static void pod_vector_realloc(void* _vec,size_t new_cap,size_t elem_size)
{
	int_vector* int_vec=_vec;
	void* old_data=int_vec->data;
	void* new_data=malloc(new_cap*elem_size);
	memcpy(new_data,old_data,int_vec->size*elem_size);
	int_vec->data=new_data;
	int_vec->capacity=new_cap;
	free(old_data);
}

static void int_vector_realloc(int_vector* int_vec,size_t new_cap)
{
	pod_vector_realloc(int_vec,new_cap,sizeof(int));
}

void int_vector_push_back(int_vector* int_vec,int val)
{
	if(int_vec->size>=int_vec->capacity)
	{
		int_vector_realloc(int_vec,2*(int_vec->capacity+4));
	}
	int_vec->data[int_vec->size]=val;
	++int_vec->size;
}

static void size_t_vector_realloc(size_t_vector* size_t_vec,size_t new_cap)
{
	pod_vector_realloc(size_t_vec,new_cap,sizeof(size_t));
}

void size_t_vector_push_back(size_t_vector* size_t_vec,size_t val)
{
	if(size_t_vec->size>=size_t_vec->capacity)
	{
		size_t_vector_realloc(size_t_vec,2*(size_t_vec->capacity+2));
	}
	size_t_vec->data[size_t_vec->size]=val;
	++size_t_vec->size;
}

