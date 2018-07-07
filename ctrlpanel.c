/**** ctrlpanel.c ****/  /**** formatted with 4-column tabs ****/

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


/**************************************************************************
A `CtrlPanel' object is a Motif XmRowColumn widget that enforces
a simple horizontal or vertical layout, and facilitates the
association of callback functions with control panel buttons.
The application sets up a null-terminated structure of button-
label and callback pairs and then passes this structure to
ctrlPanel_create().  A {"", "", NULL, NULL} tuple is replaced by an
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


#include "ctrlpanel.h"


/*
Private functions:  currently, none.
*/



/*
Public functions:
*/

/*
ctrlPanel_create() creates a `CtrlPanel' object that
associates XmPushButtons with callback functions.  The
orientation can be vertical or horizontal.
*/

CtrlPanel ctrlPanel_create(parent, instance_name, items, char_set, orientation)
Widget parent;
char *instance_name;
CtrlPanelItem *items;
XmStringCharSet char_set;
int orientation;
{
	Arg args[10];
	int i, n, num_btns, separator;
	CtrlPanel cObject;
	char temp_str[MAX_TEMP_STR + 1];
	XmString xmstr_label;

	if (!(cObject = (CtrlPanel) XtMalloc((Cardinal) sizeof(_CtrlPanel))))
		return NULL;
	cObject->self = cObject;
	/*
	instances have precedence over classes:
	*/
	cObject->instance = XtCreateManagedWidget(instance_name,
		xmFrameWidgetClass, parent, NULL, 0);
	cObject->class = XtCreateManagedWidget("CtrlPanel",
		xmFrameWidgetClass, cObject->instance, NULL, 0);
	i = 0;
	if (orientation == ctrlPanel_VERTICAL) {
		XtSetArg(args[i], XmNorientation, (XtArgVal) XmVERTICAL); i++;
	}
	else if (orientation == ctrlPanel_HORIZONTAL) {
		XtSetArg(args[i], XmNorientation, (XtArgVal) XmHORIZONTAL); i++;
	}
	cObject->ctrlBox = XtCreateManagedWidget("ctrlBox",
		xmRowColumnWidgetClass, cObject->class, args, i);
	for (num_btns = 0; !(!items[num_btns].label && !items[num_btns].name &&
				!items[num_btns].callback); num_btns++)
		/* count the buttons */;
	cObject->buttons =
		(Widget *) XtMalloc((Cardinal) (num_btns * sizeof(Widget)));
	for (n = 0, separator = 1; n < num_btns; n++) {
		i = 0;
		xmstr_label = NULL;
		if (items[n].label && *items[n].label) {
			xmstr_label = XmStringCreateLtoR(items[n].label, char_set);
			XtSetArg(args[i], XmNlabelString, xmstr_label); i++;
		}
		if (items[n].label && !*items[n].label) {/* empty string ==> separator */
			if (orientation == ctrlPanel_VERTICAL) {
				if (!items[n].name)
					sprintf(temp_str, "separator%d", separator++);
				cObject->buttons[n] = XtCreateManagedWidget(
					(items[n].name) ? items[n].name : temp_str,
					xmSeparatorWidgetClass, cObject->ctrlBox, NULL, 0);
			}
		}
		else {
			if (!items[n].name)
				sprintf(temp_str, "button%d", n);
			cObject->buttons[n] = XtCreateManagedWidget(
				(items[n].name) ? items[n].name : temp_str,
				xmPushButtonWidgetClass, cObject->ctrlBox, args, i);
			if (items[n].callback)
				XtAddCallback(cObject->buttons[n], XmNactivateCallback,
					(XtCallbackProc) items[n].callback,
					(XtPointer) items[n].client_data);
		}
		if (xmstr_label)
			XmStringFree(xmstr_label);
	}	/* for each button */
	return cObject;
}	/* ctrlPanel_create */


/*
ctrlPanel_destroy() frees the storage for a `CtrlPanel' object.
*/

void ctrlPanel_destroy(cObject)
CtrlPanel cObject;
{
	XtDestroyWidget(cObject->instance);
	XtFree((char *) cObject->buttons);	/* free the array for the	*/
										/* XmPushButton widget IDs	*/
	XtFree((char *) cObject->self);
}	/* ctrlPanel_destroy */


/*
For the following functions ctrlPanel_<widget_name>() return
the widget IDs of the internal widgets.
*/

Widget ctrlPanel_instance(cObject)
CtrlPanel cObject;
{
	return cObject->instance;
}	/* ctrlPanel_instance */


Widget ctrlPanel_class(cObject)
CtrlPanel cObject;
{
	return cObject->class;
}	/* ctrlPanel_class */


Widget ctrlPanel_ctrlBox(cObject)
CtrlPanel cObject;
{
	return cObject->ctrlBox;
}	/* ctrlPanel_ctrlBox */

