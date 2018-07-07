/**** strstr.h ****/  /**** formatted with 4-column tabs ****/

/********************************************************************
This module provides forward and backward string search routines.
********************************************************************/


#ifndef _strstr_h
#define _strstr_h


#include <ctype.h>


/*
Constants:
*/

#define string_NO_MATCH		-1
#define string_INSENSITIVE	-2
#define string_SENSITIVE	-3


/*
Public functions:
*/

extern int string_search();
extern int string_search_reverse();
extern int string_search_insensitive();
extern int string_search_reverse_insensitive();



#endif /* _strstr_h */
