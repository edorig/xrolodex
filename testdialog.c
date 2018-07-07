/**** testdialog.c ****/  /**** formatted with 4-column tabs ****/


#include <stdio.h>

#include "dialog.h"

#include <Xm/RowColumn.h>
#include <Xm/Protocols.h>


/*
Functions:
*/

static void Quit(), Dismiss(), TestDialog();
static void DialogSelection1(), DialogSelection2(), DialogSelection3();
static void DialogCancel();
static void handle_close_buttons();


/*
Private data:
*/

static Widget topLevel, buttonBox, buttonTest1, buttonTest2,
	buttonTest3, buttonQuit;

static Dialog dialog1, dialog2, dialog3;


/*
main() creates a top-level window with four buttons, three
of which activate top-level or transient dialog boxes.  There are
several possible combinations of modal/nonmodal, stay-up/no-stay-up,
and string-input/no-string-input dialog boxes; three combinations
are tested here.
*/

void main(argc, argv)
int argc;
char *argv[];
{
	Arg args[10];
	int i;
	static int one = 1, two = 2, three = 3;
	static DialogButtonItem items1[] = {
		{"Button 1", DialogSelection1, (XtPointer) &one},
		{"Button 2", DialogSelection1, (XtPointer) &two},
		{"Button 3", DialogSelection1, (XtPointer) &three},
		{NULL, NULL, NULL},
/*		NULL		** or just provide a NULL tuple ** */
	};
	static DialogButtonItem items2[] = {
		{"Dismiss", Dismiss, (XtPointer) "one"},
		{"Button 2", DialogSelection2, (XtPointer) "two"},
		{"Button 3", DialogSelection2, (XtPointer) "three"},
		{"Button 4", DialogSelection2, (XtPointer) "four"},
		{NULL, NULL, NULL},
	};
	static DialogButtonItem items3[] = {
		{" OK ", DialogSelection3, (XtPointer) "one"},
		{"Cancel", DialogSelection3, (XtPointer) "two"},
		{NULL, NULL, NULL},
	};
	XtAppContext app;

	topLevel = XtAppInitialize(&app, "TestDialog",
		(XrmOptionDescList) NULL, 0,
		&argc, argv, (String *) NULL, (ArgList) NULL, 0);
	i = 0;
	XtSetArg(args[i], XmNorientation, (XtArgVal) XmHORIZONTAL); i++;
	XtSetArg(args[i], XmNentryAlignment,
		(XtArgVal) XmALIGNMENT_CENTER); i++;
	buttonBox = XtCreateManagedWidget("buttonBox",
		xmRowColumnWidgetClass, topLevel, args, i);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Quit",
			XmSTRING_DEFAULT_CHARSET)); i++;
	buttonQuit = XtCreateManagedWidget("buttonQuit",
		xmPushButtonWidgetClass, buttonBox, args, i);
	XtAddCallback(buttonQuit, XmNactivateCallback, Quit, NULL);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Test Dialog #1",
			XmSTRING_DEFAULT_CHARSET)); i++;
	buttonTest1 = XtCreateManagedWidget("buttonTest1",
		xmPushButtonWidgetClass, buttonBox, args, i);
	XtAddCallback(buttonTest1, XmNactivateCallback, TestDialog, NULL);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Test Dialog #2",
			XmSTRING_DEFAULT_CHARSET)); i++;
	buttonTest2 = XtCreateManagedWidget("buttonTest2",
		xmPushButtonWidgetClass, buttonBox, args, i);
	XtAddCallback(buttonTest2, XmNactivateCallback, TestDialog, NULL);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Test Dialog #3",
			XmSTRING_DEFAULT_CHARSET)); i++;
	buttonTest3 = XtCreateManagedWidget("buttonTest3",
		xmPushButtonWidgetClass, buttonBox, args, i);
	XtAddCallback(buttonTest3, XmNactivateCallback, TestDialog, NULL);

/***********************************************************************
	dialog_create(app, parent, instance_name, items, num_columns,
		title, prompt, char_set, dialog_position, modal_dialog,
		max_win_mgr, string_input, auto_popdown, default_button);
***********************************************************************/
	dialog1 = dialog_create(app, topLevel, "dialog1", items1, 30,
		"Dialog Box #1", "This is only a test...",
		XmSTRING_DEFAULT_CHARSET, dialog_CENTER, TRUE, FALSE, FALSE, TRUE, 0);
	dialog2 = dialog_create(app, topLevel, "dialog2", items2, 50,
		"Dialog Box #2", "This is only a test...",
		XmSTRING_DEFAULT_CHARSET, dialog_DEFAULT, FALSE, TRUE, TRUE, FALSE,
		dialog_NO_DEFAULT_BTN);
	dialog_forget_text(dialog2);
	dialog3 = dialog_create(app, topLevel, "dialog3", items3, 0,
		"Dialog Box #3", "This is only a test...",
		XmSTRING_DEFAULT_CHARSET, dialog_POINTER, FALSE, FALSE, FALSE, TRUE, 0);

	XtRealizeWidget(topLevel);
	dialog_realize(dialog1);
	dialog_realize(dialog2);
	dialog_realize(dialog3);
	handle_close_buttons(topLevel);
	XtAppMainLoop(app);
}	/* main */


