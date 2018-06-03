#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

// Undefine min/max macros if existent
#undef max
#undef min

// If we're compiling on Windows
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

// Fake readline function
char* readline(char* prompt) {
	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);
	char* cpy = malloc(strlen(buffer) + 1);
	strcpy(cpy, buffer);
	cpy[strlen(cpy) - 1] = '\0';
	return cpy;
}

void add_history(char* unused) {}

// If not windows include editline
#else
// For keeping track of command history
#include <editline/readline.h>
#endif

// A lispy value can either be a number or an error
typedef struct {
	int type;
	long num;
	int err;
} lval;

// Possible lispy value types
enum { LVAL_NUM, LVAL_ERR };

// Possible Error Types
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };


lval eval_uni(lval x, char* op);
lval eval_op(lval x, char* op, lval y);
lval eval(mpc_ast_t* t);
long max(long x, long y);
long min(long x, long y);
lval lval_num(long x);
lval lval_err(int x);
void flval_print(FILE* stream, lval v);
void lval_print(lval v);
void lval_println(lval v);

int main (int argc, char** argv) {

	// Create some parsers
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	// Define them with the following language
	mpca_lang(MPCA_LANG_DEFAULT,
			"\
			number   : /-?[0-9]+/;					\
			operator : '+' | '-' | '*' | '/' | '%'                  \
				 | '^' | \"min\" | \"max\"; 			\
			expr     : <number> | '(' <operator> <expr>+ ')';	\
			lispy    : /^/ <operator> <expr>+ /$/;			\
			", Number, Operator, Expr, Lispy);



	// Print Version and Exit Information
	puts("Lispy Version 0.0.0.0.1");
	puts("Press Ctrl+c to Exit\n");

	// In a never ending loop
	while (1) {
		// Output prompt and query
		char* input = readline("lispy> ");

		// Add input to history
		add_history(input);

		// Attempt to parse the user input
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lispy, &r)) {
			// On success print the abstract syntax tree
			//mpc_ast_print(r.output);
			
			// Evualuate the expression and print its output
			lval result = eval(r.output);
			lval_println(result);
			mpc_ast_delete(r.output);
		} else {
			// Otherwise print the error
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		// free allocated memory
		free(input);
	}

	mpc_cleanup(4, Number, Operator, Expr, Lispy);
	return 0;
}

lval eval_op(lval x, char* op, lval y) {
	if (x.type == LVAL_ERR) { return x; }
	if (y.type == LVAL_ERR) { return y; }

	if (strcmp(op, "+")   == 0) { return lval_num(x.num + y.num);      }
	if (strcmp(op, "-")   == 0) { return lval_num(x.num - y.num);      }
	if (strcmp(op, "*")   == 0) { return lval_num(x.num * y.num);      }
	if (strcmp(op, "/")   == 0) {	
		// If you try to divide by zero, report an error
		return y.num == 0 ? lval_err(LERR_DIV_ZERO) : lval_num(x.num / y.num);
  	}
	if (strcmp(op, "min") == 0) { return lval_num(min(x.num, y.num));  }
	if (strcmp(op, "max") == 0) { return lval_num(max(x.num, y.num));  }
	if (strcmp(op, "^")   == 0) { return lval_num(pow(x.num, y.num));  }
	if (strcmp(op, "%")   == 0) { return lval_num(fmod(x.num, y.num)); }
	
	return lval_err(LERR_BAD_OP);
}

lval eval_uni(lval x, char* op) {
	// If it's an error, return it
	if (x.type == LVAL_ERR) { return x; }

	if (strcmp(op, "-") == 0) { return lval_num(-1 * x.num); }

	return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
	// If tagged as a number, return directly
	if (strstr(t->tag, "number")) {
		// Check to see if there's some error in conversion
		errno = 0;
		long x = strtol(t->contents, NULL, 10);
		return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
	}

	// The operator is always the second child
	char* op = t->children[1]->contents;

	// We store the third child in x
	lval x = eval(t->children[2]);


	// If there is only one operand, apply a uniary operation
	if (t->children_num == 4) {
		return eval_uni(x, op);
	}

	// Iterate over the remaining children and reduce
	int i = 3;
	while (strstr(t->children[i]->tag, "expr")) {
		x = eval_op(x, op, eval(t->children[i]));
		i++;
	}

	return x;
}

long max(long x, long y) {
	if (x > y) {
		return x;
	}
	return y;
}

long min(long x, long y) {
	if (x < y) {
		return x;
	}
	return y;
}

lval lval_num(long x) {
	lval v;
	v.type = LVAL_NUM;
	v.num = x;
	return v;
}

lval lval_err(int x) {
	lval v;
	v.type = LVAL_ERR;
	v.err = x;
	return v;
}

void flval_print(FILE* stream, lval v) {
	switch (v.type) {
		// If it's a number, then print it out
		case LVAL_NUM: fprintf(stream, "%li", v.num); break;
		
		// If it's an error, indicate the error
		case LVAL_ERR:
			       fprintf(stream, "Error: ");
			       if (v.err == LERR_DIV_ZERO) {
				       fprintf(stream, "Division by zero");
			       }
			       if (v.err == LERR_BAD_OP) {
				       fprintf(stream, "Invalid Operator");
			       }
			       if (v.err == LERR_BAD_NUM) {
				       fprintf(stream, "Invalid Number");
			       }
			       break;
	}
}

void lval_print(lval v) { flval_print(stdout, v); }

void lval_println(lval v) { lval_print(v); putchar('\n'); }


