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
		case LVAL_FUN: break;

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

lval* lval_copy(lval* v) {
	lval* x = (lval*) malloc(sizeof(lval));
	x->type = v->type;

	switch (v->type)  {
		// Copy numbers and functions directly
		case LVAL_LONG: x->data.num = v->data.num; break;
		case LVAL_DOUBLE: x->data.dec = v->data.dec; break;
		case LVAL_FUN: x->fun = v->fun; break;

		// Copy strings using malloc and strcpy
		case LVAL_ERR:
			x->err = (char*) malloc(strlen(v->err) + 1);
			strcpy(x->err, v->err); break;
		case LVAL_SYM:
			x->sym = (char*) malloc(strlen(v->sym) + 1);
			strcpy(x->sym, v->sym); break;
		
		// Copy lists by copying each sub-expression
		case LVAL_SEXPR:
		case LVAL_QEXPR:
			x->count = v->count;
			x->cell = (lval**) malloc(sizeof(lval*) * x->count);
			for (int i = 0; i < x->count; i++) {
				x->cell[i] = lval_copy(v->cell[i]);
			}
			break;
	}

	return x;
}