/*
Quit() terminates the application.
*/
/*ARGSUSED*/
static void Quit(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	int *int_data = (int *) dialog_get_most_recent_button_response(dialog1);
	char *char_data1 = (char *) dialog_get_most_recent_button_response(dialog2);
	char *char_data2 = (char *) dialog_get_most_recent_button_response(dialog3);

	if (int_data)
		printf(
		"The most recently selected button's value for dialog #1 is: %d\n",
		*int_data);
	else
		printf(
		"The most recently selected button's value for dialog #1 is: %s\n",
		"(null)");
	printf("The most recently selected button's value for dialog #2 is: %s\n",
		char_data1 ? char_data1 : "(null)");
	printf("The most recently selected button's value for dialog #3 is: %s\n",
		char_data2 ? char_data2 : "(null)");
	printf("Normal application termination...\n");
	dialog_destroy(dialog1);
	dialog_destroy(dialog2);
	dialog_destroy(dialog3);
	exit(0);
}	/* Quit */


/*
Dismiss() terminates the application.
*/
/*ARGSUSED*/
static void Dismiss(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	dialog_deactivate(dialog2);
}	/* Dismiss */


/*
TestDialog() activates the dialog box.
*/
/*ARGSUSED*/
static void TestDialog(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (w == buttonTest1) {
		int n;

		n = dialog_modal_prompt(dialog1, "Please make a selection:");
		printf("Returning from modal dialog; you selected button: %d\n", n);
	}
	else if (w == buttonTest2)
		dialog_activate(dialog2);
	else
		dialog_activate(dialog3);
}	/* TestDialog */


/*
DialogSelection1() performs an application-specific action
in response to a dialog box selection by the user.
*/
/*ARGSUSED*/
static void DialogSelection1(w, client_data, call_data)
Widget w;
int *client_data;
XtPointer call_data;
{
	printf("You selected the button with value: %d\n", *client_data);
}	/* DialogSelection1 */


/*
DialogSelection2() performs an application-specific action
in response to a dialog box selection by the user.
*/
/*ARGSUSED*/
static void DialogSelection2(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	printf("You selected the button with value: %s\n",
		(char *) client_data);
	printf("The dialog box's current string response data is: %s\n",
		!dialog_get_text(dialog2) ? "(null)" : dialog_get_text(dialog2));
}	/* DialogSelection2 */


/*
DialogSelection3() performs an application-specific action
in response to a dialog box selection by the user.
*/
/*ARGSUSED*/
static void DialogSelection3(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	printf("You selected the button with value: %s\n", (char *) client_data);
}	/* DialogSelection2 */


/*
DialogCancel() performs an application-specific action
in response to a window manager "Close" request.  This
test application simply interprets the "Close" operation
as being equivalent to pressing the leftmost (0th)
mouse button.
*/
/*ARGSUSED*/
static void DialogCancel(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	dialog_cancel((Dialog) client_data, 0);
}	/* DialogCancel */


/*
handle_close_buttons() accommodates the "Close" buttons.  The actions
should be hardcoded -- the user should not be allowed to override the
XmUNMAP default for pop-ups.
*/

static void handle_close_buttons(w)
Widget w;
{
	Atom WM_DELETE_WINDOW;
	Arg arg;

	WM_DELETE_WINDOW = XmInternAtom(XtDisplay(w),
		"WM_DELETE_WINDOW", False);
	XtSetArg(arg, XmNdeleteResponse, (XtArgVal) XmDO_NOTHING);
	XtSetValues(w, &arg, 1);
	XtSetValues(dialog_dialogShell(dialog1), &arg, 1);
	XtSetValues(dialog_dialogShell(dialog2), &arg, 1);
	XtSetValues(dialog_dialogShell(dialog3), &arg, 1);
	XmAddWMProtocolCallback(w, WM_DELETE_WINDOW,
		Quit, (caddr_t) w);
	XmAddWMProtocolCallback(dialog_dialogShell(dialog1),
		WM_DELETE_WINDOW, DialogCancel, (caddr_t) dialog1);
	XmAddWMProtocolCallback(dialog_dialogShell(dialog2),
		WM_DELETE_WINDOW, DialogCancel, (caddr_t) dialog2);
	XmAddWMProtocolCallback(dialog_dialogShell(dialog3),
		WM_DELETE_WINDOW, DialogCancel, (caddr_t) dialog3);
}	/* handle_close_buttons */

