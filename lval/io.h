
#ifndef LVAL_IO
#define LVAL_IO
#include "base.h"

void flval_expr_print(FILE* stream, lval* v, char open, char close);
void flval_print(FILE* stream, lval* v);
void lval_print(lval* v);
void lval_println(lval* v);

#endif
