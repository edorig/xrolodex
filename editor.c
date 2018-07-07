/**** editor.c ****/  /**** formatted with 4-column tabs ****/

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
An `Editor' object is a Motif XmText widget attached to a menubar.
The menubar provides cut, copy, and paste functions in the edit
menu and find-selection and search and replace functions in the
find menu.  The search and replace function pops up a top-level
window with buttons and text entry areas for find and replace
operations.  Thus, the application must call the editor's realize
function to realize the search and replace window hierarchy.

Sample usage:

#include "editor.h"
...
...
void main(argc, argv)
int argc;
char *argv[];
{
	XtAppContext app;
	Widget topLevel, pane, ...;
	Editor edit;
	char *startup_text = "Start-up text";

	topLevel = XtAppInitialize(&app, "ApplicationClass",
		(XrmOptionDescList) NULL, 0,
		&argc, argv, (String *) NULL, (ArgList) NULL, 0);
	pane = XtCreateManagedWidget("pane",
		xmPanedWindowWidgetClass, topLevel, NULL, 0);
	...
	...
	edit = editor_create(pane, "ApplicationClass", "edit", 10, 40, 40,
		editor_SCROLL_DEFAULT, startup_text, XmSTRING_DEFAULT_CHARSET,
		"application", "label", editor_WIN_MGR_DECOR, editor_POPUP_MENU);
	XtRealizeWidget(topLevel);
	editor_realize(edit);	// realize the search and replace pop-up //
	...
	...
**************************************************************************/


#include "editor.h"

/*
Private support functions:
*/

static XmTextPosition get_text_cursor_position();
#ifdef MANUAL_CLIP
static int selection_to_clipboard();
static char *retrieve_from_clipboard();
#endif


/*
Private callback functions:
*/

static void CutText();
static void CopyText();
static void PasteText();
static void ClearText();
static void FindSelection();
static void ReplaceText();
static void ReplaceDismiss();
static void ReplaceFind();
static void ReplaceReplace();


/*
Private event handlers:
*/

static void PostMenu();


/*
Menu data -- the callback data is dynamic, specifically, the editor
object instance, so specify null callbacks here and use XtAddCallback()
below.
*/

static menu_entry edit_menu[] = {
	{menu_ENTRY, "Cut", "menuCutButton", NULL, NULL, NULL, NULL},
	{menu_ENTRY, "Copy", "menuCopyButton", NULL, NULL, NULL, NULL},
	{menu_ENTRY, "Paste", "menuPasteButton", NULL, NULL, NULL, NULL},
	{menu_SEPARATOR, NULL, NULL, NULL, NULL, NULL, NULL},
	{menu_ENTRY, "Clear", "menuClearButton", NULL, NULL, NULL, NULL},
	{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
};

static menu_entry find_menu[] = {
	{menu_ENTRY, "Find Selection", "menuSearchButton", NULL, NULL,
		NULL, NULL},
	{menu_ENTRY, "Search and Replace...", "menuReplaceButton", NULL, NULL,
		NULL, NULL},
	{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
};

static menu_entry menus[] = {
	{menu_SUBMENU, "Edit", "menuEditButton", NULL, NULL, edit_menu, NULL},
	{menu_SUBMENU, "Find", "menuFindButton", NULL, NULL, find_menu, NULL},
	{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
};



/*
Public functions:
*/

/*
editor_create() creates an editor object with a multi-line
edit window and a menubar that includes "Edit" and "Find"
menus.  The application can set the number of rows and
columns for the edit window.
*/
/*ARGSUSED*/
Editor editor_create(parent, app_class, instance_name, num_rows, num_columns,
	replace_columns, special_scrolling, startup_text, char_set, app_name,
	label, win_mgr_behavior, menu_behavior)
Widget parent;
char *app_class;
char *instance_name;
int num_rows, num_columns, replace_columns, special_scrolling;
char *startup_text;
XmStringCharSet char_set;
char *app_name, *label;
int win_mgr_behavior, menu_behavior;
{
	Arg args[20];
	int i;
	Editor eObject;
#ifdef RESTRICT_ACTION_AREA
	Dimension height, margin_height;
#endif

	if (!(eObject = (Editor) XtMalloc((Cardinal) sizeof(_Editor))))
		return 0;
	eObject->self = eObject;
	eObject->app_name = app_name;
	eObject->menuBar = eObject->popupMenu = NULL;
	/*
	instances have precedence over classes:
	*/
	eObject->instance = XtCreateManagedWidget(instance_name,
		xmFrameWidgetClass, parent, NULL, 0);
	eObject->class = XtCreateManagedWidget("Editor",
		xmFrameWidgetClass, eObject->instance, NULL, 0);
	eObject->editBox = XtCreateManagedWidget("editBox",
		xmFormWidgetClass, eObject->class, NULL, 0);
	if (menu_behavior == editor_MENU_BAR) {
		i = 0;
		XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
		XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
		XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
		eObject->menuBar = XmCreateMenuBar(eObject->editBox,
			"menuBar", args, i);
		XtManageChild(eObject->menuBar);
		create_menus(eObject->menuBar, menus, char_set);
	}
	else {
		eObject->popupMenu = XmCreatePopupMenu(eObject->editBox,
			"popupMenu", NULL, 0);		
		create_menus(eObject->popupMenu, menus, char_set);
		XtAddEventHandler(eObject->editBox, ButtonPressMask, FALSE,
			PostMenu, eObject->popupMenu);
	}
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	if (menu_behavior == editor_MENU_BAR) {
		XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
		XtSetArg(args[i], XmNtopWidget, (XtArgVal) eObject->menuBar); i++;
	}
	else {
		XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	}
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	eObject->editWindow = XtCreateManagedWidget("editWindow",
		xmScrolledWindowWidgetClass, eObject->editBox, args, i);
	i = 0;
	XtSetArg(args[i], XmNeditMode, (XtArgVal) XmMULTI_LINE_EDIT); i++;
	XtSetArg(args[i], XmNautoShowCursorPosition, (XtArgVal) True); i++;
	XtSetArg(args[i], XmNeditable, (XtArgVal) True); i++;
	if (special_scrolling == editor_SCROLL_LEFT_ONLY) {
		XtSetArg(args[i], XmNscrollHorizontal, (XtArgVal) False); i++;
		XtSetArg(args[i], XmNscrollVertical, (XtArgVal) True); i++;
		XtSetArg(args[i], XmNscrollLeftSide, (XtArgVal) True); i++;
	}
	else if (special_scrolling == editor_SCROLL_RIGHT_ONLY) {
		XtSetArg(args[i], XmNscrollHorizontal, (XtArgVal) False); i++;
		XtSetArg(args[i], XmNscrollVertical, (XtArgVal) True); i++;
		XtSetArg(args[i], XmNscrollLeftSide, (XtArgVal) False); i++;
	}
	if (num_rows > 0) {
		XtSetArg(args[i], XmNrows, (XtArgVal) num_rows); i++;
	}
	if (num_columns > 0) {
		XtSetArg(args[i], XmNcolumns, (XtArgVal) num_columns); i++;
	}
	if (startup_text) {
		XtSetArg(args[i], XmNvalue, (XtArgVal) startup_text); i++;
	}
	eObject->edit = XtCreateManagedWidget("edit",
		xmTextWidgetClass, eObject->editWindow, args, i);

	/*
	add callbacks and data for buttons using returned widget IDs:
	*/
	XtAddCallback(edit_menu[0].id, XmNactivateCallback,
		CutText, (XtPointer) eObject);
	XtAddCallback(edit_menu[1].id, XmNactivateCallback,
		CopyText, (XtPointer) eObject);
	XtAddCallback(edit_menu[2].id, XmNactivateCallback,
		PasteText, (XtPointer) eObject);
	XtAddCallback(edit_menu[4].id, XmNactivateCallback,
		ClearText, (XtPointer) eObject);
	XtAddCallback(find_menu[0].id, XmNactivateCallback,
		FindSelection, (XtPointer) eObject);
	XtAddCallback(find_menu[1].id, XmNactivateCallback,
		ReplaceText, (XtPointer) eObject);

	/*
	create the search and replace window:
	*/
	i = 0;
	XtSetArg(args[i], XmNallowShellResize, (XtArgVal) True); i++;
	XtSetArg(args[i], XmNmappedWhenManaged, (XtArgVal) False); i++;
	if (win_mgr_behavior == editor_WIN_MGR_DECOR)
		eObject->replaceShell = XtCreatePopupShell("replaceShell",
			topLevelShellWidgetClass, parent, args, i);
	else
		eObject->replaceShell = XtCreatePopupShell("replaceShell",
			transientShellWidgetClass, parent, args, i);
	eObject->replaceInstance = XtCreateManagedWidget(instance_name,
		xmFrameWidgetClass, eObject->replaceShell, NULL, 0);
	eObject->replaceClass = XtCreateManagedWidget("Editor",
		xmFrameWidgetClass, eObject->replaceInstance, NULL, 0);
	i = 0;
	XtSetArg(args[i], XmNsashWidth, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNsashHeight, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNsashIndent, (XtArgVal) -1); i++;
	eObject->replacePane = XtCreateManagedWidget("replacePane",
		xmPanedWindowWidgetClass, eObject->replaceClass, args, i);
	eObject->replaceControl = XtCreateManagedWidget("replaceControl",
		xmFormWidgetClass, eObject->replacePane, NULL, 0);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR(label, char_set)); i++;
	eObject->replaceLabel = XtCreateManagedWidget("replaceLabel",
		xmLabelWidgetClass, eObject->replaceControl, args, i);
	i = 0;
	XtSetArg(args[i], XmNalignment, (XtArgVal) XmALIGNMENT_CENTER); i++;
	XtSetValues(eObject->replaceLabel, args, i);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNtopWidget, (XtArgVal) eObject->replaceLabel); i++;
	XtSetArg(args[i], XmNorientation, (XtArgVal) XmHORIZONTAL); i++;
	eObject->replaceFindBox = XtCreateManagedWidget("replaceFindBox",
		xmFormWidgetClass, eObject->replaceControl, args, i);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNtopWidget, (XtArgVal) eObject->replaceFindBox); i++;
	XtSetArg(args[i], XmNorientation, (XtArgVal) XmHORIZONTAL); i++;
	eObject->replaceReplaceBox = XtCreateManagedWidget("replaceReplaceBox",
		xmFormWidgetClass, eObject->replaceControl, args, i);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("   Find: ", char_set)); i++;
	eObject->replaceFindLabel = XtCreateManagedWidget("replaceFindLabel",
		xmLabelWidgetClass, eObject->replaceFindBox, args, i);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Replace: ", char_set)); i++;
	eObject->replaceReplaceLabel = XtCreateManagedWidget("replaceReplaceLabel",
		xmLabelWidgetClass, eObject->replaceReplaceBox, args, i);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNleftWidget, (XtArgVal) eObject->replaceFindLabel); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	if (replace_columns > 0) {
		XtSetArg(args[i], XmNcolumns, (XtArgVal) replace_columns); i++;
	}
	eObject->replaceFindText = XtCreateManagedWidget("replaceFindText",
		xmTextWidgetClass, eObject->replaceFindBox, args, i);
	XtAddCallback(eObject->replaceFindText,
		XmNactivateCallback, ReplaceFind, (XtPointer) eObject);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNleftWidget, (XtArgVal) eObject->replaceReplaceLabel); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNcolumns, (XtArgVal) 40); i++;
	eObject->replaceReplaceText = XtCreateManagedWidget("replaceReplaceText",
		xmTextWidgetClass, eObject->replaceReplaceBox, args, i);
	i = 0;
	XtSetArg(args[i], XmNadjustLast, (XtArgVal) False); i++;
	XtSetArg(args[i], XmNfractionBase, 7); i++;
	eObject->replaceAction = XtCreateManagedWidget("replaceAction",
		xmFormWidgetClass, eObject->replacePane, args, i);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Find", char_set)); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNleftPosition, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNrightPosition, (XtArgVal) 2); i++;
	eObject->replaceFindButton = XtCreateManagedWidget("replaceFindButton",
		xmPushButtonWidgetClass, eObject->replaceAction, args, i);
	XtAddCallback(eObject->replaceFindButton,
		XmNactivateCallback, ReplaceFind, (XtPointer) eObject);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Replace", char_set)); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNleftPosition, (XtArgVal) 3); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNrightPosition, (XtArgVal) 4); i++;
	eObject->replaceReplaceButton = XtCreateManagedWidget("replaceReplaceButton",
		xmPushButtonWidgetClass, eObject->replaceAction, args, i);
	XtAddCallback(eObject->replaceReplaceButton,
		XmNactivateCallback, ReplaceReplace, (XtPointer) eObject);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Dismiss", char_set)); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNleftPosition, (XtArgVal) 5); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNrightPosition, (XtArgVal) 6); i++;
	eObject->replaceDismissButton = XtCreateManagedWidget("replaceDismissButton",
		xmPushButtonWidgetClass, eObject->replaceAction, args, i);
	XtAddCallback(eObject->replaceDismissButton,
		XmNactivateCallback, ReplaceDismiss, (XtPointer) eObject);
	/*
	restrict the height of the action area:
	*/
#ifdef RESTRICT_ACTION_AREA
	i = 0;
	XtSetArg(args[i], XmNmarginHeight, &margin_height); i++;
	XtGetValues(eObject->replaceAction, args, i);
	i = 0;
	XtSetArg(args[i], XmNheight, &height); i++;
	XtGetValues(eObject->replaceFindButton, args, i);
	i = 0;
	XtSetArg(args[i], XmNpaneMinimum,
		(XtArgVal) (height + (margin_height * 2))); i++;
	XtSetArg(args[i], XmNpaneMaximum,
		(XtArgVal) (height + (margin_height * 2))); i++;
#else
	i = 0;
	XtSetArg(args[i], XmNskipAdjust, (XtArgVal) True); i++;
#endif
	XtSetValues(eObject->replaceAction, args, i);
	remove_sash_traversal(eObject->replacePane);
	return eObject;
}	/* editor_create */


/*
editor_destroy() frees the storage for an `Editor' object.
*/

