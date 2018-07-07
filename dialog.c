/**** dialog.c ****/  /**** formatted with 4-column tabs ****/

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


#include "dialog.h"

/*
Private callback functions:
*/

static void DialogDismiss();
static void UpdateButtonResponse();


/*
Private support functions:
*/

static void popup_and_wait_for_button_response();
static void position_dialog();


/*
Public functions:
*/

/*
dialog_create() creates an dialog box object, optionally with
a single-line edit window for the user's string response.
*/

Dialog dialog_create(app, parent, instance_name, items, num_columns,
	title, prompt, char_set, dialog_position, modal_dialog,
	max_win_mgr, string_input, auto_popdown, default_button)
XtAppContext app;
Widget parent;
char *instance_name;
DialogButtonItem *items;
int num_columns;
char *title, *prompt;
XmStringCharSet char_set;
int dialog_position;
Boolean modal_dialog, max_win_mgr, string_input, auto_popdown;
int default_button;
{
	Arg args[15];
	int fraction_base, i, left_pos, n, num_btns, space_incr;
	XmString string;
	Dialog dObject;
#ifdef RESTRICT_ACTION_AREA
	Dimension height, margin_height;
#endif

	for (num_btns = 0; items[num_btns].label; num_btns++)
		/* count the items */;
	if (num_btns < 1)
		return 0;
	if (!(dObject = (Dialog) XtMalloc((Cardinal) sizeof(_Dialog))))
		return 0;
	dObject->self = dObject;
	dObject->parent = parent;
	dObject->num_buttons = num_btns;
	dObject->app = app;		/* used with modal dialog boxes */
	dObject->char_set = char_set;
	dObject->dialog_position = dialog_position;
	dObject->modal_dialog = modal_dialog;
	dObject->max_win_mgr = max_win_mgr;
	dObject->dialogText = NULL;	/* instantiation is optional */
	dObject->most_recent_button_data = NULL;
	dObject->button_pressed = dialog_BTN_NULL;	/* not currently used */
	dObject->remember_string_response = True;
	i = 0;
	if (title && *title) {
		XtSetArg(args[i], XmNtitle, (XtArgVal) title); i++;
	}
	XtSetArg(args[i], XmNallowShellResize, (XtArgVal) True); i++;
	XtSetArg(args[i], XmNmappedWhenManaged, (XtArgVal) False); i++;
	dObject->dialogShell = XtCreatePopupShell(instance_name,
		max_win_mgr ? topLevelShellWidgetClass : transientShellWidgetClass,
		parent, args, i);
	/*
	instances have precedence over classes:
	*/
	dObject->instance = XtCreateManagedWidget(instance_name,
		xmFrameWidgetClass, dObject->dialogShell, NULL, 0);
	dObject->class = XtCreateManagedWidget("Dialog",
		xmFrameWidgetClass, dObject->instance, NULL, 0);
	i = 0;
	XtSetArg(args[i], XmNsashWidth, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNsashHeight, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNsashIndent, (XtArgVal) -1); i++;
	dObject->dialogPane = XtCreateManagedWidget("dialogPane",
		xmPanedWindowWidgetClass, dObject->class, args, i);
	i = 0;
	XtSetArg(args[i], XmNallowResize, (XtArgVal) True); i++;
	dObject->dialogControl = XtCreateManagedWidget("dialogControl",
		xmFormWidgetClass, dObject->dialogPane, args, i);
	
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	if (!string_input) {
		XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	}
	if (!prompt)
		prompt = "";
	if (!*prompt)
		prompt = " ";
	string = XmStringCreateLtoR(prompt, char_set);
	XtSetArg(args[i], XmNlabelString, string); i++;
	dObject->dialogPrompt = XtCreateManagedWidget("dialogPrompt",
		xmLabelWidgetClass, dObject->dialogControl, args, i);
	XmStringFree(string);
	if (string_input) {
		i = 0;
		XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
		XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
		XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
		XtSetArg(args[i], XmNtopWidget, (XtArgVal) dObject->dialogPrompt); i++;
		XtSetArg(args[i], XmNvalue, (XtArgVal) ""); i++;
		if (num_columns > 0) {
			XtSetArg(args[i], XmNcolumns, (XtArgVal) num_columns); i++;
		}
		dObject->dialogText = XtCreateManagedWidget("dialogText",
			xmTextWidgetClass, dObject->dialogControl, args, i);
	}
	if (default_button != dialog_NO_DEFAULT_BTN)
		if (default_button < 0 || default_button >= num_btns)
			default_button = 0;
	space_incr = 1;
	if (num_btns == 1)
		fraction_base = 5;
	else if (num_btns == 2)
		fraction_base = (num_btns * 2) + 1;
	else {
		space_incr = dialog_BTN_WIDTH_FACTOR;
		fraction_base = (dialog_BTN_WIDTH_FACTOR * num_btns) +
			((dialog_BTN_WIDTH_FACTOR / dialog_BTN_SPACE_FACTOR) *
				(num_btns * 2));
	}
	i = 0;
	XtSetArg(args[i], XmNfractionBase, (XtArgVal) fraction_base); i++;
	XtSetArg(args[i], XmNallowResize, (XtArgVal) True); i++;
	dObject->dialogAction = XtCreateManagedWidget("dialogAction",
		xmFormWidgetClass, dObject->dialogPane, args, i);
	dObject->buttons = (Widget *)
		XtMalloc((Cardinal) (num_btns * sizeof(Widget)));
	for (left_pos = n = 0; n < num_btns; n++) {
		i = 0;
		XtSetArg(args[i], XmNuserData, (XtArgVal) items[n].client_data); i++;
		XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
		XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
		XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_POSITION); i++;
		left_pos += space_incr / dialog_BTN_SPACE_FACTOR;
		XtSetArg(args[i], XmNleftPosition, (XtArgVal) (space_incr == 1) ?
			((num_btns == 1) ? 2 : ((2 * n) + 1)) :
			left_pos); i++;
		XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_POSITION); i++;
		XtSetArg(args[i], XmNrightPosition, (XtArgVal) (space_incr == 1) ?
			((num_btns == 1) ? 3 : ((n + 1) * 2)) :
					(left_pos + space_incr)); i++;
		left_pos += space_incr + (space_incr / dialog_BTN_SPACE_FACTOR);
		if (default_button != dialog_NO_DEFAULT_BTN) {
			XtSetArg(args[i], XmNshowAsDefault,
				(XtArgVal) (n == default_button)); i++;
			XtSetArg(args[i], XmNdefaultButtonShadowThickness, (XtArgVal) 1); i++;
		}
		dObject->buttons[n] = XtCreateManagedWidget(items[n].label,
			xmPushButtonWidgetClass, dObject->dialogAction, args, i);
		XtAddCallback(dObject->buttons[n], XmNactivateCallback,
			(XtCallbackProc) UpdateButtonResponse, (XtPointer) dObject);
		if (items[n].callback)
			XtAddCallback(dObject->buttons[n], XmNactivateCallback,
				(XtCallbackProc) items[n].callback,
				(XtPointer) items[n].client_data);
		if (auto_popdown)
			XtAddCallback(dObject->buttons[n], XmNactivateCallback,
				(XtCallbackProc) DialogDismiss, (XtPointer) dObject);
	}
	/*
	restrict the height of the action area:
	*/
