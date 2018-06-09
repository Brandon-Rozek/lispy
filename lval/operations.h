#ifndef LVAL_OPERATIONS
#define LVAL_OPERATIONS

#include "../mpc.h"
#include "base.h"

// Constructor for symbol data type
lval* lval_sym(char* s);

/*
    Methods to read (parse AST), evaluate,
    copy, and delete lval structures
*/
lval* lval_read(mpc_ast_t* t);
lval* lval_eval(lval* v);
void lval_del(lval* v);
lval* lval_copy(lval* v);

#endif
