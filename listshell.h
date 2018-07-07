/**** listshell.h ****/  /**** formatted with 4-column tabs ****/

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


#ifndef _listshell_h
#define _listshell_h


#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/List.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>


#include "listshellP.h"
#include "motif.h"


/*
Constants used to control selection behavior:
*/

#define listShell_STAY_UP			-100
#define listShell_NO_STAY_UP		-101

#define listShell_WIN_MGR_DECOR		-200
#define listShell_MIN_WIN_MGR_DECOR	-201


/*
Public opaque pointer:
*/

typedef _ListShell *ListShell;


/*
Public functions:
*/

extern ListShell listShell_create();
extern void listShell_destroy();
extern void listShell_make_list();
extern void listShell_free_list();
extern void listShell_activate();
extern void listShell_realize();
extern void listShell_deactivate();
extern void listShell_add_callback();
extern Widget listShell_listShell();
extern Widget listShell_instance();
extern Widget listShell_class();
extern Widget listShell_listPane();
extern Widget listShell_listAction();
extern Widget listShell_listDismissButton();
extern Widget listShell_listPort();
extern Widget listShell_listList();


#endif /* _listshell_h */

