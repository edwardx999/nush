#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include "tokens.h"
#include "int_vector.h"

int_vector unwatched_children;

void dump_rp(string_vector const* reverse_polish)
{
#ifdef DEBUGGING
	puts("-------------------------------------------------");
	for (size_t i = 0; i < reverse_polish->size; ++i)
	{
		if (reverse_polish->data[i].data)
		{
			printf("%s\n", reverse_polish->data[i].data);
		}
		else
		{
			printf("%ld %ld\n", reverse_polish->data[i]._size, reverse_polish->data[i]._cap);
		}
	}
	puts("-------------------------------------------------");
#endif
}

void dump_op_stack(int_vector const* stack)
{
#ifdef DEBUGGING
	puts("-------------------------------------------------");
	for (size_t i = 0; i < stack->size; ++i)
	{
		printf("%d\n", stack->data[i]);
	}
	puts("-------------------------------------------------");
#endif
}

void kill_children()
{
	for (size_t i = 0; i < unwatched_children.size; ++i)
	{
		kill(unwatched_children.data[i], SIGKILL);
	}
}

void wait_for_children()
{
	for (size_t i = 0; i < unwatched_children.size; ++i)
	{
		int status;
		waitpid(unwatched_children.data[i], &status, 0);
	}
}

char const* get_chdir_err(int err)
{
	switch (err)
	{
	case EACCES:
		return "Access denied: %s\n";
	case ELOOP:
		return "Symbolic link loop: %s\n";
	case ENAMETOOLONG:
		return "Name too long: %s\n";
	case ENOENT:
		return "Path does not exist: %s\n";
	case ENOTDIR:
		return "Path is not a dir: %s\n";
	default:
		return "Unknown error: %s\n";
	}
}

size_t find_expression_end(string const* tokens, size_t begin, size_t end)
{
	for (; begin < end; ++begin)
	{
		if (tokens[begin].data == 0)
		{
			return begin;
		}
	}
	return begin;
}

void push_back_binary_operator(string_vector* out, op_code op)
{
	string op_to_push;
	op_to_push.data = 0;
	op_to_push._size = op;
	size_t const arg2_size = string_vector_cback(out)->_cap;
	size_t const arg1_location = out->size - arg2_size - 1;
	size_t const arg1_size = out->data[arg1_location]._cap;
	op_to_push._cap = arg1_size + arg2_size + 1;
	string_vector_push_back_move(out, &op_to_push);
}

void push_back_unary_operator(string_vector* out, op_code op, size_t arg_count)
{
	if (arg_count)
	{
		string op_to_push;
		op_to_push.data = 0;
		op_to_push._size = op;
		op_to_push._cap = arg_count + 1;
		string_vector_push_back_move(out, &op_to_push);
	}
}

