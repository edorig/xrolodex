/**** listshell.c ****/  /**** formatted with 4-column tabs ****/

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
A `ListShell' object is a Motif XmList widget in a shell.  The
application can specify whether the pop-up shell should disappear
or stay active when the user selects an item from the list.
Because it's in a pop-up shell, the application must call the
list-shell realize function.

Sample usage:

#include "listshell.h"
...
...
void main(argc, argv)
int argc;
char *argv[];
{
	void ListSelection(), ...;
	Widget topLevel, ...;
	ListShell list1, list2;
	Arg args[10];
	int i;
	static char *seasons[] = {"Winter", "Spring", "Summer", "Fall", NULL};
	XtAppContext app;

	topLevel = XtAppInitialize(&app, "ApplicationClass",
		(XrmOptionDescList) NULL, 0,
		&argc, argv, (String *) NULL, (ArgList) NULL, 0);
	list1 = listShell_create(topLevel, "ApplicationClass", "listShell1",
		seasons, XmSTRING_DEFAULT_CHARSET, 0, 4, listShell_STAY_UP,
		listShell_WIN_MGR_DECOR);
	listShell_add_callback(list1, ListSelection, 1);
	list2 = listShell_create(topLevel, "ApplicationClass", "listShell2",
		seasons, XmSTRING_DEFAULT_CHARSET, 0, 2, listShell_NO_STAY_UP,
		listShell_MIN_WIN_MGR_DECOR);
	listShell_add_callback(list2, ListSelection, 2);
	...
	...
	XtRealizeWidget(topLevel);
	listShell_realize(list1);
	listShell_realize(list2);
	...
	...
**************************************************************************/


#include "listshell.h"


/*
Private callback functions:
*/

static void ListShellDismiss();


/*
Private support functions:
*/

static int listShell_allocate_list();


/*
Public functions:
*/

/*
listShell_create() creates a `ListShell' object.  The
application can specify the type of window manager
behavior, among other parameters.
*/
/*ARGSUSED*/
ListShell listShell_create(ref_wid, app_class, instance_name,
	items, char_set, num_cols, num_rows, stay_up_behavior,
	win_mgr_behavior)
