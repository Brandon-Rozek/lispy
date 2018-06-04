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

typedef union numerr {
	long num;
	double dec;
	int err;
} NumErr;

// A lispy value can either be a number or an error
typedef struct {
	int type;
	NumErr data;
} lval;

// Possible lispy value types
enum { LVAL_LONG, LVAL_DOUBLE, LVAL_ERR };

// Possible Error Types
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM, LERR_BAD_ARG };


lval eval_uni(lval x, char* op);
lval eval_op(lval x, char* op, lval y);
lval eval(mpc_ast_t* t);
double max(double x, double y);
double min(double x, double y);
lval lval_long(long x);
lval lval_double(double x);
lval lval_err(int x);
void flval_print(FILE* stream, lval v);
void lval_print(lval v);
void lval_println(lval v);
size_t treeContentsLength(mpc_ast_t* t);
char* concatTreeContents(mpc_ast_t* t);
void concatNodeContents(char* stringToExtend, mpc_ast_t* t, size_t* currentLength);

int main (int argc, char** argv) {

	// Create some parsers
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Long = mpc_new("long");
	mpc_parser_t* Double = mpc_new("double");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lispy = mpc_new("lispy");

	// Define them with the following language
	mpca_lang(MPCA_LANG_DEFAULT,
			"\
			number   : /[0-9]+/;						\
			long     : '-'? <number>;	               			\
			double   : <long> '.' <number>;                                 \
			operator : '+' | '-' | '*' | '/' | '%'                          \
				 | '^' | \"min\" | \"max\"; 		        	\
			expr     : (<double> | <long>) | '(' <operator> <expr>+ ')';	\
			lispy    : /^/ <operator> <expr>+ /$/;		        	\
			", Number, Long, Double, Operator, Expr, Lispy);



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

	mpc_cleanup(6, Number, Long, Double, Operator, Expr, Lispy);
	return 0;
}

lval eval_op(lval x, char* op, lval y) {
	if (x.type == LVAL_ERR) { return x; }
	if (y.type == LVAL_ERR) { return y; }
	
	int resultType;
	if (x.type == LVAL_LONG && y.type == LVAL_LONG) {
		resultType = LVAL_LONG;
	} else {
		resultType = LVAL_DOUBLE;
	}

	double xVal;
	if (x.type == LVAL_LONG) {
		xVal = x.data.num;
	} else {
		xVal = x.data.dec;
	}

	double yVal;
	if (y.type == LVAL_LONG) {
		yVal = y.data.num;
	} else {
		yVal = y.data.dec;
	}

	if (strcmp(op, "+")    == 0) { 
		return (resultType == LVAL_LONG) ? lval_long(xVal + yVal) : lval_double(xVal + yVal);
	}
	if (strcmp(op, "-")   == 0) { 
		return (resultType == LVAL_LONG) ? lval_long(xVal - yVal) : lval_double(xVal - yVal);   
	}
	if (strcmp(op, "*")   == 0) { 
		return (resultType == LVAL_LONG) ? lval_long(xVal * yVal) : lval_double(xVal * yVal);;
	}
	if (strcmp(op, "/")   == 0) {
		if (yVal == 0) { return lval_err(LERR_DIV_ZERO); }
		return (resultType == LVAL_LONG) ? lval_long(xVal / yVal) : lval_double(xVal / yVal);
  	}
	if (strcmp(op, "min") == 0) { 
		return (resultType == LVAL_LONG) ? lval_long(min(xVal, yVal)) : lval_double(min(xVal, yVal));
	}
	if (strcmp(op, "max") == 0) { 
		return (resultType == LVAL_LONG) ? lval_long(max(xVal, yVal)) : lval_double(max(xVal, yVal));
	}
	if (strcmp(op, "^")   == 0) { 
		return (resultType == LVAL_LONG) ? lval_long(pow(xVal, yVal)) : lval_double(pow(xVal, yVal));
	}
	if (strcmp(op, "%")   == 0) { 
		return (resultType == LVAL_LONG) ? lval_long(fmod(xVal, yVal)) : lval_double(fmod(xVal, yVal));
	}
	
	return lval_err(LERR_BAD_OP);
}

lval eval_uni(lval x, char* op) {

	// If it's an error, return it
	if (x.type == LVAL_ERR) { return x; }

	double xVal = (x.type == LVAL_LONG) ? x.data.num : x.data.dec;

	if (strcmp(op, "-") == 0) { return (x.type == LVAL_LONG) ? lval_long(-1 * xVal) : lval_double(-1 * xVal); }

	return lval_err(LERR_BAD_OP);
}

lval eval(mpc_ast_t* t) {
	if (strstr(t->tag, "long")) {
		// Grab the contents of all the nodes in the tree otherwise you might not get the string you expect
		char* treeString = concatTreeContents(t);
	
		// Check to see if there's some error in conversion
		errno = 0;
		long x = strtol(treeString, NULL, 10);

		// Free the memory allocated in treestring since it's no longer needed
		free(treeString);
		
		return errno != ERANGE ? lval_long(x) : lval_err(LERR_BAD_NUM);
	}

	if (strstr(t->tag, "double")) {
		char* treeString = concatTreeContents(t);

		// Check to see if there's some error in conversion
		errno = 0;
		double x = strtod(treeString, NULL);

		free(treeString);

		return errno != ERANGE ? lval_double(x) : lval_err(LERR_BAD_NUM);
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

lval lval_long(long x) {
	lval v;
	v.type = LVAL_LONG;
	v.data.num = x;
	return v;
}

lval lval_double(double x) {
	lval v;
	v.type = LVAL_DOUBLE;
	v.data.dec = x;
	return v;
}

lval lval_err(int x) {
	lval v;
	v.type = LVAL_ERR;
	v.data.err = x;
	return v;
}

void flval_print(FILE* stream, lval v) {
	switch (v.type) {
		// If it's an integer, then print it out
		case LVAL_LONG: fprintf(stream, "%li", v.data.num); break;
		
		// Do the same for doubles
		case LVAL_DOUBLE: fprintf(stream, "%lf", v.data.dec); break;

		// If it's an error, indicate the error
		case LVAL_ERR:
			       fprintf(stream, "Error: ");
			       if (v.data.err == LERR_DIV_ZERO) {
				       fprintf(stream, "Division by zero");
			       }
			       if (v.data.err == LERR_BAD_OP) {
				       fprintf(stream, "Invalid Operator");
			       }
			       if (v.data.err == LERR_BAD_NUM) {
				       fprintf(stream, "Invalid Number");
			       }
			       break;
	}
}

void lval_print(lval v) { flval_print(stdout, v); }

void lval_println(lval v) { lval_print(v); putchar('\n'); }

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
