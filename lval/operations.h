#ifndef LVAL_OPERATIONS
#define LVAL_OPERATIONS

#include "../mpc.h"
#include "base.h"

// Constructor for other data types
lval* lval_sym(char* s);
lval* lval_err(char* m);

/*
    Methods to read (parse AST), evaluate, 
    and delete lval structures
*/
lval* lval_read(mpc_ast_t* t);
lval* lval_eval(lval* v);
void lval_del(lval* v);

#endif
