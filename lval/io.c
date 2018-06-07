#include <stdio.h>
#include "io.h"

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

		case LVAL_QEXPR: flval_expr_print(stream, v, '{', '}'); break;
	}
}

void lval_print(lval* v) { flval_print(stdout, v); }

void lval_println(lval* v) { lval_print(v); putchar('\n'); }