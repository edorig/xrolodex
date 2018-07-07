/**** testlist.c ****/  /**** formatted with 4-column tabs ****/


#include "listshell.h"


/*
Functions:
*/

static void Quit(), TestList(), ListSelection();


/*
Private data:
*/

static Widget topLevel, buttonBox, buttonTest1, buttonTest2,
	buttonQuit;

static ListShell list1, list2;


/*
main() creates a top-level window with three buttons, two
of which activate other top-level windows.
*/

void main(argc, argv)
int argc;
char *argv[];
{
	Arg args[10];
	int i;
	static char *months[] = {
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December",
		NULL
	};
	XtAppContext app;

	topLevel = XtAppInitialize(&app, "TestList",
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
		XmStringCreateLtoR("Test Viewport #1",
			XmSTRING_DEFAULT_CHARSET)); i++;
	buttonTest1 = XtCreateManagedWidget("buttonTest1",
		xmPushButtonWidgetClass, buttonBox, args, i);
	XtAddCallback(buttonTest1, XmNactivateCallback, TestList, NULL);
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Test Viewport #2",
			XmSTRING_DEFAULT_CHARSET)); i++;
	buttonTest2 = XtCreateManagedWidget("buttonTest2",
		xmPushButtonWidgetClass, buttonBox, args, i);
	XtAddCallback(buttonTest2, XmNactivateCallback, TestList, NULL);
	list1 = listShell_create(topLevel, "TestList", "listShell1", months,
		XmSTRING_DEFAULT_CHARSET, 0, 12, listShell_STAY_UP,
		listShell_WIN_MGR_DECOR);
	listShell_add_callback(list1, ListSelection, 1);
	list2 = listShell_create(topLevel, "TestList", "listShell2", months,
		XmSTRING_DEFAULT_CHARSET, 0, 6, listShell_NO_STAY_UP,
		listShell_MIN_WIN_MGR_DECOR);
	listShell_add_callback(list2, ListSelection, 2);
	XtRealizeWidget(topLevel);
	listShell_realize(list1);
	listShell_realize(list2);
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
	listShell_destroy(list1);
	listShell_destroy(list2);
	exit(0);
}	/* Quit */


/*
TestList() activates a `ListShell' object.
*/
/*ARGSUSED*/
static void TestList(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (w == buttonTest1)
		listShell_activate(list1);
	else if (w == buttonTest2)
		listShell_activate(list2);
}	/* TestList */


/*
ListSelection() performs an application-specific action
in response to a list selection by the user.
*/
/*ARGSUSED*/
static void ListSelection(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmListCallbackStruct *call_data;
{
	int list_no = (int) client_data;
	char *entry;

	XmStringGetLtoR(call_data->item, XmSTRING_DEFAULT_CHARSET,
		&entry);
	printf("You selected entry \"%s\" for list #%d\n",
		entry,
		list_no);
}	/* ListSelection */

