#ifndef LVAL_EXPRESSIONS
#define LVAL_EXPRESSIONS
#include "base.h"
#include "environment.h"

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
lval* lval_eval_sexpr(lenv* e, lval* v);

/*
    QEXPR Operations
    Head: Takes a Q-Expression and returns a Q-Expression with only the first element
    Tail: Takes a Q-Expression and returns a Q-Expression with the first element removed
    List: Takes one or more arguments and returns a new Q-Expression containing the arguments
    Eval: Takes a Q-Expression and evaluates it as if it were a S-Expression
    Join: Takes one or more Q-Expressions and returns a Q-Expression of them conjoined together
    Len:  Takes a Q-Expression and returns the length of the Q-Expression
    Init: Takes a Q-Expression and returns a Q-Expression with only the last element removed   
*/
lval* builtin_headn(lenv* e, lval* a, int n);
lval* builtin_head(lenv* e, lval* a);
lval* builtin_init(lenv* e, lval* a);
lval* builtin_tail(lenv* e, lval* a);
lval* builtin_list(lenv* e, lval* a);
lval* builtin_eval(lenv* e, lval* a);
lval* lval_join(lenv* e, lval* x, lval* y);
lval* builtin_join(lenv* e, lval* a);
lval* builtin_len(lenv* e, lval* a);
lval* builtin_cons(lenv* e, lval* a);

#endif
