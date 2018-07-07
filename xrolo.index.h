/**** xrolo.index.h ****/  /**** formatted with 4-column tabs ****/

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


#include "listshell.h"
#include "xrolo.db.h"


/*
Public functions:
*/

extern void xrolo_index_create();
extern void xrolo_index_destroy();
extern int xrolo_index_build();
