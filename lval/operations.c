#include <string.h>
#include <stdlib.h>
#include "numbers.h"
#include "expressions.h"
#include "operations.h"

lval* lval_sym(char* s) {
	lval* v = (lval *) malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = (char *) malloc(strlen(s) + 1);
	strcpy(v->sym, s);
	return v;
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

	if (strstr(t->tag, "qexpr")) {
		x = lval_qexpr();
	}

	// Fill the list with any valid expression contained
	for (int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
		if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
		if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
		x = lval_add(x, lval_read(t->children[i]));
	}

	return x;
}


lval* lval_eval(lval* v) {
	// Evauluate sexpressions
	if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }

	// All other lval types remail the same
	return v;
}

void lval_del(lval* v) {
	switch (v->type) {
		case LVAL_LONG: break;
		case LVAL_DOUBLE: break;

		// Free the string data
		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;

		// Delete all elements inside SEXPR or QEXPR
		case LVAL_QEXPR:
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


