#include <stdlib.h>
#include "numbers.h"
#include "error.h"

lval* lval_long(long x) {
	lval* v = (lval *) malloc(sizeof(lval));
	v->type = LVAL_LONG;
	v->data.num = x;
	return v;
}

lval* lval_double(double x) {
	lval* v = (lval *) malloc(sizeof(lval));
	v->type = LVAL_DOUBLE;
	v->data.dec = x;
	return v;
}

size_t treeContentsLength(mpc_ast_t* t) {
	size_t result = strlen(t->contents);
	if (t->children_num == 0) {
		return result;
	}

	for (int i = 0; i < t->children_num; i++) {
		result += treeContentsLength(t->children[i]);
	}
	return result;
}

char* concatTreeContents(mpc_ast_t* t) {
	// Calculate size needed for the string
	size_t totalLength = treeContentsLength(t);

	// Allocate memory for string and null terminator
	char* stringToExtend = (char *) malloc(totalLength + 1);
	// [TODO] Write an allocation error handler

	size_t currentLength = 0;
	concatNodeContents(stringToExtend, t, &currentLength);

	stringToExtend[totalLength] = '\0';

	return stringToExtend;
}

void concatNodeContents(char* stringToExtend, mpc_ast_t* t, size_t* currentLength) {
	size_t leafLength = strlen(t->contents);

	memcpy(stringToExtend + (*currentLength), t->contents, leafLength);
	*currentLength = *currentLength + leafLength;

	if (t->children_num != 0) {
		for (int i = 0; i < t->children_num; i++) {
			concatNodeContents(stringToExtend, t->children[i], currentLength);
		}
	}

}

lval* lval_read_double(mpc_ast_t* t) {
		char* treeString = concatTreeContents(t);

		// Check to see if there's some error in conversion
		errno = 0;
		double x = strtod(treeString, NULL);

		free(treeString);

		return errno != ERANGE ? lval_double(x) : lval_err("Invalid Number");
}
lval* lval_read_long(mpc_ast_t* t) {
		// Grab the contents of all the nodes in the tree otherwise you might not get the string you expect
		char* treeString = concatTreeContents(t);
	
		// Check to see if there's some error in conversion
		errno = 0;
		long x = strtol(treeString, NULL, 10);

		// Free the memory allocated in treestring since it's no longer needed
		free(treeString);
		
		return errno != ERANGE ? lval_long(x) : lval_err("Invalid Number");
}


double lval_getData(lval* x) {
	if (x->type == LVAL_LONG) {
		return x->data.num;
	} 
	return x->data.dec;
} 

void lval_updateData(lval* x, double val, int type) {
	if (type == LVAL_LONG) {
		x->data.num = val;
		return;
	} 
	x->data.dec = val;
} 

