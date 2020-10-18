#include "string_vector.h"
#include "string.h"
#include <stdlib.h>

void string_vector_default_construct(void* _str_vec)
{
	string_vector* sv=_str_vec;
	sv->data=0;
	sv->size=0;
	sv->capacity=0;
}

static void string_vector_realloc(string_vector* sv,size_t new_cap)
{
	string* old_data=sv->data;
	string* new_data=malloc(new_cap*sizeof(string));
	string_relocate_n(new_data,old_data,sv->size);
	sv->data=new_data;
	sv->capacity=new_cap;
	free(old_data);
}

void string_vector_emplace_back(string_vector* sv,char const* data,size_t len)
{
	if(sv->size>=sv->capacity)
	{
		string_vector_realloc(sv,2*(sv->capacity+1));
	}
	string_construct(sv->data+sv->size,data,len);
	++sv->size;
}

void string_vector_reserve(string_vector* sv,size_t cap)
{
	if(cap>sv->size)
	{
		string_vector_realloc(sv,cap);
	}
}

void string_vector_push_back_move(string_vector* sv,string* src)
{
	if(sv->size>=sv->capacity)
	{
		string_vector_realloc(sv,2*(sv->capacity+1));
	}
	string_move_construct(sv->data+sv->size,src);
	++sv->size;
}

void string_vector_push_back(string_vector* sv,string const* src)
{
	string_vector_emplace_back(sv,src->data,string_size(src));
}

void string_vector_destruct(string_vector* sv)
{
	for(size_t i=0;i<sv->size;++i)
	{
		string_destruct(sv->data+i);
	}
	free(sv->data);
}

void string_vector_clear(string_vector* sv)
{
	for(size_t i=0;i<sv->size;++i)
	{
		string_destruct(sv->data+i);
	}
	sv->size=0;
}


void string_vector_pop_back(string_vector* sv)
{
	--sv->size;
	string_destruct(sv->data+sv->size);
}

string const* string_vector_cback(string_vector const* sv)
{
	return &sv->data[sv->size-1];
}

string* string_vector_back(string_vector* sv)
{
	return (string*)string_vector_cback(sv);
}