#ifdef RESTRICT_ACTION_AREA
	i = 0;
	XtSetArg(args[i], XmNmarginHeight, &margin_height); i++;
	XtGetValues(dObject->dialogAction, args, i);
	i = 0;
	XtSetArg(args[i], XmNheight, &height); i++;
	XtGetValues(dObject->buttons[0], args, i);
	i = 0;
	XtSetArg(args[i], XmNpaneMinimum,
		(XtArgVal) (height + (margin_height * 2))); i++;
	XtSetArg(args[i], XmNpaneMaximum,
		(XtArgVal) (height + (margin_height * 2))); i++;
#else
	i = 0;
	XtSetArg(args[i], XmNskipAdjust, (XtArgVal) True); i++;
#endif
	if (default_button != dialog_NO_DEFAULT_BTN) {
		XtSetArg(args[i], XmNdefaultButton,
			(XtArgVal) dObject->buttons[default_button]); i++;
	}
	XtSetValues(dObject->dialogAction, args, i);
	remove_sash_traversal(dObject->dialogPane);
	return dObject;
}	/* dialog_create */


/*
dialog_destroy() frees the storage for a `Dialog' object.
*/

void dialog_destroy(dObject)
Dialog dObject;
{
	XtDestroyWidget(dObject->dialogShell);
	XtFree((char *) dObject->buttons);	/* free the array for the	*/
										/* XmPushButton widget IDs	*/
	XtFree((char *) dObject->self);
}	/* dialog_destroy */


