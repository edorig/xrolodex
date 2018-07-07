/**** help.c ****/  /**** formatted with 4-column tabs ****/

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


/*******************************************************************
Currently, there is no help object; that is, this module simply
provides functions for creating and activating the help system.

This module operates on an array of help items, and each item has
the following type, as defined in `help.h':

typedef struct {
	char *topic;
	char *text;
} HelpItem;

The calling module passing a pointer to this help array via the
routine `help_create_pulldown_menu()' and this function sets the
private global pointer variable `help' in the "Private globals"
section of this module.

*******************************************************************/


#include "help.h"



/*
Private callbacks:
*/

static void HelpDismiss();
static void HelpTopic();


/*
Private globals:
*/

static Widget helpShell, helpList, helpText;
static XmString *help_list;
static HelpItem *help;


/*
Public functions:
*/

/*
help_create_dialog() creates the dialog box used for displaying
the help text.
*/
/*ARGSUSED*/
void help_create_dialog(ref_wid, app_class, rows, columns, char_set)
Widget ref_wid;
char *app_class;
int rows, columns;
XmStringCharSet char_set;
{
	Arg args[20];
	int i, n;
	XmString string;
	Widget helpMainWindow, helpControl, helpPanedWindow,
		helpAction, helpDismissButton;
#ifdef RESTRICT_ACTION_AREA
	Dimension height, margin_height;
#endif

	i = 0;
	XtSetArg(args[i], XmNallowShellResize, (XtArgVal) True); i++;
	XtSetArg(args[i], XmNmappedWhenManaged, (XtArgVal) False); i++;
	helpShell = XtCreatePopupShell("helpShell", topLevelShellWidgetClass,
		ref_wid, args, i);
	i = 0;
	XtSetArg(args[i], XmNsashWidth, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNsashHeight, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNsashIndent, (XtArgVal) -1); i++;
	helpMainWindow = XtCreateManagedWidget("helpMainWindow",
		xmPanedWindowWidgetClass, helpShell, args, i);
	helpControl = XtCreateManagedWidget("helpControl",
		xmFormWidgetClass, helpMainWindow, NULL, 0);
	i = 0;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	helpPanedWindow = XtCreateManagedWidget("helpPanedWindow",
		xmPanedWindowWidgetClass, helpControl, args, i);
	helpList = XmCreateScrolledList(helpPanedWindow, "helpList", NULL, 0);
	for (n = 0; help[n].topic; n++)
		/* count the number of items */;
	help_list = (XmString *) XtMalloc((Cardinal) (sizeof(XmString) * n));
	if (help_list) {
		for (i = 0; i < n; i++)
			help_list[i] = XmStringCreateLtoR(help[i].topic, char_set);
		i = 0;
		XtSetArg(args[i], XmNitems, (XtArgVal) help_list); i++;
		XtSetArg(args[i], XmNitemCount, (XtArgVal) n); i++;
		XtSetValues(helpList, args, i);
	}
	XtManageChild(helpList);
	XtAddCallback(helpList, XmNbrowseSelectionCallback, HelpTopic, NULL);

	i = 0;
	XtSetArg(args[i], XmNeditable, (XtArgVal) False); i++;
	XtSetArg(args[i], XmNeditMode, (XtArgVal) XmMULTI_LINE_EDIT); i++;
	if (rows > 0) {
		XtSetArg(args[i], XmNrows, (XtArgVal) rows); i++;
	}
	if (columns > 0) {
		XtSetArg(args[i], XmNcolumns, (XtArgVal) columns); i++;
	}
	XtSetArg(args[i], XmNvalue, (XtArgVal) " ");
	XtSetArg(args[i], XmNautoShowCursorPosition, (XtArgVal) False);
	helpText = XmCreateScrolledText(helpPanedWindow, "helpText", args, i);
	XtManageChild(helpText);
	i = 0;
	XtSetArg(args[i], XmNadjustLast, (XtArgVal) False); i++;
	XtSetArg(args[i], XmNfractionBase, 7); i++;
	helpAction = XtCreateManagedWidget("helpAction",
		xmFormWidgetClass, helpMainWindow, args, i);
	string = XmStringCreateLtoR("Dismiss", char_set);
	i = 0;
	XtSetArg(args[i], XmNlabelString, string); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNleftPosition, (XtArgVal) 3); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNrightPosition, (XtArgVal) 4); i++;
	XtSetArg(args[i], XmNshowAsDefault, (XtArgVal) True); i++;
	helpDismissButton = XtCreateManagedWidget("helpDismissButton",
		xmPushButtonWidgetClass, helpAction, args, i);
	XmStringFree(string);
	XtAddCallback(helpDismissButton, XmNactivateCallback, HelpDismiss, NULL);
	/*
	restrict the height of the action area:
	*/
