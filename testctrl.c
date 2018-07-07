/**** testctrl.c ****/  /**** formatted with 4-column tabs ****/


#include "ctrlpanel.h"


/*
Functions:
*/

static void Quit(), Print();


/*
Private data:
*/

static Widget topLevel;
static CtrlPanel panel;


/*
main() creates a top-level window with a control panel.
*/

void main(argc, argv)
int argc;
char *argv[];
{
	static CtrlPanelItem items[] = {
		{"Quit", "quitButton", Quit, NULL},
		{"", NULL, NULL, NULL},
		{"Print", "printButton", Print, NULL},
		NULL,
	};
	XtAppContext app;

	topLevel = XtAppInitialize(&app, "TestCtrl",
		(XrmOptionDescList) NULL, 0,
		&argc, argv, (String *) NULL, (ArgList) NULL, 0);
/*	panel = ctrlPanel_create(topLevel, "panel", items,
		XmSTRING_DEFAULT_CHARSET, ctrlPanel_HORIZONTAL);*/
	panel = ctrlPanel_create(topLevel, NULL, items,
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
	ctrlPanel_destroy(panel);
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