/*
dialog_realize() realizes the editor object, specifically, the
top-level shell window used by the search-and-replace window.
*/

void dialog_realize(dObject)
Dialog dObject;
{
	XtRealizeWidget(dObject->dialogShell);
}	/* dialog_realize */


/*
dialog_activate() maps the dialog box.  If the dialog box has a
modal configuration, it blocks the application until the user
selects a command button.  See dialog_modal_prompt() for an
alternative approach for activating a modal dialog box.
*/

void dialog_activate(dObject)
Dialog dObject;
{
	if (dObject->dialogText && !dObject->remember_string_response)
		XmTextSetString(dObject->dialogText, "");		/* clean slate */

	position_dialog(dObject);
	if (dObject->modal_dialog)
		popup_and_wait_for_button_response(dObject);
	else
		XtPopup(dObject->dialogShell, XtGrabNone);
}	/* dialog_activate */


/*
dialog_deactivate() removes the dialog box.
*/

void dialog_deactivate(dObject)
Dialog dObject;
{
	XtPopdown(dObject->dialogShell);
}	/* dialog_deactivate */


/*
dialog_modal_prompt() maps the dialog box, waits for a response,
and returns the (integer) index of the chosen button, zero-based.
It does not return the value of the client data stored with the
chosen button.  See dialog_activate() for an alternative approach
for activating a dialog box.
*/

int dialog_modal_prompt(dObject, prompt)
Dialog dObject;
char *prompt;
{
	if (dObject->modal_dialog) {
		if (dObject->dialogText && !dObject->remember_string_response)
			XmTextSetString(dObject->dialogText, "");	/* clean slate */

		position_dialog(dObject);
		dialog_set_prompt(dObject, prompt);
		popup_and_wait_for_button_response(dObject);
		return dObject->button_pressed;
	}
	else
		return dialog_BTN_NULL;
}	/* dialog_modal_prompt */


/*
dialog_set_prompt() allows the application to set the
dialog box's prompt, independent of invoking the dialog
box.  The temporary `XmString' variable is freed--this
function could be called a zillion times.
*/

void dialog_set_prompt(dObject, prompt)
Dialog dObject;
char *prompt;
{
	if (prompt && *prompt) {
		Arg args[1];
		XmString str_prompt = XmStringCreateLtoR(prompt, dObject->char_set);

		XtSetArg(args[0], XmNlabelString, (XtArgVal) str_prompt);
		XtSetValues(dObject->dialogPrompt, args, 1);	/* fixed */
		XmStringFree(str_prompt);
	}
}	/* dialog_set_prompt */


/*
dialog_get_text() returns the user's string response from
the text edit widget.  The caller must free the text with
XtFree().  Note that string response may not be enabled for
a particular dialog box instance.
*/