Widget ref_wid;
char *app_class;
char *instance_name;
char *items[];
XmStringCharSet char_set;
int num_cols, num_rows;
int stay_up_behavior, win_mgr_behavior;
{
	Arg args[10];
	int i;
	ListShell lObject;
#ifdef RESTRICT_ACTION_AREA
	Dimension height, margin_height;
#endif

	if (!(lObject = (ListShell) XtMalloc((Cardinal) sizeof(_ListShell))))
		return NULL;
	lObject->self = lObject;
	i = 0;
	XtSetArg(args[i], XmNallowShellResize, (XtArgVal) True); i++;
	XtSetArg(args[i], XmNmappedWhenManaged, (XtArgVal) False); i++;
	if (win_mgr_behavior == listShell_WIN_MGR_DECOR)
		lObject->listShell = XtCreatePopupShell(instance_name,
			topLevelShellWidgetClass, ref_wid, args, i);
	else
		lObject->listShell = XtCreatePopupShell(instance_name,
			transientShellWidgetClass, ref_wid, args, i);
	/*
	instances have precedence over classes:
	*/
	lObject->instance = XtCreateManagedWidget(instance_name,
		xmFrameWidgetClass, lObject->listShell, NULL, 0);
	lObject->class = XtCreateManagedWidget("ListShell",
		xmFrameWidgetClass, lObject->instance, NULL, 0);
	i = 0;
	XtSetArg(args[i], XmNsashWidth, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNsashHeight, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNsashIndent, (XtArgVal) -1); i++;
	lObject->listPane = XtCreateManagedWidget("listPane",
		xmPanedWindowWidgetClass, lObject->class, args, i);
	lObject->listPort = XtCreateManagedWidget("listPort",
		xmScrolledWindowWidgetClass, lObject->listPane, NULL, 0);
	lObject->is_active = FALSE;
	lObject->num_items = 0;
	lObject->list_items = NULL;
	i = 0;
	if (items) {
		if (listShell_allocate_list(lObject, items, char_set)) {
			XtSetArg(args[i], XmNitems, (XtArgVal) lObject->list_items); i++;
			XtSetArg(args[i], XmNitemCount, (XtArgVal) lObject->num_items); i++;
		}
	}
	if (num_rows > 0) {
		XtSetArg(args[i], XmNvisibleItemCount, (XtArgVal) num_rows); i++;
	}
/*	XtSetArg(args[i], XmNlistSizePolicy, (XtArgVal) XmRESIZE_IF_POSSIBLE); i++;*/
/*	XtSetArg(args[i], XmNselectionPolicy, (XtArgVal) XmBROWSE_SELECT); i++;*/
/*	XtSetArg(args[i], XmNtraversalOn, (XtArgVal) False); i++;*/
	lObject->listList = XtCreateManagedWidget("listList",
		xmListWidgetClass, lObject->listPort, args, i);
	if (stay_up_behavior != listShell_STAY_UP)
		XtAddCallback(lObject->listList, XmNbrowseSelectionCallback,
			ListShellDismiss, (XtPointer) lObject);
	lObject->stay_up_behavior = stay_up_behavior;

	i = 0;
	XtSetArg(args[i], XmNadjustLast, (XtArgVal) False); i++;
	XtSetArg(args[i], XmNfractionBase, 3); i++;
	lObject->listAction = XtCreateManagedWidget("listAction",
		xmFormWidgetClass, lObject->listPane, args, i);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Dismiss", char_set)); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNleftPosition, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNrightPosition, (XtArgVal) 2); i++;
	XtSetArg(args[i], XmNshowAsDefault, (XtArgVal) True); i++;
	lObject->listDismissButton = XtCreateManagedWidget("listDismissButton",
		xmPushButtonWidgetClass, lObject->listAction, args, i);
	XtAddCallback(lObject->listDismissButton,
		XmNactivateCallback, ListShellDismiss, (XtPointer) lObject);
	if (num_cols > 0) {
		XmFontList fontlist;
		XmFontContext context;

		i = 0;
		XtSetArg(args[i], XmNfontList, &fontlist); i++;
		XtGetValues(lObject->listList, args, i);
		if (XmFontListInitFontContext(&context, fontlist)) {
			XFontStruct *font;
			XmStringCharSet charset;

			if (XmFontListGetNextFont(context, &charset, &font)) {
				Dimension temp_width, width;

				width = (num_cols + 1) * font->max_bounds.width;	/* hack */
				i = 0;
				XtSetArg(args[i], XmNhighlightThickness, &temp_width); i++;
				XtGetValues(lObject->listList, args, i);
				width += 2 * temp_width;
				i = 0;
				XtSetArg(args[i], XmNlistMarginWidth, &temp_width); i++;
				XtGetValues(lObject->listList, args, i);
				width += 2 * temp_width;
				i = 0;
				XtSetArg(args[i], XmNscrolledWindowMarginWidth, &temp_width); i++;
				XtGetValues(lObject->listPort, args, i);
				width += 2 * temp_width;
				i = 0;
				XtSetArg(args[i], XmNwidth, (XtArgVal) width); i++;
				XtSetValues(lObject->listList, args, i);
			}
			XmFontListFreeFontContext(context);
		}
	}
	/*
	restrict the height of the action area:
	*/
#ifdef RESTRICT_ACTION_AREA
	i = 0;
	XtSetArg(args[i], XmNmarginHeight, &margin_height); i++;
	XtGetValues(lObject->listAction, args, i);
	i = 0;
	XtSetArg(args[i], XmNheight, &height); i++;
	XtGetValues(lObject->listDismissButton, args, i);
	i = 0;
	XtSetArg(args[i], XmNpaneMinimum,
		(XtArgVal) (height + (margin_height * 2))); i++;
	XtSetArg(args[i], XmNpaneMaximum,
		(XtArgVal) (height + (margin_height * 2))); i++;
#else
	i = 0;
	XtSetArg(args[i], XmNskipAdjust, (XtArgVal) True); i++;
#endif
	XtSetValues(lObject->listAction, args, i);
	remove_sash_traversal(lObject->listPane);
	return lObject;
}	/* listShell_create */


/*
listShell_destroy() frees the storage for a `ListShell' object.
*/

void listShell_destroy(lObject)
ListShell lObject;
{
	XtDestroyWidget(lObject->listShell);
	listShell_free_list(lObject);
	XtFree((char *) lObject->self);
}	/* listShell_destroy */


/*
listShell_make_list() builds a list from a null-terminated
array of strings and initializes the list widget's item resources.
*/

