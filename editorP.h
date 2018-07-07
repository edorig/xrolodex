/**** editorP.h ****/  /**** formatted with 4-column tabs ****/

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


#ifndef _editorP_h
#define _editorP_h


/*
Private structure:
*/

typedef struct {
	void *self;
	char *app_name;					/* for the clipboard label */
	Widget instance;
	Widget class;
	Widget editBox;
	Widget menuBar;
	Widget popupMenu;
	Widget editWindow;
	Widget edit;
	Widget replaceShell;
	Widget replaceInstance;
	Widget replaceClass;
	Widget replacePane;
	Widget replaceControl;
	Widget replaceLabel;
	Widget replaceFindBox;
	Widget replaceReplaceBox;
	Widget replaceFindLabel;
	Widget replaceReplaceLabel;
	Widget replaceFindText;
	Widget replaceReplaceText;
	Widget replaceAction;
	Widget replaceFindButton;
	Widget replaceReplaceButton;
	Widget replaceDismissButton;
} _Editor;


#endif /* _editorP_h */