void editor_destroy(eObject)
Editor eObject;
{
	XtDestroyWidget(eObject->replaceShell);
	XtDestroyWidget(eObject->instance);
	XtFree((char *) eObject->self);
}	/* editor_destroy */


/*
editor_realize() realizes the editor object, specifically, the
top-level shell window used by the search and replace window.
*/

void editor_realize(eObject)
Editor eObject;
{
	XtRealizeWidget(eObject->replaceShell);
}	/* editor_realize */


/*
editor_set_sensitive() allows the application to set the
sensitivity of the XmText widget to True.
*/

void editor_set_sensitive(eObject)
Editor eObject;
{
	XtSetSensitive(eObject->edit, True);
}	/* editor_set_sensitive */


/*
editor_set_nonsensitive() allows the application to set the
sensitivity of the XmText widget to False.
*/

void editor_set_nonsensitive(eObject)
Editor eObject;
{
	XtSetSensitive(eObject->edit, False);
}	/* editor_set_nonsensitive */


/*
editor_get_text() returns the text for the text edit widget.
The caller must free the text with XtFree().
*/

char *editor_get_text(eObject)
Editor eObject;
{
	return (char *) XmTextGetString(eObject->edit);
}	/* editor_get_text */


/*
editor_set_text() sets the text for the text edit widget.
*/

