#ifndef LVAL_ENVIRONMENT
#define LVAL_ENVIRONMENT
#include "base.h"

struct lenv {
    int count;
    char** syms;
    lval** vals;
};

lenv* lenv_new(void);
void lenv_del(lenv* e);
// Obtain a variable from the environment
// e is the environment
// k is the symbol
lval* lenv_get(lenv* e, lval* k);
void lenv_put(lenv* e, lval* k, lval* v);

lval* lval_fun(lbuiltin func);

void lenv_add_builtin(lenv* e, char* name, lbuiltin func);
void lenv_add_builtins(lenv* e);

lval* builtin_def(lenv* e, lval* a);
lval* builtin_ls(lenv* e, lval* a);


#endif
