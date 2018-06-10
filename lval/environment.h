#ifndef LVAL_ENVIRONMENT
#define LVAL_ENVIRONMENT
#include "base.h"

struct lenv {
    // Represents the parent environment
    lenv* par;
    int count;
    char** syms;
    lval** vals;
};

lenv* lenv_new(void);
void lenv_del(lenv* e);
lenv* lenv_copy(lenv* e);

// Obtain a variable from the environment
// e is the environment
// k is the symbol
lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);
void lenv_def(lenv* e, lval* k, lval* v);

lval* lval_builtin(lbuiltin func);

void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);

lval* builtin_def(lenv* e, lval* a);
lval* builtin_ls(lenv* e, lval* a);
lval* builtin_lambda(lenv* e, lval* a);
lval* builtin_def(lenv* e, lval* a);
lval* builtin_put(lenv* e, lval* a);
lval* builtin_var(lenv* e, lval* a, char* func);

lval* lval_call(lenv* e, lval* f, lval* a);

#endif
