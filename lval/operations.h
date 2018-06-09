#ifndef LVAL_OPERATIONS
#define LVAL_OPERATIONS

#include "../mpc.h"
#include "base.h"
#include "environment.h"

// Constructor for symbol data type
lval* lval_sym(char* s);

/*
    Methods to read (parse AST), evaluate,
    copy, and delete lval structures
*/
lval* lval_read(mpc_ast_t* t);
lval* lval_eval(lenv* e, lval* v);
void lval_del(lval* v);
lval* lval_copy(lval* v);

// Math libraries
lval* builtin_add(lenv* e, lval* a);
lval* builtin_sub(lenv* e, lval* a);
lval* builtin_mul(lenv* e, lval* a);
lval* builtin_div(lenv* e, lval* a);
lval* builtin_pow(lenv* e, lval* a);
lval* builtin_mod(lenv* e, lval* a);
lval* builtin_min(lenv* e, lval* a);
lval* builtin_max(lenv* e, lval* a);



#endif