char *dialog_get_text(dObject)
Dialog dObject;
{
	return (dObject->dialogText) ?
		(char *) XmTextGetString(dObject->dialogText) : NULL;
}	/* dialog_get_text */


/*
dialog_set_text() allows the application to set, or update,
the user's string response from the text-edit widget.
*/

void dialog_set_text(dObject, string)
Dialog dObject;
char *string;
{
	if (dObject->dialogText && string && *string)
		XmTextSetString(dObject->dialogText, string);
}	/* dialog_set_text */


/*
dialog_forget_text() modifies the behavior of a dialog box
with a string-response field, so that the dialog is mapped each
time with an empty text entry field.
*/

void dialog_forget_text(dObject)
Dialog dObject;
{
	dObject->remember_string_response = False;
}	/* dialog_forget_text */


/*
dialog_remember_text() modifies the behavior of a dialog box
with a string-response field, so that the dialog is mapped each
time without clearing the text entry field.
*/

void dialog_remember_text(dObject)
Dialog dObject;
{
	dObject->remember_string_response = True;
}	/* dialog_remember_text */


/*
dialog_get_most_recent_button_response() returns the user's most
recent button response/selection.
*/

XtPointer dialog_get_most_recent_button_response(dObject)
Dialog dObject;
{
	return dObject->most_recent_button_data;
}	/* dialog_get_most_recent_button_response */


/*
dialog_cancel() can be used to cancel a dialog box without
waiting for a user's response. For example, if the user selects
the window manager's "Close" button in a modal dialog, the
application can call this function to terminate the dialog.
In most cases, the application will interpret this action to
be equivalent to selecting one of the dialog's button, for
example, the "Cancel" button.
*/

void dialog_cancel(dObject, button_pressed)
Dialog dObject;
int button_pressed;
{
	dObject->button_pressed = button_pressed;
	XtPopdown(dObject->dialogShell);
}	/* dialog_cancel */


/*
For the following functions dialog_<widget_name>() returns
the widget IDs of the internal widgets.  Many of these
aren't really needed and could be deleted.  Using macros is
another possibility.  And, of course, Xt provides functions
for obtaining this information.
*/

Widget dialog_dialogShell(dObject)
Dialog dObject;
{
	return dObject->dialogShell;
}	/* dialog_dialogShell */


Widget dialog_instance(dObject)
Dialog dObject;
{
	return dObject->instance;
}	/* dialog_instance */


Widget dialog_class(dObject)
Dialog dObject;
{
	return dObject->class;
}	/* dialog_class */


Widget dialog_dialogPane(dObject)
Dialog dObject;
{
	return dObject->dialogPane;
}	/* dialog_dialogPane */


Widget dialog_dialogControl(dObject)
Dialog dObject;
{
	return dObject->dialogControl;
}	/* dialog_dialogControl */


Widget dialog_dialogPrompt(dObject)
Dialog dObject;
{
	return dObject->dialogPrompt;
}	/* dialog_dialogPrompt */


Widget dialog_dialogText(dObject)
Dialog dObject;
{
	return dObject->dialogText;
}	/* dialog_dialogText */


Widget dialog_dialogAction(dObject)
Dialog dObject;
{
	return dObject->dialogAction;
}	/* dialog_dialogAction */


/*
Private callback functions:
*/

/*
DialogDismiss() is a private callback that removes
the dialog box.
*/
/*ARGSUSED*/
static void DialogDismiss(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	Dialog dObject = (Dialog) client_data;

	XtPopdown(dObject->dialogShell);
}	/* DialogDismiss */


