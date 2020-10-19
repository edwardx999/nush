#include "string.h"
#include <stdlib.h>
#include <string.h>

static char* string_small_data(string const* str)
{
	return ((char*)(&str->_size)) + 1;
}

static int string_is_small(string const* str)
{
	return str->data == string_small_data(str);
}

static void assign_small_size(string* str, size_t len)
{
	unsigned char* sp = (unsigned char*)&str->_size;
	*sp = (unsigned char)len;
}

static size_t const small_cap = (sizeof(size_t) * 2 - 2);

size_t string_size(string const* str)
{
	if (string_is_small(str))
	{
		return (size_t)(*((unsigned char*)&str->_size));
	}
	return str->_size;
}

size_t string_capacity(string const* str)
{
	if (string_is_small(str))
	{
		return small_cap;
	}
	return str->_cap;
}

void string_construct(void* _str, char const* data, size_t len)
{
	string* str = _str;
	str->_size = len;
	if (len < small_cap)
	{
		str->data = ((char*)(&str->_size)) + 1;
	}
	else
	{
		str->data = malloc(len + 1);
	}
	memcpy(str->data, data, len);
	str->data[len] = 0;
}

void string_construct_cstr(void* str, char const* data)
{
	string_construct(str, data, strlen(data));
}

void string_default_construct(void* _str)
{
	string_construct(_str, "", 0);
}

void string_copy_construct(void* dst, string const* src)
{
	string_construct(dst, src->data, string_size(src));
}

void string_copy_assign(string* dst, string const* src)
{
	string_assign(dst, src->data, string_size(src));
}

void string_append(string* str, char const* src, size_t len)
{
	size_t const old_len = string_size(str);
	size_t const new_len = len + old_len;
	string_resize(str, new_len);
	memcpy(str->data + old_len, src, len);
}

void string_assign(string* dst, char const* data, size_t to_copy)
{
	if (data == dst->data) return;
	string_reserve(dst, to_copy);
	dst->_size = to_copy;
	memcpy(dst->data, data, to_copy);
	dst->data[to_copy] = 0;
}

void string_relocate_n(void* _dst, string* src, size_t count)
{
	string* dst = _dst;
	memcpy(dst, src, sizeof(*src) * count);
	for (size_t i = 0; i < count; ++i)
	{
		if (string_is_small(src + i))
		{
			dst[i].data = string_small_data(dst + i);
		}
	}
}

void string_relocate(void* dst, string* src)
{
	string_relocate_n(dst, src, 1);
}

void string_move_construct(void* _dst, string* src)
{
	string* dst = _dst;
	string_relocate(dst, src);
	src->_size = 0;
	src->data = string_small_data(src);
}

void string_move_assign(string* dst, string* src)
{
	string_destruct(dst);
	string_move_construct(dst, src);
}

void string_swap(string* a, string* b)
{
	string temp;
	memcpy(&temp, a, sizeof(*a));
	string_relocate(a, b);
	memcpy(b, &temp, sizeof(*a));
	if (b->data == string_small_data(a))
	{
		b->data = string_small_data(b);
	}
}

void string_destruct(string* str)
{
	if (!string_is_small(str))
	{
		free(str->data);
	}
}

void string_resize(string* str, size_t len)
{
	string_reserve(str, len);
	if (string_is_small(str))
	{
		assign_small_size(str, len);
	}
	else
	{
		str->_size = len;
	}
	str->data[len] = 0;
}

void string_reserve(string* str, size_t cap)
{
	if (cap > string_capacity(str))
	{
		char* new_data = malloc(cap + 1);
		size_t const len = string_size(str);
		memcpy(new_data, str->data, len + 1);
		string_destruct(str);
		str->data = new_data;
		str->_size = len;
		str->_cap = cap;
	}
}

int string_compare(string const* a, string const* b)
{
	return strcmp(a->data, b->data);
}

int string_equal(string const* a, string const* b)
{
	size_t const size = string_size(a);
	return size == string_size(b) && memcmp(a->data, b->data, size) == 0;
}

int string_not_equal(string const* a, string const* b)
{
	size_t const size = string_size(a);
	return size != string_size(b) || memcmp(a->data, b->data, size) != 0;
}