void editor_set_text(eObject, text)
Editor eObject;
char *text;
{
	XmTextSetString(eObject->edit, text);
}	/* editor_set_text */


/*
For the following functions editor_<widget_name>() returns
the widget IDs of the internal widgets.  Many of these
aren't really needed and could be deleted.  Using macros is
another possibility.  And, of course, Xt provides functions
for obtaining this information.
*/

Widget editor_instance(eObject)
Editor eObject;
{
	return eObject->instance;
}	/* editor_instance */


Widget editor_class(eObject)
Editor eObject;
{
	return eObject->class;
}	/* editor_class */


Widget editor_editBox(eObject)
Editor eObject;
{
	return eObject->editBox;
}	/* editor_editBox */


Widget editor_menuBar(eObject)
Editor eObject;
{
	return eObject->menuBar;
}	/* editor_menuBar */


/*ARGSUSED*/
Widget editor_menuEditButton(eObject)		/* maintain consistent interface */
Editor eObject;
{
	return menus[0].id;
}	/* editor_menuEditButton */


/*ARGSUSED*/
Widget editor_menuFindButton(eObject)		/* maintain consistent interface */
Editor eObject;
{
	return menus[1].id;
}	/* editor_menuFindButton */


/*ARGSUSED*/
Widget editor_menuCutButton(eObject)		/* maintain consistent interface */
Editor eObject;
{
	return edit_menu[0].id;
}	/* editor_menuCutButton */


