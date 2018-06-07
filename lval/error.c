#include <stdlib.h>
#include <string.h>
#include "error.h"

lval* lval_err(char* m) {
	lval* v = (lval *) malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = (char *) malloc(strlen(m) + 1);
	strcpy(v->err, m);
	return v;
}