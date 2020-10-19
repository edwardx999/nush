#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "string_vector.h"

typedef enum op_codes {
	op_INVALID,
	op_and,
	op_or,
	op_pipe,
	op_redirect_input,
	op_redirect_output,
	op_semicolon,
	op_background,
	op_paren,
	op_start_paren = op_paren,
	op_end_paren,
	op_COUNT,
	op_execute,
	op_subshell = op_paren
} op_code;

int read_line(string* dst, FILE* file);

/*
	Turn a c-string into a vector of tokens; operators are represented as data being 0, and _size being op_code.
*/
void tokenize(string_vector* out, char const* line);

int operator_precedence(op_code code);

size_t operator_arity(op_code code);

typedef struct operator_string {
	char const data[3];
	unsigned char size;
} operator_string;

operator_string const* get_operator_strings();

typedef struct op_identification {
	op_code code;
	char const* end;
} op_ident;

op_ident identify_operator(char const* str);

/* TODO:

   while (1) {
	 printf("tokens$ ");
	 fflush(stdout);
	 line = read_line()
	 if (that was EOF) {
		exit(0);
	 }
	 tokens = tokenize(line);
	 foreach token in reverse(tokens):
	   puts(token)
   }

*/




