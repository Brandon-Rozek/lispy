#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"

lval* lval_err(char* fmt, ...) {
	lval* v = (lval *) malloc(sizeof(lval));
	v->type = LVAL_ERR;

	// Create a va list and initialize it
	va_list va;
	va_start(va, fmt);

	// Allocate 512 bytes of space	
	v->err = (char *) malloc(512);

	//printf the error string with a maximum of 511 characters
	vsnprintf(v->err, 511, fmt, va);

	// Reallocate to the number of actual bytes
	v->err = realloc(v->err, strlen(v->err) + 1);

	// Cleanup our va list
	va_end(va);

	return v;
}

char* ltype_name(int t) {
  switch(t) {
    case LVAL_FUN: return "Function";
    case LVAL_LONG: return "Long";
	case LVAL_DOUBLE: return "Double";
    case LVAL_ERR: return "Error";
    case LVAL_SYM: return "Symbol";
    case LVAL_SEXPR: return "S-Expression";
    case LVAL_QEXPR: return "Q-Expression";
    default: return "Unknown";
  }
}
