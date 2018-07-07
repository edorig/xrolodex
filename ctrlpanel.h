/**** ctrlpanel.h ****/  /**** formatted with 4-column tabs ****/

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


#ifndef _ctrlPanel_h
#define _ctrlPanel_h


#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/RowColumn.h>


#include "ctrlpanelP.h"


/*
Constants used to control horizontal or vertical orientation:
*/

#define ctrlPanel_HORIZONTAL	-100
#define ctrlPanel_VERTICAL		-101
#define ctrlPanel_NULL			-102


/*
Public opaque pointer:
*/

typedef _CtrlPanel *CtrlPanel;


/*
Public convenience structure access for control
panel buttons and their callback functions:
*/

typedef _CtrlPanelItem *CtrlPanelList;
typedef _CtrlPanelItem CtrlPanelItem;


/*
Public functions:
*/

extern CtrlPanel ctrlPanel_create();
extern void ctrlPanel_destroy();
extern Widget ctrlPanel_instance();
extern Widget ctrlPanel_class();
extern Widget ctrlPanel_ctrlBox();


#endif /* _ctrlPanel_h */

