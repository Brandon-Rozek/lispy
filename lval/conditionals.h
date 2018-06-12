#ifndef LVAL_CONDITIONALS
#define LVAL_CONDITIONALS

#include "base.h"

lval* builtin_gt(lenv* e, lval* a);
lval* builtin_lt(lenv* e, lval* a);
lval* builtin_ge(lenv* e, lval* a);
lval* builtin_le(lenv* e, lval* a);

lval* builtin_ord(lenv* e, lval* a, char* op);

int lval_eq(lval* x, lval* y);
lval* builtin_cmp(lenv* e, lval* a, char* op);
lval* builtin_eq(lenv* e, lval* a);
lval* builtin_ne(lenv* e, lval* a);

lval* builtin_if(lenv* e, lval* a);
lval* builtin_and(lenv* e, lval* a);
lval* builtin_or(lenv* e, lval* a);

#endif
