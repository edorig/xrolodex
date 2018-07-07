/**** ctrlpanelP.h ****/  /**** formatted with 4-column tabs ****/

/**************************************************************************
A `CtrlPanel' object is a Motif XmRowColumn widget that enforces
a simple horizontal or vertical layout, and facilitates the
association of callback functions with control panel buttons.
The application sets up a null-terminated structure of button-
label and callback pairs and then passes this structure to
ctrlPanel_create().  A {"", NULL, NULL} tuple is replaced by an
XmSeparator.

The data structures for using `CtrlPanel' are illustrated by the
following code segment:

#include "ctrlpanel.h"
...
...

static void Quit(), Print();	// callbacks //
...
...
void main(argc, argv)
int argc;
char *argv[];
{
	Widget topLevel;
	CtrlPanel panel;
	static CtrlPanelItem items[] = {			// array of `CtrlPanelItem's	//
		// {<label>, <name>, <callback>, <data>} tuples //
		{"Quit", "quitButton", Quit, NULL},
		{"Print", "printButton", Print, NULL},
		NULL,									// null-terminated //
	};
	XtAppContext app;

	topLevel = XtAppInitialize(&app, "ApplicationClass",
		(XrmOptionDescList) NULL, 0,
		&argc, argv, (String *) NULL, (ArgList) NULL, 0);
	panel = ctrlPanel_create(topLevel, "panel", items,
		XmSTRING_DEFAULT_CHARSET, ctrlPanel_HORIZONTAL);
	...
	...
	ctrlPanel_destroy(panel);			// probably from a callback //
	...
	...
**************************************************************************/


#ifndef _ctrlpanelP_h
#define _ctrlpanelP_h


/*
Constants:
*/

#define MAX_TEMP_STR	30


/*
Private structure for the CtrlPanel widget:
*/

typedef struct {
	void *self;
	Widget instance;
	Widget class;
	Widget ctrlBox;
	Widget *buttons;		/* or `WidgetList buttons;' */
} _CtrlPanel;


/*
Private convenience structure for the items and their callbacks:
*/

typedef struct {
	char *label;
	char *name;
	void (*callback)();		/* should NOT be XtCallbackProc */
	XtPointer client_data;
} _CtrlPanelItem;


#endif /* _ctrlpanelP_h */

