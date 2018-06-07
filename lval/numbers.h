#ifndef LVALIO_NUMBERS
#define LVALIO_NUMBERS
#include <stddef.h>
#include "../mpc.h"
#include "base.h"
// Including operations.h for lval_err
#include "operations.h"

// Constructors for numeric data types
lval* lval_long(long x);
lval* lval_double(double x);

// Helper functions to parse the tree and obtain the full string to parse
size_t treeContentsLength(mpc_ast_t* t);
char* concatTreeContents(mpc_ast_t* t);
void concatNodeContents(char* stringToExtend, mpc_ast_t* t, size_t* currentLength);

// Parsing of numeric types
lval* lval_read_double(mpc_ast_t* t);
lval* lval_read_long(mpc_ast_t* t);

// Accessing the numeric data in a lval structure
// TODO: Rename these methods
double lval_getData(lval* x);
void lval_updateData(lval* x, double val, int type);


#endif