/*
	Create a "tree" out the tokens in reverse polish notation. Operators are marked by
	data=0, _size=op_code, and _capacity=subtree_size (including operator). The rest are tokens (normal string s).
	& and ; are interpreted as unary operators exclusively.
	; is appended to the reverse polish execution. To execute this "tree", start from the end, read the operator subtree
	sizes to get where the arguments are, and recur.
	I've implemented this algorithm (shunting-yard) before, so this is based on a C++ version at https://github.com/edwardx999/Exlib
	Although I got rid of error checking, because no one needs that.
*/
void to_reverse_polish(string_vector* tokens, string_vector* out, int_vector* operator_stack, size_t_vector* arity_stack)
{
	string_vector_clear(out);
	int_vector_clear(operator_stack);
	size_t_vector_clear(arity_stack); // keep track of number of statements in parentheses
	size_t_vector_push_back(arity_stack, 0);
	string* data = tokens->data;
	size_t const size = tokens->size;
	size_t start = 0;
	for (; start < size;)
	{
		size_t next = find_expression_end(data, start, size);
		for (size_t j = start; j < next; ++j)
		{
			string_vector_push_back_move(out, &data[j]);
		}
		size_t const unary_arg_count = next - start;
		push_back_unary_operator(out, op_execute, unary_arg_count);
		if (next >= size)
		{

			break;
		}
		op_code op_found = (op_code)data[next]._size;
		if (op_found == op_start_paren)
		{
			int_vector_push_back(operator_stack, op_start_paren);
			//dump_op_stack(operator_stack);
			size_t_vector_push_back(arity_stack, 0);
		}
		else if (op_found == op_end_paren)
		{
			while (operator_stack->size)
			{
				op_code op_top = (op_code)int_vector_back(operator_stack);
				if (op_top == op_start_paren)
				{
					int_vector_pop_back(operator_stack);
					break;
				}
				push_back_binary_operator(out, op_top);
				int_vector_pop_back(operator_stack);
				//dump_op_stack(operator_stack);
			}
			size_t tree_size = 0;
			{
				size_t const arity = size_t_vector_back(arity_stack) + !!unary_arg_count;
				size_t_vector_pop_back(arity_stack);
				string const* pos = string_vector_cback(out);
				for (size_t i = 0; i < arity; ++i)
				{
					size_t const subtree_size = pos->_cap;
					tree_size += subtree_size;
					pos -= subtree_size;
				}
			}
			push_back_unary_operator(out, op_subshell, tree_size);
		}
		else if (op_found == op_semicolon || op_found == op_background)
		{
			++arity_stack->data[arity_stack->size - 1];
			while (operator_stack->size)
			{
				op_code op_top = (op_code)int_vector_back(operator_stack);
				if (op_top == op_start_paren)
				{
					break;
				}
				push_back_binary_operator(out, int_vector_back(operator_stack));
				int_vector_pop_back(operator_stack);
				//dump_op_stack(operator_stack);
			}
			if (op_found == op_background)
			{
				push_back_unary_operator(out, op_found, string_vector_cback(out)->_cap);
			}
		}
		else
		{
			while (operator_stack->size)
			{
				op_code op_top = (op_code)int_vector_back(operator_stack);
				if (operator_precedence(op_found) > operator_precedence(op_top))
				{
					break;
				}
				push_back_binary_operator(out, op_top);
				int_vector_pop_back(operator_stack);
			}
			//dump_op_stack(operator_stack);
			int_vector_push_back(operator_stack, op_found);
			//dump_op_stack(operator_stack);
		}
		start = next + 1;
	}
	//dump_rp(out);
	//dump_op_stack(operator_stack);
	while (operator_stack->size)
	{
		push_back_binary_operator(out, int_vector_back(operator_stack));
		int_vector_pop_back(operator_stack);
	}
	tokens->size = 0;
}

int exec_reverse_polish(string const* reverse_polish, size_t count, size_t_vector* work_space);

int exec_reverse_polish_help(string const* last);

typedef struct builtin_ret {
	int execed;
	int ret_code;
} builtin_ret;

size_t find_equal_sign(char const* data)
{
	size_t i = 0;
	char c;
	while ((c = data[i]))
	{
		if (c == '=')
		{
			return i;
		}
		++i;
	}
	return -1;
}

builtin_ret exec_builtins(string const* tokens, size_t count)
{
	size_t equal_sign_loc;
	if (streq(tokens->data, "exit"))
	{
		exit(0);
	}
	else if (streq(tokens->data, "cd"))
	{
		if (count > 2)
		{
			fprintf(stderr, "Too many arguments\n");
			builtin_ret ret = { 1,1 };
			return ret;
		}
		char const* dir = tokens[1].data;
		if (chdir(dir))
		{
			int err = errno;
			fprintf(stderr, get_chdir_err(err), dir);
			builtin_ret ret = { 1,err };
			return ret;
		}
		builtin_ret ret = { 1,0 };
		return ret;
	}
	else if (streq(tokens->data, "true"))
	{
		builtin_ret ret = { 1,0 };
		return ret;
	}
	else if (streq(tokens->data, "false"))
	{
		builtin_ret ret = { 1,1 };
		return ret;
	}
	else if ((equal_sign_loc = find_equal_sign(tokens->data)) != -1)
	{
		string key; string_construct(&key, tokens->data, equal_sign_loc);
		setenv(string_data(&key), tokens->data + equal_sign_loc + 1, 1);
		string_destruct(&key);
		builtin_ret ret = { 1,0 };
		return ret;
	}
	builtin_ret ret = { 0 };
	return ret;
}

