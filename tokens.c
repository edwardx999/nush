#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "string_vector.h"
#include "tokens.h"
#include <assert.h>

int read_line(string* dst, FILE* file)
{
	while (1)
	{
		size_t const clen = string_size(dst);
		size_t const cap_left = string_capacity(dst) - clen;
		char* const data = dst->data;
		char* res = fgets(data + clen, cap_left + 1, file);
		if (res == 0)
		{
			return 1;
		}
		size_t const rlen = strlen(res);
		if (rlen == 0)
		{
			return 1;
		}
		size_t new_len;
		if (res[rlen - 1] == '\n')
		{
			if (rlen >= 2 && res[rlen - 2] == '\\')
			{
				new_len = clen + rlen - 2;
			}
			else
			{
				string_resize(dst, clen + rlen - 1);
				//fseek(stdin,ftell(stdin),SEEK_SET);
				return 0;
			}
		}
		else
		{
			new_len = clen + rlen;
		}
		string_resize(dst, new_len);
		string_reserve(dst, 2 * new_len);
	}
}

static char const* after_spaces(char const* start)
{
	while (1)
	{
		if (*start == 0 || !isspace(*start))
		{
			return start;
		}
		++start;
	}
}

static void push_string_skip_quotes(string_vector* vec, char const* start, char const* end)
{
	string str;
	string_default_construct(&str);
	string_reserve(&str, end - start);
	size_t actual_len = 0;
	for (; start < end; ++start)
	{
		if (*start != '"')
		{
			str.data[actual_len] = *start;
			++actual_len;
		}
	}
	string_resize(&str, actual_len);
	string_vector_push_back_move(vec, &str);
}

void tokenize(string_vector* out, char const* line)
{
	char c;
	line = after_spaces(line);
	int in_quotes = 0;
	op_ident op_id;
	char const* token_start = line;
	while (c = *line)
	{
		if (!in_quotes)
		{
			if (c == '"')
			{
				in_quotes = 1;
				++line;
			}
			else if (isspace(c))
			{
				push_string_skip_quotes(out, token_start, line);
				line = after_spaces(line + 1);
				token_start = line;
			}
			else if (((op_id = identify_operator(line)), op_id.code != op_INVALID))
			{
				if (token_start != line)
				{
					push_string_skip_quotes(out, token_start, line);
				}
				string operator;
				operator.data = 0;
				operator._size = op_id.code;
				string_vector_push_back_move(out, &operator);
				line = after_spaces(op_id.end);
				token_start = line;
			}
			else
			{
				++line;
			}
		}
		else
		{
			if (c == '"')
			{
				in_quotes = 0;
			}
			++line;
		}
	}
	if (token_start != line)
	{
		push_string_skip_quotes(out, token_start, line);
	}
}

int operator_precedence(op_code code)
{
	int const start = -__COUNTER__;
	switch (code)
	{
	case op_semicolon:
	case op_background:
	case op_paren:
		return start + __COUNTER__;
	case op_and:
	case op_or:
		return start + __COUNTER__;
	case op_pipe:
	case op_redirect_input:
	case op_redirect_output:
		return start + __COUNTER__;
	}
	assert(("Invalid Operator", 0));
}

size_t operator_arity(op_code code)
{
	switch (code)
	{
	case op_and:
	case op_or:
	case op_pipe:
	case op_redirect_input:
	case op_redirect_output:
		return 2;
	default:
		return 1;
	}
}

operator_string const* get_operator_strings()
{
	static operator_string operators[op_COUNT] = {
		{{},0},
		{"&&",2},
		{"||",2},
		{"|",1},
		{"<",1},
		{">",1},
		{";",1},
		{"&",1},
		{"(",1},
		{")",1},
	};
	return operators;
}

op_ident identify_operator(char const* str)
{
	op_ident ret;
	operator_string const* ops = get_operator_strings();
	for (size_t i = 1; i < op_COUNT; ++i)
	{
		size_t const op_len = ops[i].size;
		for (size_t j = 0; j < op_len; ++j)
		{
			if (ops[i].data[j] != str[j])
			{
				goto search_next;
			}
		}
		ret.code = (op_code)i;
		ret.end = op_len + str;
		return ret;
	search_next:;
	}
	ret.code = op_INVALID;
	ret.end = str;
	return ret;
}