/*ARGSUSED*/
Widget editor_menuCopyButton(eObject)		/* maintain consistent interface */
Editor eObject;
{
	return edit_menu[1].id;
}	/* editor_menuCopyButton */


/*ARGSUSED*/
Widget editor_menuPasteButton(eObject)		/* maintain consistent interface */
Editor eObject;
{
	return edit_menu[2].id;
}	/* editor_menuPasteButton */


/*ARGSUSED*/
Widget editor_menuClearButton(eObject)		/* maintain consistent interface */
Editor eObject;
{
	return edit_menu[2].id;
}	/* editor_menuClearButton */


/*ARGSUSED*/
Widget editor_menuSearchButton(eObject)		/* maintain consistent interface */
Editor eObject;
{
	return find_menu[0].id;
}	/* editor_menuSearchButton */


/*ARGSUSED*/
Widget editor_menuReplaceButton(eObject)	/* maintain consistent interface */
Editor eObject;
{
	return find_menu[1].id;
}	/* editor_menuReplaceButton */


Widget editor_editWindow(eObject)
Editor eObject;
{
	return eObject->editWindow;
}	/* editor_editWindow */


Widget editor_edit(eObject)
Editor eObject;
{
	return eObject->edit;
}	/* editor_edit */


Widget editor_replaceShell(eObject)
Editor eObject;
{
	return eObject->replaceShell;
}	/* editor_replaceShell */


