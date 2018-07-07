/**** dialog.h ****/  /**** formatted with 4-column tabs ****/

/**************************************************************************
A `Dialog' object is a Motif dialog box in a shell that supports
object-level configuration for modal or nonmodal behavior, number
of buttons, and optional string input.

A `Dialog' object maintains its state between invocations of the
dialog box, making it convenient for the application to retrieve
data stored with the dialog, such as a user's string response (in
the case of a dialog box configured for string input) and/or the
data associated with the most recently selected button.

Sample usage:

#include "dialog.h"
...
...
void main(argc, argv)
int argc;
char *argv[];
{
	void DialogSelection1(), ...;
	Widget topLevel, ...;
	Dialog dialog1, dialog2;
	Arg args[10];
	int i;
	static int one = 1, two = 2, three = 3;
	static DialogButtonItem items1[] = {
		{"Button 1", DialogSelection1, (XtPointer) &one},
		{"Button 2", DialogSelection1, (XtPointer) &two},
		{"Button 3", DialogSelection1, (XtPointer) &three},
		{NULL, NULL, NULL},
	};
	static DialogButtonItem items2[] = {
		{"Dismiss", Dismiss, (XtPointer) "one"},
		{"Button 2", DialogSelection2, (XtPointer) "two"},
		{"Button 3", DialogSelection2, (XtPointer) "three"},
		{"Button 4", DialogSelection2, (XtPointer) "four"},
		{NULL, NULL, NULL},
	};
	XtAppContext app;

	topLevel = XtAppInitialize(&app, "TestDialog",
		(XrmOptionDescList) NULL, 0,
		&argc, argv, (String *) NULL, (ArgList) NULL, 0);
...
...
\***********************************************************************
	dialog_create(app, parent, instance_name, items, num_columns,
		title, prompt, char_set, dialog_position, modal_dialog,
		max_win_mgr, string_input, auto_popdown, default_button);
***********************************************************************\
	dialog1 = dialog_create(app, topLevel, "dialog1", items1, 30,
		"Dialog Box #1", "This is only a test...",
		XmSTRING_DEFAULT_CHARSET, dialog_CENTER, TRUE, FALSE, FALSE, TRUE, 0);
	dialog2 = dialog_create(app, topLevel, "dialog2", items2, 30,
		"Dialog Box #2", "This is only a test...",
		XmSTRING_DEFAULT_CHARSET, dialog_DEFAULT, FALSE, TRUE, TRUE, FALSE, 0);
...
...
	XtRealizeWidget(topLevel);
	dialog_realize(dialog1);
	dialog_realize(dialog2);
...
...

NOTE:  A `Dialog' stores pointers (XtPointer) to client data for the
convenience of the application programmer.  In particular, a `Dialog'
objects stores the client data associated with the most recently
selected button, and provides an access function that retrieves this
value, dialog_get_most_recent_button_response().  Thus, the
application should always pass addresses as client data.  Because
client data can be of any type, there is no reason that the `Dialog'
object should interpret the client data's type, or store it locally.
In particular, if the application wants to associate integer data with
each button, it should define storage for the data in the application
(within the scope of its anticipated retrieval of the data with
dialog_get_most_recent_button_response()), pass the address of this
data in the array of button data, and then interpret (dereference)
the address that's returned.  E.g., in the first example below, it's
incorrect to pass the *value* of the integer, because Xt will store
that value internally and it may then be out of scope in a future
reference.  Examples:

main(...)
...
{
	static DialogButtonItem items1[] = {
		{"Button 1", DialogSelection1, (XtPointer) 1},	// wrong //
		{"Button 2", DialogSelection1, (XtPointer) 2},	// wrong //
		{"Button 3", DialogSelection1, (XtPointer) 3},	// wrong //
		{NULL, NULL, NULL},
	};
	...
	...
	{
	int v = (int) dialog_get_most_recent_button_response(dialog); // wrong //
	printf("Most recently selected button's value: %d\n", v); // wrong //
	}
	...
}

main(...)
...
{
	static int one = 1, two = 2, three = 3;
	static DialogButtonItem items1[] = {
		{"Button 1", DialogSelection1, (XtPointer) &one},	// right //
		{"Button 2", DialogSelection1, (XtPointer) &two},	// right //
		{"Button 3", DialogSelection1, (XtPointer) &three},	// right //
		{NULL, NULL, NULL},
	};
	...
	...
	{
	int *v = (int *) dialog_get_most_recent_button_response(dialog); // right //
	if (v)
		printf("Most recently selected button's value: %d\n", *v); // right //
	}
	...
}
**************************************************************************/


#ifndef _dialog_h
#define _dialog_h

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/Text.h>

#include "dialogP.h"
#include "motif.h"


/*
Constants:
*/

#define dialog_BTN_NULL			-1

#define dialog_CENTER_PRIMARY	-100
#define dialog_CENTER_SCREEN	-101
#define dialog_POINTER			-102
#define dialog_DEFAULT			-103


/*
Public opaque pointer:
*/

typedef _Dialog *Dialog;


/*
Public convenience structure access for dialog
box buttons and their callback functions:
*/

typedef _DialogButtonItem *DialogButtonItemList;
typedef _DialogButtonItem DialogButtonItem;


/*
Public functions:
*/

extern Dialog dialog_create();
extern void dialog_destroy();
extern void dialog_realize();
extern void dialog_activate();
extern void dialog_deactivate();
extern int dialog_modal_prompt();
extern void dialog_set_prompt();
extern char *dialog_get_text();
extern void dialog_set_text();
extern void dialog_forget_text();
extern void dialog_remember_text();
extern XtPointer dialog_get_most_recent_button_response();
extern void dialog_cancel();
extern Widget dialog_dialogShell();
extern Widget dialog_instance();
extern Widget dialog_class();
extern Widget dialog_dialogPane();
extern Widget dialog_dialogControl();
extern Widget dialog_dialogPrompt();
extern Widget dialog_dialogText();
extern Widget dialog_dialogAction();


#endif /* _dialog_h */

