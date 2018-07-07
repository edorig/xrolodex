/**** editor.h ****/  /**** formatted with 4-column tabs ****/

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


#ifndef _editor_h
#define _editor_h

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/CutPaste.h>


#include "editorP.h"
#include "motif.h"
#include "strstr.h"


/*
Constants used to control scrolling behavior:
*/

#define editor_SCROLL_LEFT_ONLY		-100
#define editor_SCROLL_RIGHT_ONLY	-101
#define editor_SCROLL_DEFAULT		-102

#define editor_WIN_MGR_DECOR		-200
#define editor_MIN_WIN_MGR_DECOR	-201

#define editor_MENU_BAR				-300
#define editor_POPUP_MENU			-301


/*
Constants required with LessTif:
*/

/***********************************************
#define ClipboardFail		XmClipboardFail
#define ClipboardSuccess	XmClipboardSuccess
#define ClipboardTruncate	XmClipboardTruncate
#define ClipboardLocked		XmClipboardLocked
#define ClipboardBadFormat	XmClipboardBadFormat
#define ClipboardNoData		XmClipboardNoData
***********************************************/

/*
Public opaque pointer:
*/

typedef _Editor *Editor;


/*
Public functions:
*/

extern Editor editor_create();
extern void editor_destroy();
extern void editor_realize();
extern void editor_set_sensitive();
extern void editor_set_nonsensitive();
extern char *editor_get_text();
extern void editor_set_text();

/*
Access functions:
*/

extern Widget editor_instance();
extern Widget editor_class();
extern Widget editor_editBox();
extern Widget editor_menuBar();
extern Widget editor_popupMenu();
extern Widget editor_menuEditButton();
extern Widget editor_menuFindButton();
extern Widget editor_menuCutButton();
extern Widget editor_menuCopyButton();
extern Widget editor_menuPasteButton();
extern Widget editor_menuClearButton();
extern Widget editor_menuSearchButton();
extern Widget editor_menuReplaceButton();
extern Widget editor_editWindow();
extern Widget editor_edit();
extern Widget editor_replaceShell();
extern Widget editor_replaceInstance();
extern Widget editor_replaceClass();
extern Widget editor_replacePane();
extern Widget editor_replaceControl();
extern Widget editor_replaceLabel();
extern Widget editor_replaceFindBox();
extern Widget editor_replaceReplaceBox();
extern Widget editor_replaceFindLabel();
extern Widget editor_replaceReplaceLabel();
extern Widget editor_replaceFindText();
extern Widget editor_replaceReplaceText();
extern Widget editor_replaceAction();
extern Widget editor_replaceFindButton();
extern Widget editor_replaceReplaceButton();
extern Widget editor_replaceDismissButton();


#endif /* _editor_h */