void listShell_make_list(lObject, items, char_set)
ListShell lObject;
char *items[];
XmStringCharSet char_set;
{
	if (listShell_allocate_list(lObject, items, char_set)) {
		Arg args[5];
		int i;

		i = 0;
		XtSetArg(args[i], XmNitems, (XtArgVal) lObject->list_items); i++;
		XtSetArg(args[i], XmNitemCount, (XtArgVal) lObject->num_items); i++;
		XtSetValues(lObject->listList, args, i);
	}
}	/* listShell_make_list */


/*
listShell_free_list() frees the storage for the list's
array of strings.
*/

void listShell_free_list(lObject)
ListShell lObject;
{
	if (lObject->list_items) {
		int i;

		for (i = 0; i < lObject->num_items; i++)
			XmStringFree(lObject->list_items[i]);
		XtFree((char *) lObject->list_items);
		lObject->list_items = NULL;
	}
}	/* listShell_free_list */


/*
listShell_realize() realizes the top-level shell window
that houses the list selection widget.
*/

void listShell_realize(lObject)
ListShell lObject;
{
	XtRealizeWidget(lObject->listShell);
}	/* listShell_realize */


/*
listShell_activate() maps the `ListShell' window.
*/

void listShell_activate(lObject)
ListShell lObject;
{
	if (!XtIsRealized(lObject->listShell))
		return;
	lObject->is_active = TRUE;
	XmListDeselectAllItems(lObject->listList);
	XtMapWidget(lObject->listShell);
}	/* listShell_activate */


/*
listShell_deactivate() removes the `ListShell' window.
*/

void listShell_deactivate(lObject)
ListShell lObject;
{
	lObject->is_active = FALSE;
	XmListDeselectAllItems(lObject->listList);
	XtUnmapWidget(lObject->listShell);
}	/* listShell_deactivate */


/*
listShell_is_active() determines whether or not the object
is currenty active/mapped.
*/

int listShell_is_active(lObject)
ListShell lObject;
{
	return lObject->is_active;
}	/* listShell_is_active */


/*
listShell_add_callback() registers a callback for the
list widget.
*/

void listShell_add_callback(lObject, cb_function, client_data)
ListShell lObject;
void (*cb_function)();
XtPointer client_data;
{
	XtAddCallback(lObject->listList,
		XmNbrowseSelectionCallback, cb_function, client_data);
}	/* listShell_add_callback */


/*
For the following functions listShell_<widget_name>() returns
the widget IDs of the internal widgets.
*/

Widget listShell_listShell(lObject)
ListShell lObject;
{
	return lObject->listShell;
}	/* listShell_listShell */


Widget listShell_instance(lObject)
ListShell lObject;
{
	return lObject->instance;
}	/* listShell_instance */


Widget listShell_class(lObject)
ListShell lObject;
{
	return lObject->class;
}	/* listShell_class */


Widget listShell_listPane(lObject)
ListShell lObject;
{
	return lObject->listPane;
}	/* listShell_listPane */


Widget listShell_listAction(lObject)
ListShell lObject;
{
	return lObject->listAction;
}	/* listShell_listAction */


Widget listShell_listDismissButton(lObject)
ListShell lObject;
{
	return lObject->listDismissButton;
}	/* listShell_listDismissButton */


Widget listShell_listPort(lObject)
ListShell lObject;
{
	return lObject->listPort;
}	/* listShell_listPort */


Widget listShell_listList(lObject)
ListShell lObject;
{
	return lObject->listList;
}	/* listShell_listList */


/*
Private support functions:
*/

/*
listShell_allocate_list() builds a dynamic list from a
null-terminated array of strings.
*/

static int listShell_allocate_list(lObject, items, char_set)
ListShell lObject;
char *items[];
XmStringCharSet char_set;
{
	int i;

	listShell_free_list(lObject);
	if (!items)
		return FALSE;
	for (lObject->num_items = 0; items[lObject->num_items];
			lObject->num_items++)
		/* count the number of items */;
	lObject->list_items = (XmString *)
		XtMalloc((Cardinal) (sizeof(XmString) * lObject->num_items));
	if (lObject->list_items) {
		for (i = 0; i < lObject->num_items; i++)
			lObject->list_items[i] = XmStringCreateLtoR(items[i], char_set);
		return TRUE;
	}
	else
		return FALSE;
}	/* listShell_allocate_list */


/*
Private callback functions:
*/

/*
ListShellDismiss() is a private callback that removes
the `ListShell' window.
*/
/*ARGSUSED*/
static void ListShellDismiss(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	ListShell lObject = (ListShell) client_data;

	XtUnmapWidget(lObject->listShell);
}	/* ListShellDismiss */

