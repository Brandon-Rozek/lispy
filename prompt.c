#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"
#include "lval.h"

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

double max(double x, double y);
double min(double x, double y);
lval* builtin_op(lval* v, char* op);


int main (int argc, char** argv) {

	// Create some parsers
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Long = mpc_new("long");
	mpc_parser_t* Double = mpc_new("double");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Qexpr = mpc_new("qexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	// Define them with the following language
	mpca_lang(MPCA_LANG_DEFAULT,
			"number   : /[0-9]+/;						                     "
			"long     : /-?[0-9]+/;	                    	                 "
			"double   : <long> '.' <number>;                                 "
			"symbol   : '+' | '-' | '*' | '/' | '%'                          \
				      | '^' | \"min\" | \"max\" 	                         \
					  |	\"list\" | \"head\" | \"tail\" | \"join\" | \"eval\" "
		
			"sexpr    : '(' <expr>* ')';					                 "
			"qexpr    : '{' <expr>* '}';                                     "
			"expr     : (<double> | <long>) | <symbol> | <sexpr> | <qexpr>;  "
			"lispy    : /^/ <expr>* /$/;	        	                     "
			, Number, Long, Double, Symbol, Sexpr, Qexpr, Expr, Lispy);


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
			// Evualuate the expression and print its output
			// lval result = eval(r.output);
			lval* result = lval_eval(lval_read(r.output));
			lval_println(result);
			lval_del(result);
			// mpc_ast_print(r.output);
			mpc_ast_delete(r.output);
		} else {
			// Otherwise print the error
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		// free allocated memory
		free(input);
	}

	mpc_cleanup(8, Number, Long, Double, Symbol, Sexpr, Qexpr, Expr, Lispy);
	return 0;
}


double max(double x, double y) {
	if (x > y) {
		return x;
	}
	return y;
}

double min(double x, double y) {
	if (x < y) {
		return x;
	}
	return y;
}






lval* builtin_op(lval* a, char* op) {
	// Ensure all arguments are numbers
	for (int i = 0; i < a->count; i++) {
		if (a->cell[i]->type != LVAL_LONG && a->cell[i]->type != LVAL_DOUBLE) {
			lval_del(a);
			return lval_err("Cannot run operation on non-number");
		}
	}

	// Pop the first element
	lval* x = lval_pop(a, 0);

	// If there are no other arguments then perform unary operation
	if (a->count == 0) {
		if (strcmp(op, "-") == 0) { lval_updateData(x, -1 * lval_getData(x), x->type); }
	}

	while (a->count > 0) {
		// Pop the next element
		lval* y = lval_pop(a, 0);
		int resultType = (x->type == LVAL_LONG && y->type == LVAL_LONG) ? LVAL_LONG : LVAL_DOUBLE;

		if (strcmp(op, "+")    == 0) { lval_updateData(x, lval_getData(x) + lval_getData(y), resultType); }
		if (strcmp(op, "-")   == 0) {  lval_updateData(x, lval_getData(x) - lval_getData(y), resultType); }
		if (strcmp(op, "*")   == 0) {  lval_updateData(x, lval_getData(x) * lval_getData(y), resultType); }
		if (strcmp(op, "/")   == 0) { 
			if (lval_getData(y) == 0) { return lval_err("Divide by Zero"); }
			 lval_updateData(x, lval_getData(x) / lval_getData(y), resultType); 
		}
		if (strcmp(op, "min") == 0) { lval_updateData(x, min(lval_getData(x), lval_getData(y)), resultType); }
		if (strcmp(op, "max") == 0) { lval_updateData(x, max(lval_getData(x), lval_getData(y)), resultType); }
		if (strcmp(op, "^")   == 0) { lval_updateData(x, pow(lval_getData(x), lval_getData(y)), resultType); }
		if (strcmp(op, "%")   == 0) { lval_updateData(x, fmod(lval_getData(x), lval_getData(y)), resultType); }
		lval_del(y);
	}

	lval_del(a);
	return x;
}
