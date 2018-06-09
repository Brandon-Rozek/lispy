#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "expressions.h"
#include "numbers.h"
#include "operations.h"
#include "error.h"

// Think about where to put these declarations later
lval* builtin(lval* a, char* func);
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

	lval* result = builtin(v, f->sym);
	lval_del(f);
	return result;
}


lval* builtin_headn(lval* a, int n) {
	LASSERT(a, a->count == 1, "Function 'head' passed too many arguments")
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'head' passed incorrect type")
	LASSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}")

	lval* v = lval_take(a, 0);
	while (v->count > n) { lval_del(lval_pop(v, v->count - 1)); }
	return v;
}

lval* builtin_head(lval* a) {
	return builtin_headn(a, 1);
}

lval* builtin_init(lval* a) {
	LASSERT(a, a->count == 1, "Function 'init' passed too many arguments")
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'init' passed incorrect type")
	LASSERT(a, a->cell[0]->count != 0, "Function 'init' passed {}")

	return builtin_headn(a, a->cell[0]->count - 1);
}


lval* builtin_tail(lval* a) {
	LASSERT(a, a->count == 1, "Function 'tail' passed too many arguments")
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'tail' passed incorrect type")
	LASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}")

	lval* v = lval_take(a, 0);
	lval_del(lval_pop(v, 0));
	return v;
}

lval* builtin_list(lval* a) {
	a->type = LVAL_QEXPR;
	return a;
}

lval* builtin_eval(lval* a) {
	LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments")
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'eval' passed incorrect type")

	lval* x = lval_take(a, 0);
	x->type = LVAL_SEXPR;
	return lval_eval(x);
}

lval* lval_join(lval* x, lval* y) {
	// For each cell in 'y' add it to 'x'
	while (y->count) {
		x = lval_add(x, lval_pop(y, 0));
	}

	// Delete the empty y and return x
	lval_del(y);
	return x;
}

lval* builtin_join(lval* a) {
	for (int i = 0 ; i < a->count; i++) {
		LASSERT(a, a->cell[i]->type == LVAL_QEXPR, "Function 'join' passed incorrect type")
	}

	lval* x = lval_pop(a, 0);
	
	while (a->count) {
		x = lval_join(x, lval_pop(a, 0));
	}

	lval_del(a);
	return x;
}

lval* builtin_len(lval* a) {
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'len' passed incorrect type")
	lval* x = lval_long(a->cell[0]->count);
	
	lval_del(a);
	return x;
}

lval* builtin_cons(lval* a) {
	LASSERT(a, a->cell[0]->type != LVAL_QEXPR, "Function 'cons' passed incorrect type on first argument")
	LASSERT(a, a->cell[1]->type == LVAL_QEXPR, "Function 'cons' passed incorrect type on second argument")
	LASSERT(a, a->count == 2, "Function 'cons' passed an incorrect number of arguments")
	
	lval* x = lval_qexpr();
	x = lval_add(x, lval_pop(a, 0));
	x = lval_join(x, lval_pop(a, 0));

	lval_del(a);

	return x;

}
