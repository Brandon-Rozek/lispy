#ifndef LVAL_ERROR
#define LVAL_ERROR
#include <stdbool.h>
#include "base.h"

lval* lval_err(char* m);

#define LASSERT(args, cond, err) \
  if (!(cond)) { lval_del(args); return lval_err(err); }

#endif
