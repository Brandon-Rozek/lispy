#include "environment.h"
#include <stdlib.h>
#include "error.h"
#include "expressions.h"
#include "operations.h"
#include "conditionals.h"

lenv* lenv_new(void) {
    lenv* e = (lenv*) malloc(sizeof(lenv));
    e->par = NULL;
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

lenv* lenv_copy(lenv* e) {
    lenv* n = (lenv*) malloc(sizeof(lenv));
    n->par = e->par;
    n->count = e->count;
    n->syms = (char**) malloc(sizeof(char*) * n->count);
    n->vals = (lval**) malloc(sizeof(lval*) * n->count);
    for (int i = 0; i < e->count; i++) {
        n->syms[i] = (char*) malloc(strlen(e->syms[i]) + 1);
        strcpy(n->syms[i], e->syms[i]);
        n->vals[i] = lval_copy(e->vals[i]);
    }
    return n;
}

lval* lenv_get(lenv* e, lval* k) {
    // Iterate over all items in environment
    for (int i = 0; i < e->count; i++) {
        // Check if the stored string matches the symbol string
        // If it does, return a copy of hte value
        if (strcmp(e->syms[i], k->data.sym) == 0) {
            return lval_copy(e->vals[i]);
        }
    }

    // If no symbol found so far, check in parent
    if (e->par) {
        return lenv_get(e->par, k);
    }

    // If no symbol found and no parent, return error
    return lval_err("Unbounded symbol %s", k->data.sym);
}

void lenv_put(lenv* e, lval* k, lval* v) {
    // Iterate over all items in the environment
    // This is to see if the variables already exist
    for (int i = 0; i < e->count; i++) {
        // If a variable is found, delete the item at that position
        // Then replace it with the data provided by the user
        if (strcmp(e->syms[i], k->data.sym) == 0) {
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
    e->syms[e->count - 1] = (char*) malloc(strlen(k->data.sym) + 1);
    strcpy(e->syms[e->count - 1], k->data.sym);
}

void lenv_def(lenv* e, lval* k, lval* v) {
    // Iterate until e has no parent
    while (e->par) { e = e->par; }

    // Put the value in e
    lenv_put(e, k, v);
}

lval* lval_builtin(lbuiltin func) {
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_FUN;
  v->builtin = func;
  return v;
}

void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
    lval* k = lval_sym(name);
    lval* v = lval_builtin(func);
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
    
    lenv_add_builtin(e, "def", builtin_def);
    lenv_add_builtin(e, "=", builtin_put);
    lenv_add_builtin(e, "ls", builtin_ls);
    lenv_add_builtin(e, "\\", builtin_lambda);

    // Conditional functions
    lenv_add_builtin(e, "<", builtin_lt);
    lenv_add_builtin(e, ">", builtin_gt);
    lenv_add_builtin(e, "<=", builtin_le);
    lenv_add_builtin(e, ">=", builtin_ge);
    lenv_add_builtin(e, "==", builtin_eq);
    lenv_add_builtin(e, "!=", builtin_ne);

    lenv_add_builtin(e, "if", builtin_if);

    lenv_add_builtin(e, "and", builtin_and);
    lenv_add_builtin(e, "&&", builtin_and);
    lenv_add_builtin(e, "or", builtin_or);
    lenv_add_builtin(e, "||", builtin_or);

}

lval* builtin_ls(lenv* e, lval* a) {
    LASSERT_NUM("ls", a, 0)

    lval* x = lval_qexpr();
    for (int i = 0; i < e->count; i++) {
        lval_add(x, lval_sym(e->syms[i]));
    }

    lval_del(a);
    return x;
}

lval* lval_lambda(lval* formals, lval* body) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;

    // Set builtin to null
    v->builtin = NULL;

    // Build new environment
    v->env = lenv_new();

    // Set formals and body
    v->formals = formals;
    v->body = body;
    return v;
}

lval* builtin_lambda(lenv* e, lval* a) {
    // Check for two arguments each of which are Q-Expressions
    LASSERT_NUM("\\", a, 2)
    LASSERT_TYPE("\\", a, 0, LVAL_QEXPR)
    LASSERT_TYPE("\\", a, 1, LVAL_QEXPR)

    // Check first Q-expression contains only symbols
    for (int i = 0; i < a->cell[0]->count; i++) {
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
            "Cannot define non-symbol. Got %s, expected %s.",
            ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM))
    }

    // Pop first two arguments and pass them to lval_lambda
    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    lval_del(a);

    return lval_lambda(formals, body);
}

