/**** testctrl2.c ****/  /**** formatted with 4-column tabs ****/


#include "ctrlpanel.h"


/*
Functions:
*/

static void Quit(), Print();


/*
Private data:
*/

static Widget topLevel;
static CtrlPanel panel1, panel2;


/*
main() creates a top-level window with a control panel.
*/

void main(argc, argv)
int argc;
char *argv[];
{
	static CtrlPanelItem items1[] = {
		{"Quit", "quitButton", Quit, NULL},
		{"", NULL, NULL, NULL},
		{"Print", "printButton", Print, NULL},
		NULL,
	};
	static CtrlPanelItem items2[] = {
		{"Dismiss", "dismissButton", Quit, NULL},
		{"", NULL, NULL, NULL},
		{"Print", "printButton", Print, NULL},
		NULL,
	};
	XtAppContext app;
	Widget rc;
	Arg args[5];
	int i;

	topLevel = XtAppInitialize(&app, "TestCtrl",
		(XrmOptionDescList) NULL, 0,
		&argc, argv, (String *) NULL, (ArgList) NULL, 0);
	i = 0;
	XtSetArg(args[i], XmNorientation, (XtArgVal) XmVERTICAL); i++;
	rc = XtCreateManagedWidget("rC", xmRowColumnWidgetClass,
		topLevel, args, i);
	panel1 = ctrlPanel_create(rc, "panel1", items1,
		XmSTRING_DEFAULT_CHARSET, ctrlPanel_VERTICAL);
	panel2 = ctrlPanel_create(rc, "panel2", items2,
		XmSTRING_DEFAULT_CHARSET, ctrlPanel_VERTICAL);
	XtRealizeWidget(topLevel);
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
	printf("Your pressed the \"Quit\" button.\n");
	printf("This program is being terminated.\n");
	ctrlPanel_destroy(panel1);
	ctrlPanel_destroy(panel2);
	exit(0);
}	/* Quit */


/*
Print() prints a simple message.
*/
/*ARGSUSED*/
static void Print(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	printf("Your pressed the \"Print\" button.\n");
}	/* Print */

