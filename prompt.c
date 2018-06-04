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

typedef union typeval {
	long num;
	double dec;
} TypeVal;

// A lispy value can either be a number or an error
typedef struct lval {
	int type;
	TypeVal data;

	// Error and symbols contain string data
	char* err;
	char* sym;

	// Count and pointer to a list of lval*
	int count;
	struct lval** cell;
} lval;

// Possible lispy value types
enum { LVAL_ERR, LVAL_LONG, LVAL_DOUBLE, LVAL_SYM, LVAL_SEXPR };

double max(double x, double y);
double min(double x, double y);
lval* lval_long(long x);
lval* lval_double(double x);
lval* lval_err(char* m);
lval* lval_sym(char* s);
lval* lval_sexpr(void);
double lval_getData(lval* x);
void lval_updateData(lval* x, double val, int type);
void lval_del(lval* v);
lval* lval_read_long(mpc_ast_t* t);
lval* lval_read_double(mpc_ast_t* t);
lval* lval_read(mpc_ast_t* t);
lval* lval_add(lval* v, lval* x);
lval* lval_eval_sexpr(lval* v);
lval* lval_eval(lval* v);
lval* lval_take(lval* v, int i);
lval* lval_pop(lval* v, int i);
lval* builtin_op(lval* v, char* op);
void flval_print(FILE* stream, lval* v);
void lval_print(lval* v);
void lval_println(lval* v);
size_t treeContentsLength(mpc_ast_t* t);
char* concatTreeContents(mpc_ast_t* t);
void concatNodeContents(char* stringToExtend, mpc_ast_t* t, size_t* currentLength);

int main (int argc, char** argv) {

	// Create some parsers
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Long = mpc_new("long");
	mpc_parser_t* Double = mpc_new("double");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	// Define them with the following language
	mpca_lang(MPCA_LANG_DEFAULT,
			"\
			number   : /[0-9]+/;						           \
			long     : /-?[0-9]+/;	                    	       \
			double   : <long> '.' <number>;                        \
			symbol   : '+' | '-' | '*' | '/' | '%'                 \
				     | '^' | \"min\" | \"max\"; 		           \
			sexpr    :  '(' <expr>* ')';					       \
			expr     : (<double> | <long>) | <symbol> | <sexpr>;   \
			lispy    : /^/ <expr>* /$/;		        	           \
			", Number, Long, Double, Symbol, Sexpr, Expr, Lispy);



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
			mpc_ast_print(r.output);
			mpc_ast_delete(r.output);
		} else {
			// Otherwise print the error
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}

		// free allocated memory
		free(input);
	}

	mpc_cleanup(7, Number, Long, Double, Symbol, Sexpr, Expr, Lispy);
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

lval* lval_long(long x) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_LONG;
	v->data.num = x;
	return v;
}

lval* lval_double(double x) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_DOUBLE;
	v->data.dec = x;
	return v;
}

lval* lval_err(char* m) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(m) + 1);
	strcpy(v->err, m);
	return v;
}

lval* lval_sym(char* s) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) + 1);
	strcpy(v->sym, s);
	return v;
}

lval* lval_sexpr(void) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

void lval_del(lval* v) {
	switch (v->type) {
		case LVAL_LONG: break;
		case LVAL_DOUBLE: break;

		// Free the string data
		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;

		// Delete all elements inside SEXPR
		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++) {
				lval_del(v->cell[i]);
			}
			// Also free the memory allocated to contain the pointers
			free(v->cell);
			break;
	}

	// // Free the memory allocated for the lval struct itself
	free(v);
}

lval* lval_read_long(mpc_ast_t* t) {
		// Grab the contents of all the nodes in the tree otherwise you might not get the string you expect
		char* treeString = concatTreeContents(t);
	
		// Check to see if there's some error in conversion
		errno = 0;
		long x = strtol(treeString, NULL, 10);

		// Free the memory allocated in treestring since it's no longer needed
		free(treeString);
		
		return errno != ERANGE ? lval_long(x) : lval_err("Invalid Number");
}

lval* lval_read_double(mpc_ast_t* t) {
		char* treeString = concatTreeContents(t);

		// Check to see if there's some error in conversion
		errno = 0;
		double x = strtod(treeString, NULL);

		free(treeString);

		return errno != ERANGE ? lval_double(x) : lval_err("Invalid Number");
}