lval* builtin_var(lenv* e, lval* a, char* func) {
    LASSERT_TYPE(func, a, 0, LVAL_QEXPR)

    lval* syms = a->cell[0];
    for (int i = 0; i < syms->count; i++) {
        LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
            "Function '%s' cannot define non-symbol. "
            "Got %s, Expected %s.", func,
            ltype_name(syms->cell[i]->type),
            ltype_name(LVAL_SYM))
    }

    LASSERT(a, (syms->count == a->count-1),
        "Function '%s' passed too many arguments for symbols. "
        "Got %i, Expected %i.", func, syms->count, a->count-1)

    for (int i = 0; i < syms->count; i++) {
        // If 'def' define it globally
        if (strcmp(func, "def") == 0) {
            lenv_def(e, syms->cell[i], a->cell[i + 1]);
        }
        // If 'put' define it locally
        if (strcmp(func, "=") == 0) {
            lenv_put(e, syms->cell[i], a->cell[i + 1]);
        }
    }

    lval_del(a);
    return lval_sexpr();
}

lval* builtin_def(lenv* e, lval* a) {
    return builtin_var(e, a, "def");
}
lval* builtin_put(lenv* e, lval* a) {
    return builtin_var(e, a, "=");
}

lval* lval_call(lenv* e, lval* f, lval* a) {
    // If builtin simply apply that
    if (f->builtin) { return f->builtin(e, a); }

    // Record argument counts
    int given = a->count;
    int total = f->formals->count;

    // // While arguments still remain to be processed
    while (a->count) {
        // If we've run out of formal arguments..
        if (f->formals->count == 0) {
            lval_del(a); 
            return lval_err("Function passed too many arguments. "
                "Got %i, Expected %i.", given, total);
        }

        // Pop the first symbol from the formals 
        lval* sym = lval_pop(f->formals, 0);

        if (strcmp(sym->data.sym, "&") == 0) {
            // Ensure '&' is followed by another symbol
            if (f->formals->count != 1) {
                lval_del(a);
                return lval_err("Function format invalid."
                "Symbol '&' not followed by single symbol.");
            }

            // Next formal should be bounded to remaining arguments
            lval* nsym = lval_pop(f->formals, 0);
            lenv_put(f->env, nsym, builtin_list(e, a));
            lval_del(sym); lval_del(nsym);
            break;
        }

        // Pop the next argument from the list
        lval* val = lval_pop(a, 0);

        // Bind a copy into the function's environment
        lenv_put(f->env, sym, val);

        // Delete the symbol and value
        lval_del(sym); lval_del(val);
    }

    // The argument list is now bounded so we can clean up the given
    lval_del(a);

    // If '&' remains in formal list bind to empty list
    if (f->formals->count > 0 &&
        strcmp(f->formals->cell[0]->data.sym, "&") == 0) {
            // Check to ensure that & is no passed invalidly
            if (f->formals->count != 2) {
                return lval_err("Function format invalid."
                "Symbol '&' not followed by single symbol.");
            }

            // Pop and delete '&' symbol
            lval_del(lval_pop(f->formals, 0));

            // Pop next symbol and create empty list
            lval* sym = lval_pop(f->formals, 0);
            lval* val = lval_qexpr();

            // Bind to environment and delete
            lenv_put(f->env, sym, val);
            lval_del(sym); lval_del(val);
    }

    // If all formals have been bounded evaluate
    if (f->formals->count == 0) {
        // Set environment parent to evaluation environment
        f->env->par = e;

        // Evaluate and return
        return builtin_eval(
            f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
    } else {
        // Otherwise return partially evaluated function
        return lval_copy(f);
    }
    return lval_sexpr();
}