Widget editor_replaceInstance(eObject)
Editor eObject;
{
	return eObject->replaceInstance;
}	/* editor_replaceInstance */


Widget editor_replaceClass(eObject)
Editor eObject;
{
	return eObject->replaceClass;
}	/* editor_replaceClass */


Widget editor_replacePane(eObject)
Editor eObject;
{
	return eObject->replacePane;
}	/* editor_replacePane */


Widget editor_replaceControl(eObject)
Editor eObject;
{
	return eObject->replaceControl;
}	/* editor_replaceControl */


Widget editor_replaceLabel(eObject)
Editor eObject;
{
	return eObject->replaceLabel;
}	/* editor_replaceLabel */


Widget editor_replaceFindBox(eObject)
Editor eObject;
{
	return eObject->replaceFindBox;
}	/* editor_replaceFindBox */


Widget editor_replaceReplaceBox(eObject)
Editor eObject;
{
	return eObject->replaceReplaceBox;
}	/* editor_replaceReplaceBox */


Widget editor_replaceFindLabel(eObject)
Editor eObject;
{
	return eObject->replaceFindLabel;
}	/* editor_replaceFindLabel */


Widget editor_replaceReplaceLabel(eObject)
Editor eObject;
{
	return eObject->replaceReplaceLabel;
}	/* editor_replaceReplaceLabel */