lval* lval_read(mpc_ast_t* t) {
	// If symbol or number, convert
	if (strstr(t->tag, "long")) { return lval_read_long(t); }
	if (strstr(t->tag, "double")) { return lval_read_double(t); }
	if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }

	// If root or sexpr, then create an empty list
	lval* x = NULL;
	if (strcmp(t->tag, ">") == 0 || strstr(t->tag, "sexpr")) { 
		x = lval_sexpr(); 
	}

	// Fill the list with any valid expression contained
	for (int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
		if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
		if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
		x = lval_add(x, lval_read(t->children[i]));
	}

	return x;
}

lval* lval_add(lval* v, lval* x) {
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	v->cell[v->count - 1] = x;
	return v;
}

void flval_expr_print(FILE* stream, lval* v, char open, char close) {
	putchar(open);
	for (int i = 0; i < v->count; i++) {
		// Print value contained within
		flval_print(stream, v->cell[i]);

		// Put a trailing whitespace unless its the last element
		if (i != (v->count - 1)) {
			putchar(' ');
		}
	}
	putchar(close);
}

void flval_print(FILE* stream, lval* v) {
	switch (v->type) {
		// If it's an integer, then print it out
		case LVAL_LONG: fprintf(stream, "%li", v->data.num); break;
		
		case LVAL_DOUBLE: fprintf(stream, "%lf", v->data.dec); break;

		case LVAL_ERR: fprintf(stream, "Error: %s", v->err); break;

		case LVAL_SYM: fprintf(stream, "%s", v->sym); break;

		case LVAL_SEXPR: flval_expr_print(stream, v, '(', ')'); break;
	}
}

void lval_print(lval* v) { flval_print(stdout, v); }

void lval_println(lval* v) { lval_print(v); putchar('\n'); }

size_t treeContentsLength(mpc_ast_t* t) {
	size_t result = strlen(t->contents);
	if (t->children_num == 0) {
		return result;
	}

	for (int i = 0; i < t->children_num; i++) {
		result += treeContentsLength(t->children[i]);
	}
	return result;
}

char* concatTreeContents(mpc_ast_t* t) {
	// Calculate size needed for the string
	size_t totalLength = treeContentsLength(t);

	// Allocate memory for string and null terminator
	char* stringToExtend = malloc(totalLength + 1);
	// [TODO] Write an allocation error handler

	size_t currentLength = 0;
	concatNodeContents(stringToExtend, t, &currentLength);

	stringToExtend[totalLength] = '\0';

	return stringToExtend;
}

void concatNodeContents(char* stringToExtend, mpc_ast_t* t, size_t* currentLength) {
	size_t leafLength = strlen(t->contents);

	memcpy(stringToExtend + (*currentLength), t->contents, leafLength);
	*currentLength = *currentLength + leafLength;

	if (t->children_num != 0) {
		for (int i = 0; i < t->children_num; i++) {
			concatNodeContents(stringToExtend, t->children[i], currentLength);
		}
	}

}


lval* lval_eval_sexpr(lval* v) {
	// Evaluate children
	for (int i = 0; i < v->count; i++) {
		v->cell[i] = lval_eval(v->cell[i]);
	}

	// Error checking [If there's an error, return it]
	for (int i = 0; i < v->count; i++) {
		if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
	}

	// Empty expression
	if (v->count == 0) { return v; }

	// Single expression
	if (v->count == 1) { return lval_take(v, 0); }

	// Ensure first element is a symbol otherwise
	lval* f = lval_pop(v, 0);
	if (f->type != LVAL_SYM) {
		printf("The type of f is %d\n", f->type);
		lval_del(f); lval_del(v);
		return lval_err("S-expression does not start with symbol");
	}

	lval* result = builtin_op(v, f->sym);
	lval_del(f);
	return result;
}

lval* lval_eval(lval* v) {
	// Evauluate sexpressions
	if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }

	// All other lval types remail the same
	return v;
}

lval* lval_pop(lval* v, int i) {
	// Find the item at i
	lval* x = v->cell[i];

	// Shift the memory after the item i over the top
	memmove(&v->cell[i], &v->cell[i + 1], sizeof(lval*) * (v->count - i - 1));

	// Decrease the count of items in the list
	v->count--;

	// Reallocate the memory used
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	return x;
}

lval* lval_take(lval* v, int i) {
	lval* x = lval_pop(v, i);
	lval_del(v);
	return x;
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


double lval_getData(lval* x) {
	if (x->type == LVAL_LONG) {
		return x->data.num;
	} 
	return x->data.dec;
} 

void lval_updateData(lval* x, double val, int type) {
	if (type == LVAL_LONG) {
		x->data.num = val;
		return;
	} 
	x->data.dec = val;
} 
