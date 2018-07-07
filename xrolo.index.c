/**** xrolo.index.c ****/  /**** formatted with 4-column tabs ****/

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
`xrolo.index.c' stores a reference to a `ListShell' object from
`xrolo.c' as `entryIndex'.  It then uses xrolo_db_build_index_array()
from `xrolo.db.c' to build a null-terminated, dynamic array of
pointers to strings, where each string contains the first line of each
rolodex entry.  The routines can be used to provide a high-level
interface to the task of building and freeing the dynamic arrays that
are created in `xrolo.db.c' and `listshell.c'.

Note that `xrolo.c' allocates, realizes, and ultimately frees the
`ListShell' object.  It should not be done here.
********************************************************************/


#include "xrolo.index.h"



/*
Private support functions:
*/


/*
Private globals (these maintain the current index state):
*/

static ListShell entryIndex;
static char **index_array = NULL;


/*
Public functions:
*/

/*
xrolo_index_create() currently only stores a reference to
the index created in `xrolo.c'.
*/

void xrolo_index_create(entry_index)
ListShell entry_index;
{
	entryIndex = entry_index;
}	/* xrolo_index_create */


/*
xrolo_index_build() first builds a character-oriented array of
primary key data using xrolo_db_build_index_array(), and then passes
it to listShell_make_list() to build a XmList widget-oriented array.
The latter function also updates the list widget.
*/

int xrolo_index_build(char_set, line)
XmStringCharSet char_set;
int line;
{
	xrolo_db_free_index_array();
	index_array = xrolo_db_build_index_array(line);
	if (!index_array)
		return FALSE;
	listShell_free_list(entryIndex);
	listShell_make_list(entryIndex, index_array, char_set);
	return TRUE;
}	/* xrolo_index_build */


/*
xrolo_index_destroy() calls functions to free the dynamic, intermediate
arrays used to build the contents of the list widget.
*/

void xrolo_index_destroy()
{
	listShell_free_list(entryIndex);
	xrolo_db_free_index_array();
}	/* xrolo_index_destroy */

