#include "environment.h"
#include <stdlib.h>
#include "error.h"
#include "expressions.h"
#include "operations.h"

lenv* lenv_new(void) {
    lenv* e = (lenv*) malloc(sizeof(lenv));
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}

void lenv_del(lenv* e) {
    for (int i = 0; i < e->count; i++) {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}

lval* lenv_get(lenv* e, lval* k) {
    // Iterate over all items in environment
    for (int i = 0; i < e->count; i++) {
        // Check if the stored string matches the symbol string
        // If it does, return a copy of hte value
        if (strcmp(e->syms[i], k->sym) == 0) {
            return lval_copy(e->vals[i]);
        }
    }

    // If no symbol found return error
    return lval_err("unbounded symbol!");
}

void lenv_put(lenv* e, lval* k, lval* v) {
    // Iterate over all items in the environment
    // This is to see if the variables already exist
    for (int i = 0; i < e->count; i++) {
        // If a variable is found, delete the item at that position
        // Then replace it with the data provided by the user
        if (strcmp(e->syms[i], k->sym) == 0) {
            lval_del(e->vals[i]);
            e->vals[i] = lval_copy(v);
            return;
        }
    }

    // If no existing entry is found, allocate space for new entry
    e->count++;
    e->vals = realloc(e->vals, sizeof(lval*) * e->count);
    e->syms = realloc(e->syms, sizeof(char*) * e->count);

    // Copy contents of lval and symbol string
    e->vals[e->count - 1] = lval_copy(v);
    e->syms[e->count - 1] = (char*) malloc(strlen(k->sym) + 1);
    strcpy(e->syms[e->count - 1], k->sym);
}

lval* lval_fun(lbuiltin func) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->fun = func;
    return v;
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
    lval* k = lval_sym(name);
    lval* v = lval_fun(func);
    lenv_put(e, k, v);
    lval_del(k); lval_del(v);
}

void lenv_add_builtins(lenv* e) {
    // List functions
    lenv_add_builtin(e, "list", builtin_list);
    lenv_add_builtin(e, "head", builtin_head);
    lenv_add_builtin(e, "tail", builtin_tail);
    lenv_add_builtin(e, "eval", builtin_eval);
    lenv_add_builtin(e, "join", builtin_join);
    lenv_add_builtin(e, "len", builtin_len);
    lenv_add_builtin(e, "cons", builtin_cons);

    // Mathematical Functions
    lenv_add_builtin(e, "+", builtin_add);
    lenv_add_builtin(e, "-", builtin_sub);
    lenv_add_builtin(e, "*", builtin_mul);
    lenv_add_builtin(e, "/", builtin_div);
    lenv_add_builtin(e, "^", builtin_pow);
    lenv_add_builtin(e, "%", builtin_mod);
    lenv_add_builtin(e, "min", builtin_min);
    lenv_add_builtin(e, "max", builtin_max);
}