Widget editor_replaceFindText(eObject)
Editor eObject;
{
	return eObject->replaceFindText;
}	/* editor_replaceFindText */


Widget editor_replaceReplaceText(eObject)
Editor eObject;
{
	return eObject->replaceReplaceText;
}	/* editor_replaceReplaceText */


Widget editor_replaceAction(eObject)
Editor eObject;
{
	return eObject->replaceAction;
}	/* editor_replaceAction */


Widget editor_replaceFindButton(eObject)
Editor eObject;
{
	return eObject->replaceFindButton;
}	/* editor_replaceFindButton */


Widget editor_replaceReplaceButton(eObject)
Editor eObject;
{
	return eObject->replaceReplaceButton;
}	/* editor_replaceReplaceButton */


Widget editor_replaceDismissButton(eObject)
Editor eObject;
{
	return eObject->replaceDismissButton;
}	/* editor_replaceDismissButton */


/*
Private callback functions:
*/

/*
CutText() is a private callback for the "Cut" function.
*/
/*ARGSUSED*/
static void CutText(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmAnyCallbackStruct *call_data;
{
	Editor eObject = (Editor) client_data;

#ifdef MANUAL_CLIP
	if (selection_to_clipboard(eObject,
			call_data->event->xbutton.time))
		XmTextRemove(eObject->edit);
#else
/*********************************************************
This function still has errors in certain Motif 1.1's.
*********************************************************/
	XmTextCut(eObject->edit, call_data->event->xbutton.time);
#endif /* MANUAL_CLIP */
}	/* CutText */


/*
CopyText() is a private callback for the "Copy" function.
*/
/*ARGSUSED*/
static void CopyText(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmAnyCallbackStruct *call_data;
{
	Editor eObject = (Editor) client_data;

#ifdef MANUAL_CLIP
	if (!selection_to_clipboard(eObject, call_data->event->xbutton.time))
		;										/* silent, at present */
#else
/*********************************************************
This function still has errors in certain Motif 1.1's.
*********************************************************/
	XmTextCopy(eObject->edit, call_data->event->xbutton.time);
#endif /* MANUAL_CLIP */
}	/* CopyText */


/*
PasteText() is a private callback for the "Paste" function.
*/
/*ARGSUSED*/
static void PasteText(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmAnyCallbackStruct *call_data;
{
	Editor eObject = (Editor) client_data;

#ifdef MANUAL_CLIP
	char *buffer;

	if ((buffer = retrieve_from_clipboard(eObject)) != NULL) {
		XmTextPosition cur_pos = get_text_cursor_position(eObject);
		XmTextReplace(eObject->edit, cur_pos, cur_pos, buffer);
		XtFree(buffer);
	}
#else
/*********************************************************
This function still has errors in certain Motif 1.1's.
*********************************************************/
	XmTextPaste(eObject->edit);
#endif /* MANUAL_CLIP */
}	/* PasteText */


/*
ClearText() is a private callback for the "Clear" function.
It clears the clipboard, not the primary selection.
*/
/*ARGSUSED*/
static void ClearText(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmAnyCallbackStruct *call_data;
{
	Editor eObject = (Editor) client_data;
	long item_id;
	int data_id;
	char *buffer = "";	/* null string */
	XmString clip_label = XmStringCreateSimple(eObject->app_name);

	while (XmClipboardStartCopy(XtDisplay(eObject->edit),
			XtWindow(eObject->edit), clip_label,
			call_data->event->xbutton.time,
			eObject->edit, NULL, &item_id) != ClipboardSuccess)
		;
	while (XmClipboardCopy(XtDisplay(eObject->edit),
			XtWindow(eObject->edit), item_id, "STRING", buffer,
			(unsigned long) (strlen(buffer)),
			0, &data_id) != ClipboardSuccess)
		;
	while (XmClipboardEndCopy(XtDisplay(eObject->edit),
			XtWindow(eObject->edit), item_id) != ClipboardSuccess)
		;
	XmStringFree(clip_label);
/************************************************************************
Clearing the selection, not the clipboard would be another possibility:
	XmTextClearSelection(eObject->edit, call_data->event->xbutton.time);
************************************************************************/
}	/* ClearText */


/*
FindSelection() is a private callback for the
"Find Selection" operation.
*/
/*ARGSUSED*/
static void FindSelection(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmAnyCallbackStruct *call_data;
{
	Editor eObject = (Editor) client_data;
	char *buffer, *selection;

	if ((selection = XmTextGetSelection(eObject->edit)) == NULL)
		return;
	if ((buffer = XmTextGetString(eObject->edit)) != NULL) {
		XmTextPosition cur_pos = get_text_cursor_position(eObject);
		Arg arg;

		cur_pos++;	/* don't want to find the same text if the		*/
					/* cursor is at the beginning of the selection	*/
		cur_pos = (XmTextPosition) string_search(buffer,
			selection, (int) cur_pos);
		if (cur_pos != string_NO_MATCH) {
			XmTextSetSelection(eObject->edit, cur_pos,
				cur_pos + (XmTextPosition) strlen(selection),
				call_data->event->xbutton.time);
			XtSetArg(arg, XmNcursorPosition, cur_pos);
			XtSetValues(eObject->edit, &arg, 1);
		}
		XtFree(buffer);
	}
	XtFree(selection);
}	/* FindSelection */


/*
ReplaceText() is a private callback that maps
the search and replace window.
*/
/*ARGSUSED*/
static void ReplaceText(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmAnyCallbackStruct *call_data;
{
	Editor eObject = (Editor) client_data;

	if (!XtIsRealized(eObject->replaceShell))
		return;
	XtMapWidget(eObject->replaceShell);
}	/* ReplaceText */


/*
ReplaceDismiss() is a private callback that removes
the search and replace window.
*/
/*ARGSUSED*/
static void ReplaceDismiss(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmAnyCallbackStruct *call_data;
{
	Editor eObject = (Editor) client_data;

	XtUnmapWidget(eObject->replaceShell);
}	/* ReplaceDismiss */


/*
ReplaceFind() is a private callback for the "Find" button in the search
and replace button that finds the next occurrence of the text.
*/
/*ARGSUSED*/
static void ReplaceFind(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmAnyCallbackStruct *call_data;
{
	Editor eObject = (Editor) client_data;
	char *buffer, *search_text;

	if ((search_text = XmTextGetString(eObject->replaceFindText)) == NULL)
		return;
	if ((buffer = XmTextGetString(eObject->edit)) != NULL) {
		XmTextPosition cur_pos = get_text_cursor_position(eObject);
		Arg arg;

		cur_pos++; /* don't find the "current" text again */
		cur_pos = (XmTextPosition) string_search(buffer,
			search_text, (int) cur_pos);
		if (cur_pos != (XmTextPosition) string_NO_MATCH) {
			XmTextSetSelection(eObject->edit, cur_pos,
				cur_pos + (XmTextPosition) strlen(search_text),
				call_data->event->xbutton.time);
			XtSetArg(arg, XmNcursorPosition, cur_pos);
			XtSetValues(eObject->edit, &arg, 1);
		}
		XtFree(buffer);
	}
	XtFree(search_text);
}	/* ReplaceFind */


/*
ReplaceReplace() is a private callback that replaces the
most recently "found" text.
*/
/*ARGSUSED*/
static void ReplaceReplace(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmAnyCallbackStruct *call_data;
{
	Editor eObject = (Editor) client_data;
	char *replace_text;
	XmTextPosition cur_pos;

	if (!XmTextRemove(eObject->edit))
		return;				/* policy:  return silently */
	if ((replace_text = XmTextGetString(eObject->replaceReplaceText)) == NULL)
		replace_text = "";	/* don't trust Motif to return "" */
	cur_pos = get_text_cursor_position(eObject);
	XmTextReplace(eObject->edit, cur_pos, cur_pos, replace_text);
	XtFree(replace_text);
}	/* ReplaceFind */


/*
Private event handlers:
*/

/*
PostMenu() invokes the pop-up menu.
*/
/*ARGSUSED*/
static void PostMenu(w, menu, event)
Widget w;
Widget menu;
XEvent *event;
{
	if (event->type != ButtonPress)		/* paranoia */
		return;
	XmMenuPosition(menu, (XButtonPressedEvent *) event);
	XtManageChild(menu);
}	/* PostMenu */


/*
Private support functions:
*/

/*
get_text_cursor_position() returns the current text position.
*/

static XmTextPosition get_text_cursor_position(eObject)
Editor eObject;
{
	XmTextPosition cur_pos;
	Arg arg;

	XtSetArg(arg, XmNcursorPosition, &cur_pos);
	XtGetValues(eObject->edit, &arg, 1);
	return cur_pos;
}	/* get_text_cursor_position */


#ifdef MANUAL_CLIP
/*
selection_to_clipboard() copies the current selection to
the clipboard.
*/

static int selection_to_clipboard(eObject, time_stamp)
Editor eObject;
Time time_stamp;
{
	char *selection;
	long item_id;
	int data_id;
	Display *dpy = XtDisplay(eObject->edit);
	Window win = XtWindow(eObject->edit);
	XmString clip_label = XmStringCreateSimple(eObject->app_name);

	if ((selection = XmTextGetSelection(eObject->edit)) == NULL)
		return False;
	while (XmClipboardStartCopy(dpy, win, clip_label, time_stamp,
			eObject->edit, NULL, &item_id) != ClipboardSuccess)
		;
	while (XmClipboardCopy(dpy, win, item_id, "STRING", selection,
			(unsigned long) strlen(selection),
			0, &data_id) != ClipboardSuccess)
		;
	while (XmClipboardEndCopy(dpy, win, item_id) != ClipboardSuccess)
		;
	XmStringFree(clip_label);
	return True;
}	/* selection_to_clipboard */


/*
retrieve_from_clipboard() gets the text from the clipboard,
stores it in an internal buffer, and then returns a pointer
to the text.  The CALLER must free the buffer, before
calling this function a second time.
*/

static char *retrieve_from_clipboard(eObject)
Editor eObject;
{
	char *buffer;
	unsigned long buf_len, num_bytes;
	int attempts, status;
	Display *dpy = XtDisplay(eObject->edit);
	Window win = XtWindow(eObject->edit);

	for (attempts = 0;
			attempts < 3 &&
			(status = XmClipboardInquireLength(dpy, win, "STRING", &buf_len))
				== ClipboardLocked;
			attempts++)
		;
	if (buf_len == 0 || status != ClipboardSuccess)
		return NULL;
	if ((buffer = XtMalloc((Cardinal) (buf_len + 2))) == NULL)
		return NULL;	/* return silently */
	while ((status = XmClipboardRetrieve(dpy, win, "STRING", buffer,
			(unsigned long) (buf_len + 1), &num_bytes, NULL))
			== ClipboardLocked)
		;
	if (status == ClipboardSuccess && num_bytes == buf_len) {
		buffer[num_bytes] = '\0';
		return buffer;
	}
	else {
		XtFree(buffer);
		return NULL;
	}
}	/* retrieve_from_clipboard */
#endif /* MANUAL_CLIP */

