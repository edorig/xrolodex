/**** testedit.c ****/  /**** formatted with 4-column tabs ****/


#include "editor.h"


/*
Functions:
*/

static void Quit();


/*
Private data:
*/

static Editor edit;

static char *startup_text = 
"Line one of the text\n\
Line two of the text\n\
Line three of the text\n\
Line four of the text\n\
Line five of the text\n\
Line six of the text\n\
Line seven of the text\n\
Line eight of the text\n\
one two three four five six\n\
seven eight nine ten eleven twelve";


/*
main() creates a top-level window with an application "Quit"
button and an edit window/object.
*/

void main(argc, argv)
int argc;
char *argv[];
{
	Widget topLevel, pane, buttonBox, buttonQuit;
	Arg args[10];
	int i;
	XtAppContext app;

	topLevel = XtAppInitialize(&app, "TestEdit",
		(XrmOptionDescList) NULL, 0,
		&argc, argv, (String *) NULL, (ArgList) NULL, 0);
	pane = XtCreateManagedWidget("pane",
		xmPanedWindowWidgetClass, topLevel, NULL, 0);
	i = 0;
	XtSetArg(args[i], XmNorientation, (XtArgVal) XmHORIZONTAL); i++;
	XtSetArg(args[i], XmNentryAlignment,
		(XtArgVal) XmALIGNMENT_CENTER); i++;
	buttonBox = XtCreateManagedWidget("buttonBox",
		xmRowColumnWidgetClass, pane, args, i);
	i = 0;
	XtSetArg(args[i], XmNlabelString, XmStringCreateLtoR("Quit",
			XmSTRING_DEFAULT_CHARSET)); i++;
	buttonQuit = XtCreateManagedWidget("buttonQuit",
		xmPushButtonWidgetClass, buttonBox, args, i);
	XtAddCallback(buttonQuit, XmNactivateCallback, Quit, NULL);
	edit = editor_create(pane, "TestEdit", "edit", 10, 40, 40,
/*		editor_SCROLL_LEFT_ONLY, startup_text, XmSTRING_DEFAULT_CHARSET,*/
		editor_SCROLL_DEFAULT, startup_text, XmSTRING_DEFAULT_CHARSET,
/*		"testedit", "Search and Replace", editor_WIN_MGR_DECOR, editor_MENU_BAR);*/
		"testedit", "Search and Replace", editor_WIN_MGR_DECOR, editor_POPUP_MENU);
	XtRealizeWidget(topLevel);
	editor_realize(edit);
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
	editor_destroy(edit);
	exit(0);
}	/* Quit */

