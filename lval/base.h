#ifndef LVAL_BASE
#define LVAL_BASE
typedef union typeval {
	long num;
	double dec;
} TypeVal;

// A lispy value can either be a number, error, symbol, or an expression
typedef struct lval {
	int type;
	TypeVal data;

	// Error and symbols contain string data
	char* err;
	char* sym;

	// Count and pointer to a list of lval*
	int count;
	struct lval** cell;
} lval;

// Possible lispy value types
enum { LVAL_ERR, LVAL_LONG, LVAL_DOUBLE, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR };
#endif
