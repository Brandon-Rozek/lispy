#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "expressions.h"
#include "operations.h"
#include "error.h"

// Think about where to put this declaration later
lval* builtin_op(lval* v, char* op);

lval* lval_sexpr(void) {
	lval* v = (lval *) malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}
lval* lval_qexpr(void) {
	lval* v = (lval *) malloc(sizeof(lval));
	v->type = LVAL_QEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

lval* lval_add(lval* v, lval* x) {
	v->count++;
	v->cell = (lval **) realloc(v->cell, sizeof(lval*) * v->count);
	v->cell[v->count - 1] = x;
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
	v->cell = (lval **) realloc(v->cell, sizeof(lval*) * v->count);
	return x;
}

lval* lval_take(lval* v, int i) {
	lval* x = lval_pop(v, i);
	lval_del(v);
	return x;
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



