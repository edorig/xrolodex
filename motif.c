/**** motif.c ****/  /**** formatted with 4-column tabs ****/

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


/********************************************************************
This module provides miscellaneous functions for Motif.  They should
be compiled and stored in a library.
********************************************************************/


#include "motif.h"



/*
create_menus() is a simple menu creation function for Motif
pop-up and pull-down menus.  It can descend a menu hierarchy
creating multiple menus.
*/

void create_menus(parent, menu, char_set)
Widget parent;
menu_entry menu[];
XmStringCharSet char_set;
{
	Arg args[5];
	int i, n, current_type;

	for (i = 0; menu[i].type != menu_END; i++) {
		if (!menu[i].instance_name)
			menu[i].instance_name = "menuInstance";
		if (menu[i].type == menu_TITLE) {
			n = 0;
			if (menu[i].label) {
				XtSetArg(args[n], XmNlabelString,
					XmStringCreateLtoR(menu[i].label, char_set)); n++;
			}
			menu[i].id = XtCreateManagedWidget(menu[i].instance_name,
				xmLabelWidgetClass, parent, args, n);
			XtCreateManagedWidget("titleSeparator",
				xmSeparatorWidgetClass, parent, NULL, 0);
		}
		else if (menu[i].type == menu_SEPARATOR) {
			menu[i].id = XtCreateManagedWidget(menu[i].instance_name,
				xmSeparatorWidgetClass, parent, NULL, 0);
		}
		else if (menu[i].type == menu_SUBMENU) {
			Widget sub_menu;

			sub_menu = XmCreatePulldownMenu(parent,
				"menuPane", NULL, 0);
			n = 0;
			XtSetArg(args[n], XmNsubMenuId, sub_menu); n++;
			if (menu[i].label) {
				XtSetArg(args[n], XmNlabelString,
					XmStringCreateLtoR(menu[i].label, char_set)); n++;
			}
			menu[i].id = XtCreateManagedWidget(menu[i].instance_name,
				xmCascadeButtonWidgetClass, parent, args, n);
			create_menus(sub_menu, menu[i].menu, char_set);
		}
		else {	/* attempt to create a menu entry */
			if (i == 0)
				current_type = menu[i].type;
			else {
				if (menu[i].type != current_type)
					fprintf(stderr, "Heterogeneous menu entry types!!!\n");
			}
			n = 0;
			if (menu[i].label) {
				XtSetArg(args[n], XmNlabelString,
					XmStringCreateLtoR(menu[i].label, char_set)); n++;
			}
			menu[i].id = XtCreateManagedWidget(menu[i].instance_name,
				(current_type == menu_TOG_ENTRY) ?
					xmToggleButtonWidgetClass : xmPushButtonWidgetClass,
				parent, args, n);
			if (menu[i].callback)
				XtAddCallback(menu[i].id,
					(current_type == menu_TOG_ENTRY) ?
						XmNvalueChangedCallback : XmNactivateCallback,
					menu[i].callback, menu[i].data);
		}
	}	/* for every entry in the menu */
}	/* create_menus */


/*
remove_sash_traversal() takes the hidden sashes for the XmPanedWindow
widget out of contention for traversal so that the user can tab
between the control and action areas.  Thanks to, and see, Heller [1991]
for a more extensive discussion of tab groups and traversal issues.
*/

void remove_sash_traversal(pane)
Widget pane;
{
	Arg args[3];
	Widget *children;
	int i, num_children;

	i = 0;
	XtSetArg(args[i], XmNchildren, &children); i++;
	XtSetArg(args[i], XmNnumChildren, &num_children); i++;
	XtGetValues(pane, args, i);
	while (num_children-- > 0)
		if (XmIsSash(children[num_children])) {
			XtSetArg(args[0], XmNtraversalOn, (XtArgVal) False);
			XtSetValues(children[num_children], args, 1);
		}
}	/* remove_sash_traversal */

