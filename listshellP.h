/**** listshellP.h ****/  /**** formatted with 4-column tabs ****/

/**************************************************************************
A `ListShell' object is a Motif XmList widget in a shell.  The
application can specify whether the pop-up shell should disappear
or stay active when the user selects an item from the list.
Because it's in a pop-up shell, the application must call the
list-shell realize function.

Sample usage:

#include "listshell.h"
...
...
void main(argc, argv)
int argc;
char *argv[];
{
	void ListSelection(), ...;
	Widget topLevel, ...;
	ListShell list1, list2;
	Arg args[10];
	int i;
	static char *seasons[] = {"Winter", "Spring", "Summer", "Fall", NULL};
	XtAppContext app;

	topLevel = XtAppInitialize(&app, "ApplicationClass",
		(XrmOptionDescList) NULL, 0,
		&argc, argv, (String *) NULL, (ArgList) NULL, 0);
	list1 = listShell_create(topLevel, "ApplicationClass", "listShell1",
		seasons, XmSTRING_DEFAULT_CHARSET, 0, 4, listShell_STAY_UP,
		listShell_WIN_MGR_DECOR);
	listShell_add_callback(list1, ListSelection, 1);
	list2 = listShell_create(topLevel, "ApplicationClass", "listShell2",
		seasons, XmSTRING_DEFAULT_CHARSET, 0, 2, listShell_NO_STAY_UP,
		listShell_MIN_WIN_MGR_DECOR);
	listShell_add_callback(list2, ListSelection, 2);
	...
	...
	XtRealizeWidget(topLevel);
	listShell_realize(list1);
	listShell_realize(list2);
	...
	...
**************************************************************************/


#ifndef _listshellP_h
#define _listshellP_h


/*
Private structure:
*/

typedef struct {
	void *self;
	Widget listShell;
	Widget instance;
	Widget class;
	Widget listPane;
	Widget listPort;
	Widget listList;
	Widget listAction;
	Widget listDismissButton;
	XmString *list_items;
	int num_items;
	int stay_up_behavior;
	int is_active;
} _ListShell;


#endif /* _listshellP_h */