/*
UpdateButtonResponse() is a private callback that records
the data associated with the most recent button selection.
It also sets an indicator variable that signals that a
button has been selected; the latter is used only with
dialogs configured for modal behavior.
*/
/*ARGSUSED*/
static void UpdateButtonResponse(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	Dialog dObject = (Dialog) client_data;
	Arg args[1];
	XtPointer user_data;
	int i;

	XtSetArg(args[0], XmNuserData, &user_data);
	XtGetValues(w, args, 1);	/* fixed */
	dObject->most_recent_button_data = user_data;
	for (i = 0; i < dObject->num_buttons; i++)
		if (w == dObject->buttons[i]) {
			dObject->button_pressed = i;
			break;
		}
}	/* UpdateButtonResponse */


/*
Private support functions:
*/

/*
popup_and_wait_for_button_response() is called to
synchronize execution for a modal dialog.
*/

static void popup_and_wait_for_button_response(dObject)
Dialog dObject;
{
	dObject->button_pressed = dialog_BTN_NULL;
	XtPopup(dObject->dialogShell, XtGrabExclusive);
	while (dObject->button_pressed == dialog_BTN_NULL) {
		XEvent event;

		XtAppNextEvent(dObject->app, &event);
		XtDispatchEvent(&event);
	}
}	/* popup_and_wait_for_button_response */


/*
position_dialog() positions the dialog on the screen.  Currently, this
function does not enforce any dialog placement policy.  That is, it
allows the programmer to position a top-level shell, even though it
is generally accepted that this should be left up to the user--the
programmer is in control.
*/

static void position_dialog(dObject)
Dialog dObject;
{
	Arg args[5];
	int i;
	Dimension d_width, d_height;

	if (dObject->dialog_position == dialog_DEFAULT)
		return;
	if (!XtIsRealized(dObject->dialogShell))
		return;
	i = 0;
	XtSetArg(args[i], XmNwidth, &d_width); i++;
	XtSetArg(args[i], XmNheight, &d_height); i++;
	XtGetValues(dObject->dialogShell, args, i);
	if (dObject->dialog_position == dialog_CENTER_PRIMARY) {
		Dimension p_width, p_height;
		Position p_x, p_y, d_x, d_y;

		i = 0;
		XtSetArg(args[i], XmNwidth, &p_width); i++;
		XtSetArg(args[i], XmNheight, &p_height); i++;
		XtGetValues(dObject->parent, args, i);
		XtTranslateCoords(dObject->parent, 0, 0, &p_x, &p_y);
		d_x = p_x + ((p_width - d_width) / 2);
		d_y = p_y + ((p_height - d_height) / 2);
		/*
		Motif will keep it on-screen:
		*/
		i = 0;
		XtSetArg(args[i], XmNx, (XtArgVal) d_x); i++;
		XtSetArg(args[i], XmNy, (XtArgVal) d_y); i++;
	}
	else if (dObject->dialog_position == dialog_CENTER_SCREEN) {
		Screen *screen = XtScreen(dObject->dialogShell);

		i = 0;
		XtSetArg(args[i], XmNx,
			(XtArgVal) ((XWidthOfScreen(screen) - (int) d_width) / 2)); i++;
		XtSetArg(args[i], XmNy,
			(XtArgVal) ((XHeightOfScreen(screen) - (int) d_height) / 2)); i++;
	}
	else if (dObject->dialog_position == dialog_POINTER) {
		Window dummy_win;
		int root_x, root_y, dummy_xy;
		unsigned int dummy_keys;

		XQueryPointer(XtDisplay(dObject->dialogShell),
			XtWindow(dObject->dialogShell),
			&dummy_win, &dummy_win, &root_x, &root_y,
			&dummy_xy, &dummy_xy, &dummy_keys);
		/*
		Motif will keep it on-screen:
		*/
		i = 0;
		XtSetArg(args[i], XmNx, (XtArgVal) (root_x - ((int) d_width / 2))); i++;
		XtSetArg(args[i], XmNy, (XtArgVal) (root_y - ((int) d_height / 2))); i++;
	}
	XtSetValues(dObject->dialogShell, args, i);
}	/* position_dialog */

