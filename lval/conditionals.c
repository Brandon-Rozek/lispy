#include "conditionals.h"
#include "numbers.h"
#include "operations.h"
#include "expressions.h"
#include "error.h"

lval* builtin_gt(lenv* e, lval* a) {
    return builtin_ord(e, a, ">");
}

lval* builtin_lt(lenv* e, lval* a) {
    return builtin_ord(e, a, "<");
}

lval* builtin_ge(lenv* e, lval* a) {
    return builtin_ord(e, a, ">=");
}

lval* builtin_le(lenv* e, lval* a) {
    return builtin_ord(e, a, "<=");
}

lval* builtin_ord(lenv* e, lval* a, char* op) {
    LASSERT_NUM(op, a, 2);
    LASSERT(a, 
        a->cell[0]->type == LVAL_LONG || a->cell[0]->type == LVAL_DOUBLE,
        "Function '%s' passed incorrect type for argument %i. " \
    "Got %s, Expected %s.", op, 0, ltype_name(a->cell[0]->type), "LONG or DOUBLE")
    LASSERT(a, 
        a->cell[1]->type == LVAL_LONG || a->cell[0]->type == LVAL_DOUBLE,
        "Function '%s' passed incorrect type for argument %i. " \
    "Got %s, Expected %s.", op, 1, ltype_name(a->cell[0]->type), "LONG or DOUBLE")

    int r;
    double xVal = (a->cell[0]->type == LVAL_LONG)? a->cell[0]->data.num : a->cell[0]->data.dec;  
    double yVal = (a->cell[1]->type == LVAL_LONG)? a->cell[1]->data.num : a->cell[1]->data.dec;
    if (strcmp(op, ">") == 0) {
        r = (xVal > yVal);
    }
    if (strcmp(op, "<") == 0) {
        r = (xVal < yVal);
    }
    if (strcmp(op, ">=") == 0) {
        r = (xVal >= yVal);
    }
    if (strcmp(op, "<=") == 0) {
        r = (xVal <= yVal);
    }

    lval_del(a);
    return lval_long(r);
}

int lval_eq(lval* x, lval* y) {
    // Different types are always unequal
    if (x->type != y->type) { return 0; }

    // Compare base on type
    switch (x->type) {
        // Compare numerical types
        case LVAL_LONG: return (x->data.num == y->data.num);
        case LVAL_DOUBLE: return (x->data.dec == y->data.dec);

        // Compare string values
        case LVAL_ERR: return (strcmp(x->err, y->err) == 0);
        case LVAL_SYM: return (strcmp(x->sym, y->sym) == 0);

        // If builtin compare, otherwise compare formals and body
        case LVAL_FUN:
            if (x->builtin || y->builtin) {
                return x->builtin == y->builtin;
            } else {
                return lval_eq(x->formals, y->formals) &&
                    lval_eq(x->body, y->body);
            }
        
        case LVAL_QEXPR:
        case LVAL_SEXPR:
            if (x->count != y->count) { return 0; }
            for (int i = 0; i < x->count; i++) {
                // If any element not equal then whole list are not equal
                if (!lval_eq(x->cell[i], y->cell[i])) { return 0; }
            }
            // Otherwise lists must be equal
            return 1;
        break;
    }

    return 0;
}

lval* builtin_cmp(lenv* e, lval* a, char* op) {
    LASSERT_NUM(op, a, 2);
    int r;
    if (strcmp(op, "==") == 0) {
        r =  lval_eq(a->cell[0], a->cell[1]);
    }
    if (strcmp(op, "!=") == 0) {
        r = !lval_eq(a->cell[0], a->cell[1]);
    }
    if (strcmp(op, "or") == 0) {
        LASSERT_TYPE("or", a, 0, LVAL_LONG)
        LASSERT_TYPE("or", a, 1, LVAL_LONG)
        r = (a->cell[0]->data.num || a->cell[1]->data.num);
    }
    if (strcmp(op, "and") == 0) {
        LASSERT_TYPE("and", a, 0, LVAL_LONG)
        LASSERT_TYPE("and", a, 1, LVAL_LONG)
        r = (a->cell[0]->data.num && a->cell[1]->data.num);
    }

    lval_del(a);
    return lval_long(r);
}

lval* builtin_eq(lenv* e, lval* a) {
    return builtin_cmp(e, a, "==");
}

lval* builtin_ne(lenv* e, lval* a) {
    return builtin_cmp(e, a, "!=");
}
lval* builtin_or(lenv* e, lval* a) {
    return builtin_cmp(e, a, "or");
}

lval* builtin_and(lenv* e, lval* a) {
    return builtin_cmp(e, a, "and");
}

lval* builtin_if(lenv* e, lval* a) {
    LASSERT_NUM("if", a, 3)
    LASSERT_TYPE("if", a, 0, LVAL_LONG)
    LASSERT_TYPE("if", a, 1, LVAL_QEXPR)
    LASSERT_TYPE("if", a, 2, LVAL_QEXPR)

    // Mark both expressions as evaluable
    lval* x;
    a->cell[1]->type = LVAL_SEXPR;
    a->cell[2]->type = LVAL_SEXPR;

    if (a->cell[0]->data.num) {
        x = lval_eval(e, lval_pop(a, 1));
    } else {
        x = lval_eval(e, lval_pop(a, 2));
    }

    // Delete the argument list and return
    lval_del(a);
    return x;
}