#ifdef RESTRICT_ACTION_AREA
	i = 0;
	XtSetArg(args[i], XmNmarginHeight, &margin_height); i++;
	XtGetValues(helpAction, args, i);
	i = 0;
	XtSetArg(args[i], XmNheight, &height); i++;
	XtGetValues(helpDismissButton, args, i);
	i = 0;
	XtSetArg(args[i], XmNpaneMinimum,
		(XtArgVal) (height + (margin_height * 2))); i++;
	XtSetArg(args[i], XmNpaneMaximum,
		(XtArgVal) (height + (margin_height * 2))); i++;
#else
	i = 0;
	XtSetArg(args[i], XmNskipAdjust, (XtArgVal) True); i++;
#endif
	XtSetValues(helpAction, args, i);
	remove_sash_traversal(helpMainWindow);
	remove_sash_traversal(helpPanedWindow);
}	/* help_create_dialog */


/*
help_realize() does the realization.  At present,
there is no argument, since there is no "Help" object.
*/

void help_realize()
{
	XtRealizeWidget(helpShell);
}	/* help_realize */


/*
help_shell() returns the shell widget
for the help dialog.
*/

Widget help_shell()
{
	return helpShell;
}	/* help_shell */


/*
help_create_pulldown_menu() adds the "Help" pull-down
menu to the end of the menu bar.
*/

void help_create_pulldown_menu(mainMenuBar, help_menu, help_data, char_set)
Widget mainMenuBar;
menu_entry help_menu[];
HelpItem *help_data;
XmStringCharSet char_set;
{
	Arg args[5];
	int i;
	XmString string;
	Widget menuHelpPane, menuHelpButton;

	help = help_data;
	menuHelpPane = XmCreatePulldownMenu(mainMenuBar, "menuHelpPane", NULL, 0);
	i = 0;
	XtSetArg(args[i], XmNsubMenuId, (XtArgVal) menuHelpPane); i++;
	string = XmStringCreateLtoR("Help", char_set);
	XtSetArg(args[i], XmNlabelString, string); i++;
	menuHelpButton = XtCreateManagedWidget("menuHelpButton",
		xmCascadeButtonWidgetClass, mainMenuBar, args, i);
	XmStringFree(string);
	i = 0;
	XtSetArg(args[i], XmNmenuHelpWidget, menuHelpButton); i++;
	XtSetValues(mainMenuBar, args, i);
	create_menus(menuHelpPane, help_menu, char_set);
}	/* help_create_pulldown_menu */


/*
Public callbacks:
*/

/*
Help() is a dispatcher for the help text.
*/
/*ARGSUSED*/
void Help(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	static int first_time = True;

	if (!XtIsRealized(helpShell))
		return;
	if (first_time) {
		first_time = False;
		XmListSelectPos(helpList, 1, True);
/*		XmTextSetString(helpText, help[0].text);*/
	}
	XtMapWidget(helpShell);
}	/* Help */


/*
Private callbacks:
*/

/*
HelpTopic() displays the appropriate help topic.
*/
/*ARGSUSED*/
static void HelpTopic(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmListCallbackStruct *call_data;
{
	XmTextSetString(helpText, help[(int) (call_data->item_position - 1)].text);
}	/* HelpTopic */


/*
HelpDismiss() simply removes the help window.
*/
/*ARGSUSED*/
static void HelpDismiss(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtUnmapWidget(helpShell);
}	/* HelpDismiss */

