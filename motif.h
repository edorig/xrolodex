/**** motif.h ****/  /**** formatted with 4-column tabs ****/

/********************************************************************
This module provides declarations for miscellaneous functions and
data structures.
********************************************************************/


#ifndef _motif_h
#define _motif_h

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/SashP.h>


/*
Constants:
*/

#define menu_TITLE		-1001
#define menu_SEPARATOR	-1002
#define menu_SUBMENU	-1003
#define menu_ENTRY		-1004
#define menu_TOG_ENTRY	-1005
#define menu_END		-1006


/*
Public data structures:
*/

typedef struct _menu_entry {
	int type;
	char *label;				/* can be NULL */
	char *instance_name;
	void (*callback)();			/* can be NULL */
	XtPointer data;				/* can be NULL */
	struct _menu_entry *menu;	/* can be NULL */
	Widget id;					/* RETURN */
} menu_entry;


/*
Public functions:
*/

extern void create_menus();
extern void remove_sash_traversal();



#endif /* _motif_h */
