#ifndef STRING_H
#define STRING_H
#include <stddef.h>

typedef struct string {
	char* data;
	size_t _size;
	size_t _cap;
} string;

static int streq(char const* a,char const* b)
{
	size_t c=0;
	while(1)
	{
		if(a[c]==0) return b[c]==0;
		if(a[c]!=b[c]) return 0;
		++c;
	}
}

void string_construct(void* str,char const* data,size_t len);

void string_construct_cstr(void* str,char const* data);

void string_default_construct(void* str);

void string_copy_construct(void* dst,string const* src);

void string_copy_assign(string* dst,string const* src);

void string_assign(string* dst,char const* src,size_t len);

void string_relocate_n(void* dst,string* src,size_t count);

void string_relocate(void* dst,string* src);

void string_append(string* str,char const* src,size_t len);

void string_move_construct(void* dst,string* src);

void string_move_assign(string* dst,string* src);

void string_swap(string* a,string* b);

void string_destruct(string* str);

void string_resize(string*,size_t len);

void string_reserve(string*,size_t cap);

size_t string_size(string const* str);

size_t string_capacity(string const* str);

static char* string_data(string* str)
{
	return str->data;
}

static char const* string_cdata(string const* str)
{
	return str->data;
}

int string_compare(string const* a,string const* b);

int string_equal(string const* a,string const* b);

int string_not_equal(string const* a,string const* b);

#endif