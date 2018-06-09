#ifndef LVAL_BASE
#define LVAL_BASE

struct lval;
struct lenv;

typedef struct lval lval;
typedef struct lenv lenv;

typedef lval* (*lbuiltin) (lenv*, lval*);

typedef union typeval {
	long num;
	double dec;
} TypeVal;

// A lispy value can either be a number, error, symbol, or an expression
struct lval {
	int type;
	TypeVal data;

	// Error and symbols contain string data
	char* err;
	char* sym;
	lbuiltin fun;

	// Count and pointer to a list of lval*
	int count;
	lval** cell;
};

// Possible lispy value types
enum { LVAL_ERR, LVAL_LONG, LVAL_DOUBLE, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR, LVAL_FUN };
#endif
