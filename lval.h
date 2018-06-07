
#ifndef LVAL_H
#define LVAL_H

// Bring in the lval struct and lispy types
#include "lval/base.h"
// Error functionality
#include "lval/error.h"
// Adds functionality for the numeric data type
#include "lval/numbers.h"
// Adds functionality for the expression (Q or S) data type
#include "lval/expressions.h"
// Add read, write, and error functionality
#include "lval/operations.h"
// Add the ability to print out lval structures
#include "lval/io.h"

#endif
