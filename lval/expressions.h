#ifndef LVAL_EXPRESSIONS
#define LVAL_EXPRESSIONS
#include "base.h"

// Constructors for the SEXPR and QEXPR data type
lval* lval_sexpr(void);
lval* lval_qexpr(void);

/*
--------------------------------------------------------
    Add the functionality to be able to add or remove
    expressions from the group
*/
lval* lval_add(lval* v, lval* x);
// Remove an expression from the group and return it 
lval* lval_pop(lval* v, int i);
// Remove an expression from the group and delete the rest the group
lval* lval_take(lval* v, int i);

/* ----------------------------------------------------*/

// Adds the ability to evalutate each expression in the group
lval* lval_eval_sexpr(lval* v);


#endif
