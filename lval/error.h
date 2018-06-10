#ifndef LVAL_ERROR
#define LVAL_ERROR
#include <stdbool.h>
#include "base.h"

lval* lval_err(char* fmt, ...);
char* ltype_name(int t);

#define LASSERT(args, cond, fmt, ...) \
  if (!(cond)) { \
    lval* err = lval_err(fmt, ##__VA_ARGS__); \
    lval_del(args); \
    return err; \
  }

#endif