void exec_direct_no_builtins(string const* tokens, size_t count)
{
	char* args[count + 1];
	for (size_t i = 0; i < count; ++i)
	{
		char* value = getenv(tokens[i].data);
		if (value)
		{
			args[i] = value;
		}
		else
		{
			args[i] = tokens[i].data;
		}
	}
	args[count] = 0;
	execvp(args[0], args);
}

int exec_one_command(string const* tokens, size_t count)
{
	{
		builtin_ret const r = exec_builtins(tokens, count);
		if (r.execed)
		{
			return r.ret_code;
		}
	}
	int const cpid = fork();
	if (cpid)
	{
		int status;
		waitpid(cpid, &status, 0);
		return status;
	}
	else
	{
		int_vector_clear(&unwatched_children);
		exec_direct_no_builtins(tokens, count);
	}
}

int exec_pipe(string const* last)
{
	int pipe_ends[2];
	pipe(pipe_ends);
	int const read_end = pipe_ends[0];
	int const write_end = pipe_ends[1];
	int const writer_id = fork();
	if (writer_id)
	{
		close(write_end);
		int reader_id = fork();
		if (reader_id) // in parent
		{
			close(read_end);
			int status;
			waitpid(writer_id, 0, 0);
			waitpid(reader_id, &status, 0);
			return WEXITSTATUS(status);
		}
		else // in reader
		{
			close(0);
			dup(read_end);
			int_vector_clear(&unwatched_children);
			exit(exec_reverse_polish_help(last - 1));
		}
	}
	else
	{
		// in writer
		close(read_end);
		close(1);
		dup(write_end);
		size_t reader_arg_tree_size = (last - 1)->_cap;
		int_vector_clear(&unwatched_children);
		exit(exec_reverse_polish_help(last - reader_arg_tree_size - 1));
	}
}

int exec_redirect_input(string const* last)
{
	size_t const in_tree_size = (last - 1)->_cap;
	int fd = open((last - in_tree_size)->data, O_RDONLY);
	if (fd < 0)
	{
		return 1;
	}
	int const cpid = fork();
	if (cpid) //parent
	{
		close(fd);
		int status;
		waitpid(cpid, &status, 0);
		return WEXITSTATUS(status);
	}
	else
	{
		close(0);
		dup(fd);
		size_t reader_arg_tree_size = (last - 1)->_cap;
		int_vector_clear(&unwatched_children);
		exit(exec_reverse_polish_help(last - reader_arg_tree_size - 1));
	}
}

