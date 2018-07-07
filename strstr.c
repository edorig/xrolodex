/**** strstr.c ****/  /**** formatted with 4-column tabs ****/

/**********************************************************************
*  Copyright (c) 1991 - 1996 Iris Computing Laboratories.
*
*  This software is provided for demonstration purposes only.  As
*  freely-distributed, modifiable source code, this software carries
*  absolutely no warranty.  Iris Computing Laboratories disclaims
*  all warranties for this software, including any implied warranties
*  of merchantability and fitness, and shall not be liable for
*  damages of any type resulting from its use.
*  Permission to use, copy, modify, and distribute this source code
*  for any purpose and without fee is hereby granted, provided that
*  the above copyright and this permission notice appear in all copies
*  and supporting documentation, and provided that Iris Computing
*  Laboratories not be named in advertising or publicity pertaining
*  to the redistribution of this software without specific, written
*  prior permission.
**********************************************************************/


/********************************************************************
This module provides forward and backward string search routines.
They should be compiled and stored in a miscellaneous library.
NOTE:  For string-search performance reasons, it is undesirable
to combine these functions to produce, say, two functions, with
selectable case sensitivity.
********************************************************************/


#include "strstr.h"



/*
Private functions:
*/

static char get_upper();



/*
string_search() searches a buffer (any buffer) beginning at the
designated offset and returns the offset of the first occurrence
of the search text.
*/

int string_search(buffer, search_text, offset)
char *buffer, *search_text;
register int offset;
{
	register int next;
	register int begin_match_offset = offset;

	if (*search_text)
		for (next = 0; buffer[offset]; offset++)
			if (buffer[offset] == search_text[next]) {
				begin_match_offset = offset;
				while (search_text[next] &&
						buffer[offset++] == search_text[next++])
					;
				if (!search_text[next] &&
						buffer[--offset] == search_text[--next])
					return begin_match_offset;
				offset = begin_match_offset;
				next = 0;
			}
	return string_NO_MATCH;
}   /* string_search */


/*
string_search_insensitive() searches a buffer (any buffer) beginning
at the designated offset and returns the offset of the first occurrence
of the search text.  It ignores upper- and lowercase differences.
*/

int string_search_insensitive(buffer, search_text, offset)
char *buffer, *search_text;
register int offset;
{
	register int next;
	register int begin_match_offset = offset;

	if (*search_text)
		for (next = 0; buffer[offset]; offset++)
			if (get_upper(buffer[offset]) == get_upper(search_text[next])) {
				begin_match_offset = offset;
				while (search_text[next] &&
				get_upper(buffer[offset++]) == get_upper(search_text[next++]))
					;
				if (!search_text[next] &&
				get_upper(buffer[--offset]) == get_upper(search_text[--next]))
					return begin_match_offset;
				offset = begin_match_offset;
				next = 0;
			}
	return string_NO_MATCH;
}   /* string_search_insensitive */


/*
string_search_reverse() searches a buffer (any buffer) beginning at
the designated offset and returns the offset of the first occurrence
of the search text.  Currently, string_search_reverse() uses the
pedestrian approach -- search through the buffer backward until there's
a match, then search through the search string forward -- it should
search both strings backward.
*/

int string_search_reverse(buffer, search_text, offset)
char *buffer, *search_text;
register int offset;
{
	register int next;
	register int begin_match_offset = offset;

	if (*search_text) {
		for (next = 0; offset > -1; offset--) {
			if (buffer[offset] == search_text[next]) {
				begin_match_offset = offset;
				while (search_text[next] &&
						buffer[offset++] == search_text[next++])
					;
				if (!search_text[next] &&
						buffer[--offset] == search_text[--next]) {
					return begin_match_offset;
				}
				offset = begin_match_offset;
				next = 0;
			}
		}
	}
	return string_NO_MATCH;
}   /* string_search_reverse */


/*
string_search_reverse_insensitive() searches a buffer (any buffer)
beginning at the designated offset and returns the offset of the
first occurrence of the search text.  Currently,
string_search_reverse_insensitive() uses the pedestrian approach --
search through the buffer backward until there's a match, then search
through the search string forward -- it should search both strings
backward.
*/

int string_search_reverse_insensitive(buffer, search_text, offset)
char *buffer, *search_text;
register int offset;
{
	register int next;
	register int begin_match_offset = offset;

	if (*search_text) {
		for (next = 0; offset > -1; offset--) {
			if (get_upper(buffer[offset]) == get_upper(search_text[next])) {
				begin_match_offset = offset;
				while (search_text[next] &&
				get_upper(buffer[offset++]) == get_upper(search_text[next++]))
					;
				if (!search_text[next] &&
				get_upper(buffer[--offset]) == get_upper(search_text[--next])) {
					return begin_match_offset;
				}
				offset = begin_match_offset;
				next = 0;
			}
		}
	}
	return string_NO_MATCH;
}   /* string_search_reverse */


/*
Private functions:
*/

static char get_upper(c)
char c;
{
	return islower(c) ? toupper(c) : c;
}	/* get_upper */