int exec_redirect_output(string const* last)
{
	size_t const in_tree_size = (last - 1)->_cap;
	int const fd = open((last - in_tree_size)->data, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (fd < 0)
	{
		return 1;
	}
	int const cpid = fork();
	if (cpid) // parent
	{
		close(fd);
		int status;
		waitpid(cpid, &status, 0);
		return WEXITSTATUS(status);
	}
	else
	{
		close(1);
		dup(fd);
		size_t const reader_arg_tree_size = (last - 1)->_cap;
		int_vector_clear(&unwatched_children);
		exit(exec_reverse_polish_help(last - reader_arg_tree_size - 1));
	}
}

int spawn_subshell(string const* last)
{
	int const cpid = fork();
	if (cpid)
	{
		return cpid;
	}
	else
	{
		int_vector_clear(&unwatched_children);
		size_t_vector work_space; size_t_vector_default_construct(&work_space);
		size_t const subtree_size = last->_cap - 1;
		exit(exec_reverse_polish(last - subtree_size, subtree_size, &work_space));
	}
}

// return the return code
int exec_reverse_polish_help(string const* last)
{
	assert(last->data == 0);
	size_t const subtree_size = last->_cap;
	op_code const op = (op_code)last->_size;
	switch (op)
	{
	case op_and:
	{
		size_t const reader_arg_tree_size = (last - 1)->_cap;
		int const ret = exec_reverse_polish_help(last - reader_arg_tree_size - 1);
		if (!ret)
		{
			return exec_reverse_polish_help(last - 1);
		}
		return ret;
	}
	case op_or:
	{
		size_t const reader_arg_tree_size = (last - 1)->_cap;
		int const ret = exec_reverse_polish_help(last - reader_arg_tree_size - 1);
		if (ret)
		{
			return exec_reverse_polish_help(last - 1);
		}
		return ret;
	}
	case op_pipe:
		return exec_pipe(last);
	case op_redirect_input:
		return exec_redirect_input(last);
	case op_redirect_output:
		return exec_redirect_output(last);
	case op_semicolon:
		return exec_reverse_polish_help(last - 1);
	case op_background:
	{
		int const id = fork();
		if (id)
		{
			int_vector_push_back(&unwatched_children, id);
			return 0;
		}
		else
		{
			close(0); // don't let child take my stdin
			int_vector_clear(&unwatched_children);
			exit(exec_reverse_polish_help(last - 1));
		}
	}
	case op_execute:
		return exec_one_command(last - subtree_size + 1, subtree_size - 1);
	case op_subshell:
	{
		int status;
		int const pid = spawn_subshell(last);
		waitpid(pid, &status, 0);
		return WEXITSTATUS(status);
	}
	}
}

int exec_reverse_polish(string const* reverse_polish, size_t count, size_t_vector* work_space)
{
	size_t_vector indices = *work_space;
	size_t_vector_clear(&indices);
	for (size_t end = count; end > 0;)
	{
		size_t_vector_push_back(&indices, end);
		string const* last = reverse_polish + end - 1;
		end -= last->_cap;
	}
	string const* before_start = reverse_polish - 1;
	int ret = 0;
	for (size_t i = indices.size; i-- > 0;)
	{
		ret = exec_reverse_polish_help(before_start + indices.data[i]);
	}
	*work_space = indices;
	return ret;
}

int main(int argc, char** argv)
{
	atexit(wait_for_children);
	string_vector tokens; string_vector_default_construct(&tokens);
	string_vector reverse_polish; string_vector_default_construct(&reverse_polish);
	int_vector stack; int_vector_default_construct(&stack);
	size_t_vector work_space; size_t_vector_default_construct(&work_space);
	if (argc == 1)
	{
		string line; string_default_construct(&line);
		char const message[] = "nush$ ";
		while (1)
		{
			fwrite(message, sizeof(message) - 1, 1, stdout);
			fflush(stdout);
			if (read_line(&line, stdin))
			{
				break;
			}
			tokenize(&tokens, line.data);
			to_reverse_polish(&tokens, &reverse_polish, &stack, &work_space);
			//dump_rp(&reverse_polish);
			exec_reverse_polish(reverse_polish.data, reverse_polish.size, &work_space);
			string_resize(&line, 0);
		}
		string_destruct(&line);
	}
	else
	{
		string_vector lines; string_vector_default_construct(&lines);
		FILE* const file = fopen(argv[1], "r");
		while (1)
		{
			string line; string_default_construct(&line);
			if (read_line(&line, file))
			{
				string_destruct(&line);
				break;
			}
			string_vector_push_back_move(&lines, &line);
		}
		fclose(file);
		for (size_t i = 0; i < lines.size; ++i)
		{
			string const* line = lines.data + i;
			tokenize(&tokens, line->data);
			//dump_rp(&tokens);
			to_reverse_polish(&tokens, &reverse_polish, &stack, &work_space);
			//dump_rp(&reverse_polish);
			exec_reverse_polish(reverse_polish.data, reverse_polish.size, &work_space);
		}
		string_vector_destruct(&lines);
	}
	size_t_vector_destruct(&work_space);
	int_vector_destruct(&stack);
	string_vector_destruct(&reverse_polish);
	string_vector_destruct(&tokens);
	return 0;
}
