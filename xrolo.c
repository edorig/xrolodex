/**** xrolo.c ****/  /**** formatted with 4-column tabs ****/

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
`xrolodex' implements a simple rolodex application.  It is composed
of the following modules:
	`xrolo.c'		-- widget creation for the top-level interface,
					   plus callbacks and support functions.
	`xrolo.db.c'	-- rolodex entry manipulation functions and
					   the state of the database.
	`xrolo.help.txt'-- a module for providing help system text.
	`xrolo.index.c'	-- the rolodex index manipulation functions.

	`streamdb.c'	-- a stream- or byte-oriented database object.
	`strstr.c'		-- a string search module.
	`ctrlpanel.c'	-- a control panel pseudo-widget.
	`editor.c'		-- a self-contained editor pseudo-widget.
	`listshell.c'	-- a list selection box pseudo-widget.
	`dialog.c'		-- a dialog box pseudo-widget.
	`help.c'		-- a generalized help system.
	`motif.c'		-- OSF/Motif miscellaneous functions.

See these modules for more details.

Note that this implementation contains source code specifically
designed to provide conversation, sometimes debate, as well as
examples that motivate discussion of certain techniques in both
lecture and laboratory settings.  For example, we use one
dialog box, `messageDialog' to display different messages
(efficient), but in another case, we use two dialog boxes in a
nonmodal situation simply to distinguish between the context in
which their callbacks are activated (not as efficient).
********************************************************************/

#include <stdlib.h> 
#include "xrolo.h"


/*
Private functions:
*/

static Widget create_main_menu_bar();
static CtrlPanel create_entry_control_panel();
static void create_dialogs();
static int check_for_unsaved_changes();
static void load_next_rolodex_entry();
static void load_previous_rolodex_entry();
static void new_entry();
static int save_current_text();
static void insert_new_db_entry();
static void save_current_db_entry();
static void find_entry_containing_text_forward();
static void find_entry_containing_text_reverse();
static void print_entry_or_file();
static void open_rolodex_file(), save_as_rolodex_file();
static void do_save_as_operation();
static void save_first_time();
static void update_current_entry();
static int is_rolodex_active();
static void initialize_critical_variables();
static void cleanup_and_exit();
static void user_message();
static void initialize_shell_icons();
static void handle_close_buttons();
static int convert_delimiter_string();
#ifdef ALT_ERROR_HANDLER
static void app_error_handler();
#else
	/* add nothing */
#endif

#ifdef EDITRES
extern void _XEditResCheckMessages();
#endif


/*
Private callbacks:
*/

static void About(), Open(), Save(), SaveAs(), Exit();
static void Next(), Previous(), First(), Last();
static void Index(), Find(), Sort(), Delimiter(), SetIndexRow(), SetSortRow();
static void Insert(), InsertAfter(), Copy(), Delete(), Undelete();
static void EntryModified(), IndexSelection();
static void SensitiveSearch(), WraparoundSearch();
static void FileSelectionOK(), FileSelectionCancel();
static void OverwriteOK(), OverwriteCancel();
static void DelimiterOK(), DelimiterCancel(), DelimiterReset();
static void SetIndexRowOK(), SetIndexRowCancel(), SetIndexRowReset();
static void SetSortRowOK(), SetSortRowCancel(), SetSortRowReset();
static void SaveUnsavedChanges(), DiscardUnsavedChanges(),
	CancelUnsavedChanges();
static void FindForward(), FindReverse(), FindDismiss();
static void Close(), UnmanageWindow(), UnmapWindow(), DialogCancel();
static void Print(), PrintCurrent(), PrintAll(), PrintDismiss();
static void PrintStandardOutput(), PrintFormfeed();


/*
Data structures for the help text:
*/

#include "xrolo.help.txt"



/*
Private globals (can be modified):
*/

static int start_up_index_row;		/* see resources */
static int index_row;				/* see resources */
static int start_up_sort_row;		/* see resources */
static int sort_row;				/* see resources */
static int force_save;				/* see resources */
static int index_stay_up;			/* see resources */
static char *start_up_delimiter;	/* see resources */
static char *delimiter;				/* see resources */
static int case_sensitive_search;	/* see resources */
static int wraparound_search;		/* see resources */
static char *print_command;			/* see resources */
static int print_standard_output;	/* see resources */
static int print_formfeed;			/* see resources */
static char *save_as_filename = NULL;
static Pixmap icon_pixmap;


/*
Private globals (global for callback convenience only -- never modified):
*/

static CtrlPanel entryPanel;
static Editor entryEditWindow;
static ListShell entryIndex;
static Dialog openUnsavedDialog, exitUnsavedDialog, overwriteDialog,
	delimiterDialog, indexDialog, sortDialog, messageDialog, aboutDialog;
static Widget databaseLabel, fileSelectionDialog;
static Widget findShell, findText;
static Widget printShell, printText;


/*
Private globals (these variables maintain the state of rolodex entries):
*/

static int editing_new_entry = FALSE;
static EntryDB current_entry = NULL; /* opaque usage as indicator variable */
static EntryDB temp_entry = NULL;    /* opaque usage as indicator variable */
static int entry_modified = FALSE;
static int first_time = TRUE;


/*
Private character set, passed to other modules:
*/

static XmStringCharSet char_set = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;


/*
Private resource globals (set during start-up):
*/

static XtResource resources[] = {
	{XtNviewportMenuBar, XtCViewportMenuBar, XtRBoolean, sizeof(Boolean),
		XtOffset(ApplicationDataPtr, viewport_menubar), XtRImmediate,
		(XtPointer) FALSE},
	{XtNviewportRows, XtCViewportRows, XtRInt, sizeof(int),
		XtOffset(ApplicationDataPtr, viewport_rows), XtRImmediate,
		(XtPointer) xrolo_DEFAULT_VIEWPORT_ROWS},
	{XtNviewportColumns, XtCViewportColumns, XtRInt, sizeof(int),
		XtOffset(ApplicationDataPtr, viewport_columns), XtRImmediate,
		(XtPointer) xrolo_DEFAULT_VIEWPORT_COLUMNS},
	{XtNindexRows, XtCIndexRows, XtRInt, sizeof(int),
		XtOffset(ApplicationDataPtr, index_rows), XtRImmediate,
		(XtPointer) xrolo_DEFAULT_INDEX_ROWS},
	{XtNindexColumns, XtCIndexColumns, XtRInt, sizeof(int),
		XtOffset(ApplicationDataPtr, index_columns), XtRImmediate,
		(XtPointer) xrolo_DEFAULT_INDEX_COLUMNS},
	{XtNcenterDialogs, XtCCenterDialogs, XtRBoolean, sizeof(Boolean),
		XtOffset(ApplicationDataPtr, center_dialogs), XtRImmediate,
		(XtPointer) TRUE},
	{XtNminimalDialogs, XtCMinimalDialogs, XtRBoolean, sizeof(Boolean),
		XtOffset(ApplicationDataPtr, minimal_dialogs), XtRImmediate,
		(XtPointer) TRUE},
	{XtNforceSave, XtCForceSave, XtRBoolean, sizeof(Boolean),
		XtOffset(ApplicationDataPtr, force_save), XtRImmediate,
		(XtPointer) TRUE},
	{XtNindexStayUp, XtCIndexStayUp, XtRBoolean, sizeof(Boolean),
		XtOffset(ApplicationDataPtr, index_stay_up), XtRImmediate,
		(XtPointer) TRUE},
	{XtNfindSensitive, XtCFindSensitive, XtRBoolean, sizeof(Boolean),
		XtOffset(ApplicationDataPtr, find_sensitive), XtRImmediate,
		(XtPointer) FALSE},
	{XtNfindWraparound, XtCFindWraparound, XtRBoolean, sizeof(Boolean),
		XtOffset(ApplicationDataPtr, find_wraparound), XtRImmediate,
		(XtPointer) FALSE},
	{XtNentryDelimiter, XtCEntryDelimiter, XtRString, sizeof(String),
		XtOffset(ApplicationDataPtr, delimiter),
		XtRString, xrolo_DEFAULT_DELIMITER},
	{XtNprintCommand, XtCPrintCommand, XtRString, sizeof(String),
		XtOffset(ApplicationDataPtr, print_command),
		XtRString, xrolo_DEFAULT_PRINT_COMMAND},
	{XtNprintStandardOutput, XtCPrintStandardOutput, XtRBoolean, sizeof(Boolean),
		XtOffset(ApplicationDataPtr, print_standard_output), XtRImmediate,
		(XtPointer) FALSE},
	{XtNprintFormfeed, XtCPrintFormfeed, XtRBoolean, sizeof(Boolean),
		XtOffset(ApplicationDataPtr, print_formfeed), XtRImmediate,
		(XtPointer) FALSE},
	{XtNsortRow, XtCSortRow, XtRInt, sizeof(int),
		XtOffset(ApplicationDataPtr, sort_row), XtRImmediate,
		(XtPointer) xrolo_DEFAULT_SORT_ROW},
	{XtNindexRow, XtCIndexRow, XtRInt, sizeof(int),
		XtOffset(ApplicationDataPtr, index_row), XtRImmediate,
		(XtPointer) xrolo_DEFAULT_INDEX_ROW},
	{XtNdirectoryMask, XtCDirectoryMask, XtRString, sizeof(String),
		XtOffset(ApplicationDataPtr, dir_mask),
		XtRString, ""},	/* default is home directory; see create_dialogs() */
};

#ifdef FALLBACK_RES
char *fallback_resources[] = {
	"*mainWindow*horizontalSpacing: 5",
	"*mainWindow*verticalSpacing: 5",
	"*databaseLabel*marginWidth: 5",
	"*databaseLabel*marginHeight: 5",
	"*ctrlBox*marginWidth: 5",
	"*ctrlBox*marginHeight: 7",
	"*editBox*horizontalSpacing: 5",
	"*editBox*verticalSpacing: 5",
	"*helpShell*title: xrolodex",
	"*helpPanedWindow*sashWidth: 20",
	"*helpPanedWindow*sashHeight: 8",
	"*helpPanedWindow*sashIndent: -50",
	"*helpPanedWindow*spacing: 14",
	"*helpList*visibleItemCount: 10",
	"*helpList*scrollBarDisplayPolicy: STATIC",
	"*helpList*listMarginWidth: 5",
	"*helpList*listMarginHeight: 5",
	"*helpText*marginWidth: 5",
	"*helpText*marginHeight: 5",
	"*helpText*scrollBarDisplayPolicy: STATIC",
	"*helpAction*marginWidth: 5",
	"*helpAction*marginHeight: 5",
	"*listShell*title: xrolodex",
	"*listList*listMarginWidth: 5",
	"*listList*listMarginHeight: 5",
	"*listAction*marginWidth: 5",
	"*listAction*marginHeight: 5",
	"*exitUnsavedDialog*title: xrolodex",
	"*openUnsavedDialog*title: xrolodex",
	"*overwriteDialog*title: xrolodex",
	"*delimiterDialog*title: xrolodex",
	"*sortDialog*title: xrolodex",
	"*indexDialog*title: xrolodex",
	"*messageDialog*title: xrolodex",
	"*aboutDialog*title: xrolodex",
	"*dialogControl*horizontalSpacing: 5",
	"*dialogControl*verticalSpacing: 5",
	"*dialogAction*marginWidth: 5",
	"*dialogAction*marginHeight: 5",
	"*findShell*title: xrolodex",
	"*findControl*horizontalSpacing: 5",
	"*findControl*verticalSpacing: 5",
	"*findAction*marginWidth: 5",
	"*findAction*marginHeight: 5",
	"*replaceShell*title: xrolodex",
	"*replaceControl*horizontalSpacing: 5",
	"*replaceControl*verticalSpacing: 5",
	"*replaceAction*marginWidth: 5",
	"*replaceAction*marginHeight: 5",
	"*printShell*title: xrolodex",
	"*printControl*horizontalSpacing: 5",
	"*printControl*verticalSpacing: 5",
	"*printAction*marginWidth: 5",
	"*printAction*marginHeight: 5",
	NULL
};
#endif


/*
main() creates a top-level window from a system of manager
widgets, using several pseudo-widgets for the control panel,
the edit area for rolodex entries, and the index facility.
*/

void main(argc, argv)
int argc;
char *argv[];
{
	Arg args[10];
	int i;
	XtAppContext app;
	Widget topLevel, mainWindow, mainMenuBar;
	static XtCallbackRec text_mod_cb_list[] = {
		{EntryModified, (XtPointer) NULL},
		{(XtCallbackProc) NULL, (XtPointer) NULL},
	};
	ApplicationData resource_data;
	char *icon_name;

	if ((argc == 2 && strcmp(argv[1], "-help") == 0)) {
		printf("\nusage:  xrolodex [<filename>] [-iconName <icon name>]\n\n");
		exit(0);
	}
	topLevel = XtAppInitialize(&app, xrolo_APP_CLASS,
		(XrmOptionDescList) NULL, 0, &argc, argv,
#ifdef FALLBACK_RES
		fallback_resources,
#else
		(char **) NULL,
#endif
		(ArgList) NULL, 0);
	if (argc > 4) {
		printf("\nusage:  xrolodex [<filename>] [-iconName <icon name>]\n");
		exit(0);
	}
	XtGetApplicationResources(topLevel, &resource_data, resources,
		XtNumber(resources), NULL, 0);
	delimiter = start_up_delimiter = resource_data.delimiter; /* set globals */
	force_save = resource_data.force_save; /* set globals */
	index_stay_up = resource_data.index_stay_up; /* set globals */
	case_sensitive_search = resource_data.find_sensitive; /* set globals */
	wraparound_search = resource_data.find_wraparound; /* set globals */
	sort_row = start_up_sort_row = resource_data.sort_row; /* set globals */
	index_row = start_up_index_row = resource_data.index_row; /* set globals */
	print_command = resource_data.print_command;
	print_standard_output = resource_data.print_standard_output; /* set globals */
	print_formfeed = resource_data.print_formfeed; /* set globals */
	mainWindow = XtCreateManagedWidget("mainWindow",
		xmFormWidgetClass, topLevel, NULL, 0);

	mainMenuBar = create_main_menu_bar(mainWindow);

	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNtopWidget, (XtArgVal) mainMenuBar); i++;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("  ", char_set)); i++;
	databaseLabel = XtCreateManagedWidget("databaseLabel",
		xmLabelWidgetClass, mainWindow, args, i);

	entryPanel = create_entry_control_panel(mainWindow);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNtopWidget, (XtArgVal) databaseLabel); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetValues(ctrlPanel_instance(entryPanel), args, i);

	entryEditWindow =
		editor_create(mainWindow, xrolo_APP_CLASS, "entryEditWindow",
		resource_data.viewport_rows, resource_data.viewport_columns, 40,
		editor_SCROLL_DEFAULT, "", char_set, "xrolodex",
		"Editor/Viewport Search and Replace",
		resource_data.minimal_dialogs ?
			editor_MIN_WIN_MGR_DECOR : editor_WIN_MGR_DECOR,
		resource_data.viewport_menubar ? editor_MENU_BAR : editor_POPUP_MENU);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNleftWidget, (XtArgVal) ctrlPanel_instance(entryPanel)); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNtopWidget, (XtArgVal) databaseLabel); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetValues(editor_instance(entryEditWindow), args, i);
	i = 0;
	XtSetArg(args[i], XmNmodifyVerifyCallback, text_mod_cb_list); i++;
	XtSetValues(editor_edit(entryEditWindow), args, i);

	/*
	the list selection box's items will be added later:
	*/
	entryIndex = listShell_create(topLevel, xrolo_APP_CLASS, "listShell",
		NULL, char_set,
		resource_data.index_columns, resource_data.index_rows,
		index_stay_up ? listShell_STAY_UP : listShell_NO_STAY_UP,
		resource_data.minimal_dialogs ?
			listShell_MIN_WIN_MGR_DECOR : listShell_WIN_MGR_DECOR);
	listShell_add_callback(entryIndex, IndexSelection, 0);
	xrolo_index_create(entryIndex);

	create_dialogs(app, topLevel, &resource_data);
	help_create_dialog(topLevel, xrolo_APP_CLASS,
		xrolo_help_rows, xrolo_help_columns, char_set);

	if (argc == 4 && strcmp(argv[2], "-iconName") == 0)
		icon_name = argv[3];
	else if (argc == 3 && strcmp(argv[1], "-iconName") == 0)
		icon_name = argv[2];
	else
		icon_name = NULL;
	initialize_shell_icons(topLevel, icon_name);
	XtRealizeWidget(topLevel);
	XtRealizeWidget(findShell);
	XtRealizeWidget(printShell);
	help_realize();
	editor_realize(entryEditWindow);
	listShell_realize(entryIndex);
	dialog_realize(openUnsavedDialog);
	dialog_realize(exitUnsavedDialog);
	dialog_realize(overwriteDialog);
	dialog_realize(delimiterDialog);
	dialog_realize(sortDialog);
	dialog_realize(indexDialog);
	dialog_realize(messageDialog);
	dialog_realize(aboutDialog);
	dialog_set_prompt(aboutDialog, VERSION_INFO);
	handle_close_buttons(topLevel);
	if (argc > 1 && strcmp(argv[1], "-iconName") != 0 && strlen(argv[1]) > 0)
		open_rolodex_file(argv[1]);
	XtAppMainLoop(app);
#ifdef ALT_ERROR_HANDLER
	XtAppSetErrorHandler(app, app_error_handler);
#else
	/* add nothing */
#endif
}	/* main */


/*
create_main_menu_bar() creates the application-level menu bar,
including pull-down menus and menu entries.
*/

static Widget create_main_menu_bar(parent)
Widget parent;
{
	static menu_entry file_menu[] = {
		{menu_ENTRY, "New...", "menuNewButton", Open, NULL, NULL, NULL},
		{menu_ENTRY, "Open...", "menuOpenButton", Open, NULL, NULL, NULL},
		{menu_ENTRY, "Save", "menuSaveButton", Save, NULL, NULL, NULL},
		{menu_ENTRY, "Save As...", "menuSaveAsButton", SaveAs, NULL,
			NULL, NULL},
		{menu_ENTRY, "Print...", "menuPrintButton", Print, NULL, NULL, NULL},
		{menu_ENTRY, "Exit", "menuExitButton", Exit, NULL, NULL, NULL},
		{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
	};
	static menu_entry edit_menu[] = {
		{menu_ENTRY, "Insert", "menuInsertButton", Insert, NULL, NULL, NULL},
		{menu_ENTRY, "Insert After", "menuInsertAfterButton", InsertAfter,
			NULL, NULL, NULL},
		{menu_ENTRY, "Copy", "menuCopyButton", Copy, NULL, NULL, NULL},
		{menu_ENTRY, "Delete", "menuDeleteButton", Delete, NULL,
			NULL, NULL},
		{menu_ENTRY, "Undelete", "menuUndeleteButton", Undelete, NULL,
			NULL, NULL},
		{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
	};
	static menu_entry index_row_menu[] = {
		{menu_ENTRY, "Reset to Default", "menuResetIndexButton",
			SetIndexRowReset, (XtPointer) NULL, NULL, NULL},
		{menu_ENTRY, "Modify...", "menuModifyIndexButton", SetIndexRow,
			(XtPointer) NULL, NULL, NULL},
		{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
	};
	static menu_entry find_menu[] = {
		{menu_ENTRY, "Find Entry...", "menuFindEntryButton", Find, NULL,
			NULL, NULL},
		{menu_ENTRY, "Index...", "menuIndexButton", Index, NULL,
			NULL, NULL},
		{menu_SUBMENU, "Set Index Row", "menuSetIndexRowButton", NULL, NULL,
			index_row_menu, NULL},
		{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
	};
	static menu_entry sort_row_menu[] = {
		{menu_ENTRY, "Reset to Default", "menuResetSortButton",
			SetSortRowReset, (XtPointer) NULL, NULL, NULL},
		{menu_ENTRY, "Modify...", "menuModifySortButton", SetSortRow,
			(XtPointer) NULL, NULL, NULL},
		{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
	};
	static menu_entry sort_menu[] = {
		{menu_ENTRY, "Ascending", "menuAscendingButton", Sort,
			(XtPointer) xrolo_ASCEND, NULL, NULL},
		{menu_ENTRY, "Descending", "menuDescendingButton", Sort,
			(XtPointer) xrolo_DESCEND, NULL, NULL},
		{menu_SUBMENU, "Set Sort Row", "menuSetSortRowButton", NULL, NULL,
			sort_row_menu, NULL},
		{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
	};
	static menu_entry delimiter_menu[] = {
		{menu_ENTRY, "Reset to Default", "menuResetDelimiterButton",
			DelimiterReset, (XtPointer) NULL, NULL, NULL},
		{menu_ENTRY, "Modify...", "menuModifyDelimiterButton", Delimiter,
			(XtPointer) NULL, NULL, NULL},
		{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
	};
	static menu_entry options_menu[] = {
		{menu_SUBMENU, "Set Delimiter", "menuDelimiterButton", NULL, NULL,
			delimiter_menu, NULL},
		{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
	};
	static menu_entry menus[] = {
		{menu_SUBMENU, "File", "menuFileButton", NULL, NULL, file_menu, NULL},
		{menu_SUBMENU, "Edit", "menuEditButton", NULL, NULL, edit_menu, NULL},
		{menu_SUBMENU, "Find", "menuFindButton", NULL, NULL, find_menu, NULL},
		{menu_SUBMENU, "Sort", "menuSortButton", NULL, NULL, sort_menu, NULL},
		{menu_SUBMENU, "Options", "menuOptionsButton", NULL, NULL,
			options_menu, NULL},
		{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
	};
	static menu_entry help_menu[] = {
		{menu_ENTRY, "Help...", "menuHelpWindowButton",
			Help, NULL, NULL, NULL},
		{menu_ENTRY, "About xrolodex...", "menuAboutButton",
			About, NULL, NULL, NULL},
		{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
	};
	Arg args[10];
	int i;
	Widget mainMenuBar;

	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	mainMenuBar = XmCreateMenuBar(parent, "mainMenuBar", args, i);
	XtManageChild(mainMenuBar);
	create_menus(mainMenuBar, menus, char_set);
	help_create_pulldown_menu(mainMenuBar, help_menu, help_data, char_set);
	return mainMenuBar;
}	/* create_main_menu_bar */


/*
create_entry_control_panel() creates the control panel that's
used to manipulate rolodex entries in the rolodex window.
*/

static CtrlPanel create_entry_control_panel(parent)
Widget parent;
{
	static CtrlPanelItem items[] = {
		{"First", NULL, First, NULL},
		{"Last", NULL, Last, NULL},
		{"", NULL, NULL, NULL},				/* separator */
		{"Previous", NULL, Previous, NULL},
		{"Next", NULL, Next, NULL},
		{"", NULL, NULL, NULL},				/* separator */
		{"Insert", NULL, Insert, NULL},
		{"Copy", NULL, Copy, NULL},
		{"", NULL, NULL, NULL},				/* separator */
		{"Index...", NULL, Index, NULL},
		{NULL, NULL, NULL, NULL},
	};
	CtrlPanel entryControlPanel;

	entryControlPanel = ctrlPanel_create(parent, "entryControlPanel",
		items, char_set, ctrlPanel_VERTICAL);
	return entryControlPanel;
}	/* create_entry_control_panel */


/*
create_dialogs() creates the dialog boxes used for prompting
for filenames, for prompts related to unsaved changes to
the rolodex file, the print dialog, and the find-entry window.
*/

static void create_dialogs(app, parent, resource_data)
XtAppContext app;
Widget parent;
ApplicationData *resource_data;
{
	static DialogButtonItem exit_unsaved_items[] = {
		{"Save", SaveUnsavedChanges, (XtPointer) xrolo_EXIT_WARNING},
		{"Discard", DiscardUnsavedChanges, (XtPointer) xrolo_EXIT_WARNING},
		{"Cancel", CancelUnsavedChanges, (XtPointer) xrolo_EXIT_WARNING},
		{NULL, NULL, NULL},
	};
	static DialogButtonItem open_warning_items[] = {
		{"Save", SaveUnsavedChanges, (XtPointer) xrolo_OPEN_WARNING},
		{"Discard", DiscardUnsavedChanges, (XtPointer) xrolo_OPEN_WARNING},
		{"Cancel", CancelUnsavedChanges, (XtPointer) xrolo_OPEN_WARNING},
		{NULL, NULL, NULL},
	};
	static DialogButtonItem overwrite_items[] = {
		{"Overwrite", OverwriteOK, NULL},
		{"Cancel", OverwriteCancel, NULL},
		{NULL, NULL, NULL},
	};
	static DialogButtonItem delimiter_items[] = {
		{"OK", DelimiterOK, NULL},
		{"Cancel", DelimiterCancel, NULL},
		{NULL, NULL, NULL},
	};
	static DialogButtonItem sort_items[] = {
		{"OK", SetSortRowOK, NULL},
		{"Cancel", SetSortRowCancel, NULL},
		{NULL, NULL, NULL},
	};
	static DialogButtonItem index_items[] = {
		{"OK", SetIndexRowOK, NULL},
		{"Cancel", SetIndexRowCancel, NULL},
		{NULL, NULL, NULL},
	};
	static DialogButtonItem message_items[] = {
		{"OK", NULL, NULL},
		{NULL, NULL, NULL},
	};
	static menu_entry find_toggle_menu[] = {
		{menu_TOG_ENTRY, "Case sensitive", "findSensitiveButton",
			SensitiveSearch, NULL, NULL, NULL},
		{menu_TOG_ENTRY, "Wraparound", "findWraparoundButton",
			WraparoundSearch, NULL, NULL, NULL},
		{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
	};
	static menu_entry print_toggle_menu[] = {
		{menu_TOG_ENTRY, "Print to standard output",
			"printStandardOutputButton", PrintStandardOutput, NULL, NULL, NULL},
		{menu_TOG_ENTRY, "Add formfeed after each entry",
			"printFormfeedButton", PrintFormfeed, NULL, NULL, NULL},
		{menu_END, NULL, NULL, NULL, NULL, NULL, NULL},
	};
	char dir_mask[xrolo_DIR_MASK_MAX_LEN + 2], *home;
	Arg args[20];
	int i;
#ifdef RESTRICT_ACTION_AREA
	Dimension height, margin_height;
#endif
	Widget findPane, findControl, findLabel, findAction,
		findForwardButton, findReverseButton, findDismissButton,
		findToggleBox;
	Widget printPane, printControl, printLabel, printAction,
		printCurrentButton, printAllButton, printDismissButton, printToggleBox;

	if (*resource_data->dir_mask)
		strncpy(dir_mask, resource_data->dir_mask, xrolo_DIR_MASK_MAX_LEN);
	else if ((home = (char *) getenv("HOME")) == NULL)
		strcpy(dir_mask, xrolo_DIR_MASK);
	else {
		strncpy(dir_mask, home, xrolo_DIR_MASK_MAX_LEN);
		strncat(dir_mask, "/", xrolo_DIR_MASK_MAX_LEN - strlen(home));
		strncat(dir_mask, xrolo_DIR_MASK, xrolo_DIR_MASK_MAX_LEN - strlen(home) - 1);
	}
	i = 0;
	XtSetArg(args[i], XmNdialogTitle,
		XmStringCreateLtoR("xrolodex", char_set)); i++;
	XtSetArg(args[i], XmNdirMask,
		XmStringCreateLtoR(dir_mask, char_set)); i++;
	fileSelectionDialog = (Widget) XmCreateFileSelectionDialog(parent,
		"fileSelectionDialog", args, i);
	XtAddCallback(fileSelectionDialog, XmNokCallback,
		FileSelectionOK, NULL);
	XtAddCallback(fileSelectionDialog, XmNcancelCallback,
		FileSelectionCancel, NULL);
	XtUnmanageChild((Widget) XmFileSelectionBoxGetChild(fileSelectionDialog,
		XmDIALOG_HELP_BUTTON));
/***********************************************************************
	dialog_create(app, parent, instance_name, items, num_columns,
		title, prompt, char_set, dialog_position, modal_dialog,
		max_win_mgr, string_input, auto_popdown, default_button);
***********************************************************************/
	exitUnsavedDialog = dialog_create(app, parent, "exitUnsavedDialog",
		exit_unsaved_items, 0, "", xrolo_EXIT_WARNING_MSG, char_set,
		resource_data->center_dialogs ? dialog_CENTER_PRIMARY : dialog_DEFAULT,
		FALSE, !resource_data->minimal_dialogs, FALSE, FALSE, 0);
	openUnsavedDialog = dialog_create(app, parent, "openUnsavedDialog",
		open_warning_items, 0, "", xrolo_OPEN_WARNING_MSG, char_set,
		resource_data->center_dialogs ? dialog_CENTER_PRIMARY : dialog_DEFAULT,
		FALSE, !resource_data->minimal_dialogs, FALSE, FALSE, 0);
	overwriteDialog = dialog_create(app, parent, "overwriteDialog",
		overwrite_items, 0, "", "File already exists!", char_set,
		resource_data->center_dialogs ? dialog_CENTER_PRIMARY : dialog_DEFAULT,
		FALSE, !resource_data->minimal_dialogs, FALSE, TRUE, 0);
	messageDialog = dialog_create(app, parent, "messageDialog",
		message_items, 0, "", "", char_set,
		resource_data->center_dialogs ? dialog_CENTER_PRIMARY : dialog_DEFAULT,
		FALSE, !resource_data->minimal_dialogs, FALSE, TRUE, 0);
	aboutDialog = dialog_create(app, parent, "aboutDialog",
		message_items, 0, "", "", char_set,
		resource_data->center_dialogs ? dialog_CENTER_PRIMARY : dialog_DEFAULT,
		FALSE, !resource_data->minimal_dialogs, FALSE, TRUE, 0);
	delimiterDialog = dialog_create(app, parent, "delimiterDialog",
		delimiter_items, 0, "", "Modify/Set Entry Delimiter", char_set,
/*		resource_data->center_dialogs ? dialog_CENTER_PRIMARY : dialog_DEFAULT,*/
		dialog_DEFAULT,
		FALSE, !resource_data->minimal_dialogs, TRUE, TRUE, 0);
	sortDialog = dialog_create(app, parent, "sortDialog",
		sort_items, 0, "", "Modify/Set Sort Row", char_set,
/*		resource_data->center_dialogs ? dialog_CENTER_PRIMARY : dialog_DEFAULT,*/
		dialog_DEFAULT,
		FALSE, !resource_data->minimal_dialogs, TRUE, TRUE, 0);
	indexDialog = dialog_create(app, parent, "indexDialog",
		index_items, 0, "", "Modify/Set Index Row", char_set,
/*		resource_data->center_dialogs ? dialog_CENTER_PRIMARY : dialog_DEFAULT,*/
		dialog_DEFAULT,
		FALSE, !resource_data->minimal_dialogs, TRUE, TRUE, 0);
	XtAddCallback(dialog_dialogText(delimiterDialog), XmNactivateCallback,
		DelimiterOK, NULL);
	XtAddCallback(dialog_dialogText(sortDialog), XmNactivateCallback,
		SetSortRowOK, NULL);
	XtAddCallback(dialog_dialogText(indexDialog), XmNactivateCallback,
		SetIndexRowOK, NULL);

	i = 0;
	XtSetArg(args[i], XmNallowShellResize, (XtArgVal) True); i++;
	XtSetArg(args[i], XmNmappedWhenManaged, (XtArgVal) False); i++;
	findShell = XtCreatePopupShell("findShell",
		resource_data->minimal_dialogs ?
			transientShellWidgetClass: topLevelShellWidgetClass,
		parent, args, i);
	i = 0;
	XtSetArg(args[i], XmNsashWidth, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNsashHeight, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNsashIndent, (XtArgVal) -1); i++;
	findPane = XtCreateManagedWidget("findPane",
		xmPanedWindowWidgetClass, findShell, args, i);
	findControl = XtCreateManagedWidget("findControl",
		xmFormWidgetClass, findPane, NULL, 0);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Search for the next entry containing:",
			char_set)); i++;
	XtSetArg(args[i], XmNalignment, (XtArgVal) XmALIGNMENT_CENTER); i++;
	findLabel = XtCreateManagedWidget("findLabel",
		xmLabelWidgetClass, findControl, args, i);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNtopWidget, (XtArgVal) findLabel); i++;
	XtSetArg(args[i], XmNcolumns, (XtArgVal) 40); i++;
	findText = XtCreateManagedWidget("findText",
		xmTextWidgetClass, findControl, args, i);
	XtAddCallback(findText, XmNactivateCallback, FindForward, NULL);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNtopWidget, (XtArgVal) findText); i++;
	XtSetArg(args[i], XmNorientation, (XtArgVal) XmVERTICAL); i++;
	XtSetArg(args[i], XmNindicatorType, (XtArgVal) XmN_OF_MANY); i++;
	findToggleBox = XtCreateManagedWidget("findToggleBox",
		xmRowColumnWidgetClass, findControl, args, i);
	create_menus(findToggleBox, find_toggle_menu, char_set);
	i = 0;
	XtSetArg(args[i], XmNset, (XtArgVal) case_sensitive_search); i++;
	XtSetValues(find_toggle_menu[0].id, args, i);
	i = 0;
	XtSetArg(args[i], XmNset, (XtArgVal) wraparound_search); i++;
	XtSetValues(find_toggle_menu[1].id, args, i);
	i = 0;
	XtSetArg(args[i], XmNadjustLast, (XtArgVal) False); i++;
	XtSetArg(args[i], XmNfractionBase, 7); i++;
	findAction = XtCreateManagedWidget("findAction",
		xmFormWidgetClass, findPane, args, i);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Forward", char_set)); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNleftPosition, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNrightPosition, (XtArgVal) 2); i++;
	findForwardButton = XtCreateManagedWidget("findForwardButton",
		xmPushButtonWidgetClass, findAction, args, i);
	XtAddCallback(findForwardButton,
		XmNactivateCallback, FindForward, NULL);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Reverse", char_set)); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNleftPosition, (XtArgVal) 3); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNrightPosition, (XtArgVal) 4); i++;
	findReverseButton = XtCreateManagedWidget("findReverseButton",
		xmPushButtonWidgetClass, findAction, args, i);
	XtAddCallback(findReverseButton,
		XmNactivateCallback, FindReverse, NULL);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Dismiss", char_set)); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNleftPosition, (XtArgVal) 5); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNrightPosition, (XtArgVal) 6); i++;
	findDismissButton = XtCreateManagedWidget("findDismissButton",
		xmPushButtonWidgetClass, findAction, args, i);
	XtAddCallback(findDismissButton,
		XmNactivateCallback, FindDismiss, NULL);
	/*
	restrict the height of the action area:
	*/
#ifdef RESTRICT_ACTION_AREA
	i = 0;
	XtSetArg(args[i], XmNmarginHeight, &margin_height); i++;
	XtGetValues(findAction, args, i);
	i = 0;
	XtSetArg(args[i], XmNheight, &height); i++;
	XtGetValues(findDismissButton, args, i);
	i = 0;
	XtSetArg(args[i], XmNpaneMinimum,
		(XtArgVal) (height + (margin_height * 2))); i++;
	XtSetArg(args[i], XmNpaneMaximum,
		(XtArgVal) (height + (margin_height * 2))); i++;
#else
	i = 0;
	XtSetArg(args[i], XmNskipAdjust, (XtArgVal) True); i++;
#endif
	XtSetValues(findAction, args, i);
	remove_sash_traversal(findPane);

	i = 0;
	XtSetArg(args[i], XmNallowShellResize, (XtArgVal) True); i++;
	XtSetArg(args[i], XmNmappedWhenManaged, (XtArgVal) False); i++;
	printShell = XtCreatePopupShell("printShell",
		resource_data->minimal_dialogs ?
			transientShellWidgetClass: topLevelShellWidgetClass,
		parent, args, i);
	i = 0;
	XtSetArg(args[i], XmNsashWidth, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNsashHeight, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNsashIndent, (XtArgVal) -1); i++;
	printPane = XtCreateManagedWidget("printPane",
		xmPanedWindowWidgetClass, printShell, args, i);
	printControl = XtCreateManagedWidget("printControl",
		xmFormWidgetClass, printPane, NULL, 0);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Print command:", char_set)); i++;
	XtSetArg(args[i], XmNalignment, (XtArgVal) XmALIGNMENT_CENTER); i++;
	printLabel = XtCreateManagedWidget("printLabel",
		xmLabelWidgetClass, printControl, args, i);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNtopWidget, (XtArgVal) printLabel); i++;
	XtSetArg(args[i], XmNcolumns, (XtArgVal) 40); i++;
	XtSetArg(args[i], XmNvalue, (XtArgVal) print_command); i++;
	printText = XtCreateManagedWidget("printText",
		xmTextWidgetClass, printControl, args, i);
	XtAddCallback(printText, XmNactivateCallback, PrintCurrent, NULL);
	i = 0;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET); i++;
	XtSetArg(args[i], XmNtopWidget, (XtArgVal) printText); i++;
	XtSetArg(args[i], XmNorientation, (XtArgVal) XmVERTICAL); i++;
	XtSetArg(args[i], XmNindicatorType, (XtArgVal) XmN_OF_MANY); i++;
	printToggleBox = XtCreateManagedWidget("printToggleBox",
		xmRowColumnWidgetClass, printControl, args, i);
	create_menus(printToggleBox, print_toggle_menu, char_set);
	i = 0;
	XtSetArg(args[i], XmNset, (XtArgVal) print_standard_output); i++;
	XtSetValues(print_toggle_menu[0].id, args, i);
	i = 0;
	XtSetArg(args[i], XmNset, (XtArgVal) print_formfeed); i++;
	XtSetValues(print_toggle_menu[1].id, args, i);
	i = 0;
	XtSetArg(args[i], XmNadjustLast, (XtArgVal) False); i++;
	XtSetArg(args[i], XmNfractionBase, 10); i++;
	printAction = XtCreateManagedWidget("printAction",
		xmFormWidgetClass, printPane, args, i);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Print Current", char_set)); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNleftPosition, (XtArgVal) 1); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNrightPosition, (XtArgVal) 3); i++;
/*	XtSetArg(args[i], XmNshowAsDefault, (XtArgVal) True); i++;*/
	printCurrentButton = XtCreateManagedWidget("printCurrentButton",
		xmPushButtonWidgetClass, printAction, args, i);
	XtAddCallback(printCurrentButton,
		XmNactivateCallback, PrintCurrent, NULL);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Print All", char_set)); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNleftPosition, (XtArgVal) 4); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNrightPosition, (XtArgVal) 6); i++;
	printAllButton = XtCreateManagedWidget("printAllButton",
		xmPushButtonWidgetClass, printAction, args, i);
	XtAddCallback(printAllButton,
		XmNactivateCallback, PrintAll, NULL);
	i = 0;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR("Dismiss", char_set)); i++;
	XtSetArg(args[i], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNbottomAttachment, (XtArgVal) XmATTACH_FORM); i++;
	XtSetArg(args[i], XmNleftAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNleftPosition, (XtArgVal) 7); i++;
	XtSetArg(args[i], XmNrightAttachment, (XtArgVal) XmATTACH_POSITION); i++;
	XtSetArg(args[i], XmNrightPosition, (XtArgVal) 9); i++;
	printDismissButton = XtCreateManagedWidget("printDismissButton",
		xmPushButtonWidgetClass, printAction, args, i);
	XtAddCallback(printDismissButton,
		XmNactivateCallback, PrintDismiss, NULL);
	/*
	restrict the height of the action area:
	*/
#ifdef RESTRICT_ACTION_AREA
	i = 0;
	XtSetArg(args[i], XmNmarginHeight, &margin_height); i++;
	XtGetValues(printAction, args, i);
	i = 0;
	XtSetArg(args[i], XmNheight, &height); i++;
	XtGetValues(printDismissButton, args, i);
	i = 0;
	XtSetArg(args[i], XmNpaneMinimum,
		(XtArgVal) (height + (margin_height * 2))); i++;
	XtSetArg(args[i], XmNpaneMaximum,
		(XtArgVal) (height + (margin_height * 2))); i++;
#else
	i = 0;
	XtSetArg(args[i], XmNskipAdjust, (XtArgVal) True); i++;
#endif
	XtSetValues(printAction, args, i);
	remove_sash_traversal(printPane);
}	/* create_dialogs */


/*
Private callback functions:
*/

/*
Close() terminates the application after freeing dynamic
resources and processing unsaved changes.
*/
/*ARGSUSED*/
static void Close(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (check_for_unsaved_changes(exitUnsavedDialog))
		return;
	cleanup_and_exit((Widget) client_data);
}	/* Close */


/*
UnmapWindow() provides "Close" button functionality for pop-ups.
*/
/*ARGSUSED*/
static void UnmapWindow(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtUnmapWidget((Widget) client_data);
}	/* UnmapWindow */


/*
UnmanageWindow() provides "Close" button functionality for Motif dialogs.
*/
/*ARGSUSED*/
static void UnmanageWindow(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtUnmanageChild((Widget) client_data);
}	/* UnmanageWindow */


/*
DialogCancel() provides "Close" button functionality for
`Dialog' dialogs.  Because `xrolodex' does NOT interpret
data associated with any of its dialogs' buttons, it
doesn't matter which button we interpret the "Close"
operation as being equivalent to; thus, the 0th button
is used; see dialog_cancel() in `dialog.c'.
*/
/*ARGSUSED*/
static void DialogCancel(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	dialog_cancel((Dialog) client_data, 0);
}	/* DialogCancel */


/*
About() displays the version number.
*/
/*ARGSUSED*/
static void About(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	dialog_activate(aboutDialog);
}	/* About */


/*
Exit() terminates the application after freeing dynamic
resources.  It first checks with the user via a dialog box.
*/
/*ARGSUSED*/
static void Exit(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (check_for_unsaved_changes(exitUnsavedDialog))
		return;
	cleanup_and_exit(w);
}	/* Exit */


/*
Open() prompts regarding unsaved changes and then
activates the file selection box for loading a rolodex
file.  FileSelectionOK() manages the actual open/load
operation for the rolodex file.
*/
/*ARGSUSED*/
static void Open(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	Arg arg;
	static XtCallbackRec open_cb_list[] = {
		{FileSelectionOK, (XtPointer) xrolo_OPEN},	/* ptr warning */
		{(XtCallbackProc) NULL, (XtPointer) NULL},
	};

	if (check_for_unsaved_changes(openUnsavedDialog))
		return;
	XtSetArg(arg, XmNokCallback, (XtArgVal) open_cb_list);
	XtSetValues(fileSelectionDialog, &arg, 1);
	XtManageChild(fileSelectionDialog);
}	/* Open */


/*
Save() saves changes to the currently open rolodex database.
*/
/*ARGSUSED*/
static void Save(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	save_first_time();
	save_current_text();
	xrolo_db_save(force_save ? xrolo_db_FORCE_SAVE : xrolo_db_MOD_SAVE);
}	/* Save */


/*
SaveAs() activates the file selection box for specifying
a rolodex file.  FileSelectionOK() manages the actual save
operation for the rolodex file.
*/
/*ARGSUSED*/
static void SaveAs(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	Arg arg;
	static XtCallbackRec save_as_cb_list[] = {
		{FileSelectionOK, (XtPointer) xrolo_SAVE_AS},	/* ptr warning */
		{(XtCallbackProc) NULL, (XtPointer) NULL},
	};

	if (!is_rolodex_active())
		return;
	XtSetArg(arg, XmNokCallback, (XtArgVal) save_as_cb_list);
	XtSetValues(fileSelectionDialog, &arg, 1);
	XtManageChild(fileSelectionDialog);
}	/* SaveAs */


/*
EntryModified() signals modifications to the rolodex entry viewport.
It is registered with the editor pseudo-widget's modify callback.
*/
/*ARGSUSED*/
static void EntryModified(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XmTextVerifyCallbackStruct *cbs = (XmTextVerifyCallbackStruct *) call_data;

	if (!is_rolodex_active()) {
		if (cbs->reason == XmCR_MODIFYING_TEXT_VALUE)
			cbs->doit = False;
		return;
	}
	entry_modified = TRUE;
}	/* EntryModified */


/*
SensitiveSearch() signals modifications to the rolodex entry
search case sensitivity.  It is registered with the toggle
button's value-changed callback.
*/
/*ARGSUSED*/
static void SensitiveSearch(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XmToggleButtonCallbackStruct *toggle =
		(XmToggleButtonCallbackStruct *) call_data;

	case_sensitive_search = (toggle->set) ? TRUE : FALSE; /* force T/F */
}	/* SensitiveSearch */


/*
WraparoundSearch() signals modifications to the rolodex entry
search behavior.  It is registered with the toggle button's
value-changed callback.
*/
/*ARGSUSED*/
static void WraparoundSearch(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XmToggleButtonCallbackStruct *toggle =
		(XmToggleButtonCallbackStruct *) call_data;

	wraparound_search = (toggle->set) ? TRUE : FALSE; /* force T/F */
}	/* WraparoundSearch */


/*
Print() activates the "Print" dialog.
*/
/*ARGSUSED*/
static void Print(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!XtIsRealized(printShell))
		return;
	XtMapWidget(printShell);
}	/* Print */


/*
PrintCurrent() print the current entry.
*/
/*ARGSUSED*/
static void PrintCurrent(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	print_entry_or_file(xrolo_db_PRINT_CURRENT);
}	/* PrintCurrent */


/*
PrintAll() print the entire file.
*/
/*ARGSUSED*/
static void PrintAll(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	print_entry_or_file(xrolo_db_PRINT_ALL);
}	/* PrintAll */


/*
PrintDismiss() simply removes the "Print" dialog.
*/
/*ARGSUSED*/
static void PrintDismiss(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtUnmapWidget(printShell);
}	/* PrintDismiss */


/*
PrintStandardOutput() signals modifications to the print options.
It is registered with the toggle button's value-changed callback.
*/
/*ARGSUSED*/
static void PrintStandardOutput(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XmToggleButtonCallbackStruct *toggle =
		(XmToggleButtonCallbackStruct *) call_data;

	print_standard_output = (toggle->set) ? TRUE : FALSE; /* force T/F */
	XtSetSensitive(printText, !print_standard_output);
}	/* PrintStandardOutput */


/*
PrintFormfeed() signals modifications to the print options.
It is registered with the toggle button's value-changed callback.
*/
/*ARGSUSED*/
static void PrintFormfeed(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XmToggleButtonCallbackStruct *toggle =
		(XmToggleButtonCallbackStruct *) call_data;

	print_formfeed = (toggle->set) ? TRUE : FALSE; /* force T/F */
}	/* PrintFormfeed */


/*
FileSelectionOK() responds to a user's file selection.  It
must handle rolodex file "Open" and "SaveAs" operations.
*/
/*ARGSUSED*/
static void FileSelectionOK(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	Arg args[5];
	int i;
	char *filename;
	XmString xmstring;

	i = 0;
	XtSetArg(args[i], XmNtextString, &xmstring); i++;
	XtGetValues(fileSelectionDialog, args, i);
	if (!XmStringGetLtoR(xmstring, char_set, &filename))
		return;
	XtUnmanageChild(fileSelectionDialog);
	if ((long int) client_data == xrolo_OPEN)
		open_rolodex_file(filename);
	else if ((long int) client_data == xrolo_SAVE_AS) {
		/*
		save_as_rolodex_file() *may* activate a nonmodal dialog box
		and return with it still pending, so create a copy of the local
		filename before freeing it.  Since callbacks are involved, there
		is no point in passing it as arguments to procedures and setting
		(and resetting) callback client-data values:
		*/
		if (save_as_filename)
			XtFree(save_as_filename);
		if ((save_as_filename =
		(char *) XtNewString((String) filename)) != NULL) {
			save_as_rolodex_file();
		}
	}
	XmStringFree(xmstring);
	XtFree(filename);
}	/* FileSelectionOK */


/*
FileSelectionCancel() simply unmaps the file selection box.
*/
/*ARGSUSED*/
static void FileSelectionCancel(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtUnmanageChild(fileSelectionDialog);
}	/* FileSelectionCancel */


/*
SaveUnsavedChanges() handles the appropriate action if a user
chooses the "OK" button, depending on application context.
*/
/*ARGSUSED*/
static void SaveUnsavedChanges(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	save_first_time();
	save_current_text();
	xrolo_db_save(xrolo_db_MOD_SAVE);
	if ((long int) client_data == xrolo_EXIT_WARNING) {
		dialog_deactivate(exitUnsavedDialog);
		cleanup_and_exit(w);
	}
	else if ((long int) client_data == xrolo_OPEN_WARNING) {
		Arg arg;
		static XtCallbackRec open_cb_list[] = {
			{FileSelectionOK, (XtPointer) xrolo_OPEN},	/* ptr warning */
			{(XtCallbackProc) NULL, (XtPointer) NULL},
		};

		dialog_deactivate(openUnsavedDialog);
		XtSetArg(arg, XmNokCallback, (XtArgVal) open_cb_list);
		XtSetValues(fileSelectionDialog, &arg, 1);
		XtManageChild(fileSelectionDialog);
	}
}	/* SaveUnsavedChanges */


/*
DiscardUnsavedChanges() handles the appropriate action if a user
chooses the "Discard" button, depending on application context.
*/
/*ARGSUSED*/
static void DiscardUnsavedChanges(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if ((long int) client_data == xrolo_EXIT_WARNING) {
		dialog_deactivate(exitUnsavedDialog);
		cleanup_and_exit(w);
	}
	else if ((long int) client_data == xrolo_OPEN_WARNING) {
		Arg arg;
		static XtCallbackRec open_cb_list[] = {
			{FileSelectionOK, (XtPointer) xrolo_OPEN},	/* ptr warning */
			{(XtCallbackProc) NULL, (XtPointer) NULL},
		};

		dialog_deactivate(openUnsavedDialog);
		XtSetArg(arg, XmNokCallback, (XtArgVal) open_cb_list);
		XtSetValues(fileSelectionDialog, &arg, 1);
		XtManageChild(fileSelectionDialog);
	}
}	/* DiscardUnsavedChanges */


/*
CancelUnsavedChanges() handles the appropriate action if a user
chooses the "Cancel" button, depending on application context.
*/
/*ARGSUSED*/
static void CancelUnsavedChanges(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if ((long int) client_data == xrolo_EXIT_WARNING)
		dialog_deactivate(exitUnsavedDialog);
	else if ((long int) client_data == xrolo_OPEN_WARNING)
		dialog_deactivate(openUnsavedDialog);
}	/* CancelUnsavedChanges */


/*
OverwriteOK() handles the appropriate action if a user
chooses the "OK" button during a "Save As" operation
that leads to the file-overwrite warning.
*/
/*ARGSUSED*/
static void OverwriteOK(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	do_save_as_operation();
}	/* OverwriteOK */


/*
OverwriteCancel() handles the appropriate action if a user
chooses the "Cancel" button during a "Save As" operation
that leads to the file-overwrite warning.
*/
/*ARGSUSED*/
static void OverwriteCancel(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	/* currently, nothing */
}	/* OverwriteCancel */


/*
FindForward() handles the search in the forward direction for an
entry containing the specified text.
*/
/*ARGSUSED*/
static void FindForward(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	find_entry_containing_text_forward();
}	/* FindForward */


/*
FindReverse() handles the search in the reverse direction for an
entry containing the specified text.
*/
/*ARGSUSED*/
static void FindReverse(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	find_entry_containing_text_reverse();
}	/* FindReverse */


/*
FindDismiss() simply removes the "Find" dialog.
*/
/*ARGSUSED*/
static void FindDismiss(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	XtUnmapWidget(findShell);
}	/* FindDismiss */


/*
First() advances to the first entry in the rolodex database.
*/
/*ARGSUSED*/
static void First(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	save_current_text();
	if (xrolo_db_current_entry_is_first_entry())
		return;
	if ((temp_entry = xrolo_db_first_entry()) != NULL)
		update_current_entry(temp_entry);
}	/* First */


/*
Last() advances to the last entry in the rolodex database.
*/
/*ARGSUSED*/
static void Last(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	save_current_text();
	if (xrolo_db_current_entry_is_last_entry())
		return;
	if ((temp_entry = xrolo_db_last_entry()) != NULL)
		update_current_entry(temp_entry);
}	/* Last */


/*
Next() advances to the next entry in the rolodex database.
*/
/*ARGSUSED*/
static void Next(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	load_next_rolodex_entry();
}	/* Next */


/*
Previous() advances to the previous entry in the rolodex database.
*/
/*ARGSUSED*/
static void Previous(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	load_previous_rolodex_entry();
}	/* Previous */


/*
Index() invokes the rolodex index system for selecting a
specific rolodex entry.
*/
/*ARGSUSED*/
static void Index(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	save_current_text();
	if (!xrolo_index_build(char_set, index_row))
		return;
	listShell_activate(entryIndex);
}	/* Index */


/*
Find() locates the next occurence of the specified text
across rolodex entries, beginning with the next entry.
*/
/*ARGSUSED*/
static void Find(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!XtIsRealized(findShell))
		return;
	XtMapWidget(findShell);
}	/* Find */


/*
Insert() clears the rolodex entry area for new text.
*/
/*ARGSUSED*/
static void Insert(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	new_entry(xrolo_INSERT);
}	/* Insert */


/*
InsertAfter() clears the rolodex entry area for new text.
*/
/*ARGSUSED*/
static void InsertAfter(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	new_entry(xrolo_ADD);
}	/* InsertAfter */


/*
Copy() inserts a new rolodex entry into the rolodex file.
*/
/*ARGSUSED*/
static void Copy(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	if (!current_entry) {
		user_message("Nothing to copy.");
		return;
	}
	save_current_text();
	user_message("Inserting a duplicate entry.");
	insert_new_db_entry();
	if (listShell_is_active(entryIndex) &&
	!xrolo_index_build(char_set, index_row))
		user_message("Can't (re)build index.");
}	/* Copy */


/*
Delete() deletes the current rolodex entry from the rolodex file.
*/
/*ARGSUSED*/
static void Delete(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
/*	listShell_deactivate(entryIndex);*/
	if (!index_stay_up)
		listShell_deactivate(entryIndex);
	if (editing_new_entry) {
		if (!current_entry)
			editor_set_text(entryEditWindow, "");
		else {
			current_entry = xrolo_db_current_entry();
			editor_set_text(entryEditWindow, xrolo_db_get_text(current_entry));
		}
		editing_new_entry = entry_modified = FALSE;
	}
	else {
		/*
		it's possible to delete all entries:
		*/
		if ((current_entry = xrolo_db_delete_current_entry()) == NULL)
			editor_set_text(entryEditWindow, "");
		else
			editor_set_text(entryEditWindow, xrolo_db_get_text(current_entry));
		entry_modified = FALSE;
	}
	if (listShell_is_active(entryIndex) &&
	!xrolo_index_build(char_set, index_row))
		user_message("Can't (re)build index.");
}	/* Delete */


/*
Undelete() recovers the most recent rolodex entry deletion.
*/
/*ARGSUSED*/
static void Undelete(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	save_current_text();
	if ((temp_entry = xrolo_db_recover_deleted_entry()) != NULL)
		update_current_entry(temp_entry);
}	/* Undelete */


/*
IndexSelection() chooses a new current rolodex entry from
the rolodex file based on the user's selection in the item
selection box.
*/
/*ARGSUSED*/
static void IndexSelection(w, client_data, call_data)
Widget w;
XtPointer client_data;
XmListCallbackStruct *call_data;
{
	save_current_text();
	temp_entry = (!xrolo_db_current_entry_is_first_entry()) ?
		xrolo_db_first_entry() : current_entry;
	if (!temp_entry)
		return;
	if ((temp_entry = xrolo_db_nth_entry(call_data->item_position - 1)) != NULL)
		update_current_entry(temp_entry);
	else {
		if ((temp_entry = xrolo_db_first_entry()) != NULL)
			update_current_entry(temp_entry);
	}
}	/* IndexSelection */


/*
Sort() sorts the rolodex database entries.
*/
/*ARGSUSED*/
static void Sort(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	if (!is_rolodex_active())
		return;
	if (!current_entry) {
		user_message("Nothing to sort.");
		return;
	}
	save_current_text();
	temp_entry = ((long int) client_data == xrolo_ASCEND) ?
		xrolo_db_sort_ascending(sort_row) : xrolo_db_sort_descending(sort_row);
	if (temp_entry)
		update_current_entry(temp_entry);
	if (listShell_is_active(entryIndex) &&
	!xrolo_index_build(char_set, index_row))
		user_message("Can't (re)build index.");
}	/* Sort */


/*
Delimiter() allows the user to change the card delimiter dynamically.
*/
/*ARGSUSED*/
static void Delimiter(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	dialog_activate(delimiterDialog);
}	/* Delimiter */


/*
DelimiterOK() handles the appropriate action if a user chooses
the "OK" button during a request to change the delimiter.  The
text field version of the delimiter is interpreted with
convert_delimiter_string() to process any special format characters
such as \n, \t, and so on.  Note that the interpreted text should
have fewer characters than the original text.
*/
/*ARGSUSED*/
static void DelimiterOK(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	static char *new_delimiter = NULL;
	char *text_delimiter;

	text_delimiter = dialog_get_text(delimiterDialog);
	if (text_delimiter && *text_delimiter) {
		if (new_delimiter)
			free(new_delimiter);	/* free any existing storage */
		if ((new_delimiter = 
		(char *) malloc((unsigned) (strlen(text_delimiter) + 1))) == NULL) {
			fprintf(stderr, "xrolodex: can't allocate memory for delimiter!\n");
			user_message("Can't allocate memory for delimiter!");
			XtFree(text_delimiter);
			return;
		}
		if (!convert_delimiter_string(new_delimiter, text_delimiter)) {
			user_message("Error(s) in delimiter.");
		}
		delimiter = new_delimiter;	/* point global delimiter at local storage */
		xrolo_db_set_delimiter(delimiter);
	}
	XtFree(text_delimiter);	/* free dynamic XmText-created storage */
}	/* DelimiterOK */


/*
DelimiterCancel() handles the appropriate action if a user
chooses the "Cancel" button during a request to change the
delimiter.
*/
/*ARGSUSED*/
static void DelimiterCancel(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	/* currently, nothing */
}	/* DelimiterCancel */


/*
DelimiterReset() handles the appropriate action if a user chooses
to reset the delimiter to the start-up value.
*/
/*ARGSUSED*/
static void DelimiterReset(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	delimiter = start_up_delimiter;
	xrolo_db_set_delimiter(delimiter);
}	/* DelimiterReset */


/*
SetSortRow() allows the user to change the sort line/row.
*/
/*ARGSUSED*/
static void SetSortRow(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	dialog_activate(sortDialog);
}	/* SetSortRow */


/*
SetSortRowOK() handles the appropriate action if a user chooses
the "OK" button during a request to change the sort row.
*/
/*ARGSUSED*/
static void SetSortRowOK(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	int new_sort_row;
	char *text_sort_row;

	text_sort_row = dialog_get_text(sortDialog);
	if (text_sort_row && *text_sort_row ) {
		new_sort_row = atoi(text_sort_row);
		if (new_sort_row > 0)
			sort_row = new_sort_row;
		else
			user_message("Sort line/row must be one or greater.");
	}
	XtFree(text_sort_row);	/* free dynamic XmText-created storage */
}	/* SetSortRowOK */


/*
SetSortRowCancel() handles the appropriate action if a user
chooses the "Cancel" button during a request to change the
sort row.
*/
/*ARGSUSED*/
static void SetSortRowCancel(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	/* currently, nothing */
}	/* SetSortRowCancel */


/*
SetSortRowReset() handles the appropriate action if a user chooses
to reset the sort row to the start-up value.
*/
/*ARGSUSED*/
static void SetSortRowReset(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	sort_row = start_up_sort_row;
}	/* SetSortRowReset */


/*
SetIndexRow() allows the user to change the index line/row.
*/
/*ARGSUSED*/
static void SetIndexRow(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	dialog_activate(indexDialog);
}	/* SetIndexRow */


/*
SetIndexRowOK() handles the appropriate action if a user chooses
the "OK" button during a request to change the index row.
*/
/*ARGSUSED*/
static void SetIndexRowOK(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	int new_index_row;
	char *text_index_row;

	text_index_row = dialog_get_text(indexDialog);
	if (text_index_row && *text_index_row ) {
		new_index_row = atoi(text_index_row);
		if (new_index_row > 0)
			index_row = new_index_row;
		else
			user_message("Index line/row must be one or greater.");
	}
	XtFree(text_index_row);	/* free dynamic XmText-created storage */
	if (listShell_is_active(entryIndex) &&
	!xrolo_index_build(char_set, index_row))
		user_message("Can't (re)build index.");
}	/* SetIndexRowOK */


/*
SetIndexRowCancel() handles the appropriate action if a user
chooses the "Cancel" button during a request to change the
index row.
*/
/*ARGSUSED*/
static void SetIndexRowCancel(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	/* currently, nothing */
}	/* SetIndexRowCancel */


/*
SetIndexRowReset() handles the appropriate action if a user chooses
to reset the index row to the start-up value.
*/
/*ARGSUSED*/
static void SetIndexRowReset(w, client_data, call_data)
Widget w;
XtPointer client_data;
XtPointer call_data;
{
	index_row = start_up_index_row;
	if (listShell_is_active(entryIndex) &&
	!xrolo_index_build(char_set, index_row))
		user_message("Can't (re)build index.");
}	/* SetIndexRowReset */


/*
Private support functions:
*/

/*
check_for_unsaved_changes() raises the dialog box that signals
unsaved changes, if there are pending modifications to the
rolodex file.
*/

static int check_for_unsaved_changes(dialog_box)
Dialog dialog_box;
{
	if (!entry_modified && !xrolo_db_is_modified())
		return FALSE;
	dialog_activate(dialog_box);
	return TRUE;
}	/* check_for_unsaved_changes */


/*
load_next_rolodex_entry() simply requests that the next rolodex
entry be loaded, checking for several special conditions.
*/

static void load_next_rolodex_entry()
{
	save_current_text();
	if (xrolo_db_current_entry_is_last_entry())
		return;
	if ((temp_entry = xrolo_db_next_entry()) != NULL)
		update_current_entry(temp_entry);
}	/* load_next_rolodex_entry */


/*
load_previous_rolodex_entry() simply requests that the previous
rolodex entry be loaded, checking for several special conditions.
*/

static void load_previous_rolodex_entry()
{
	save_current_text();
	if (xrolo_db_current_entry_is_first_entry())
		return;
	if ((temp_entry = xrolo_db_previous_entry()) != NULL)
		update_current_entry(temp_entry);
}	/* load_previous_rolodex_entry */


/*
new_entry() clears the rolodex viewport and sets the
insertion point.
*/

static void new_entry(operation)
int operation;
{
	if (!entry_modified && editing_new_entry) {
		user_message("Already editing new entry.");
		return;
	}
	save_current_text();
	editing_new_entry = TRUE;
	editor_set_text(entryEditWindow, "");
	entry_modified = FALSE;	/* saved it before setting text */
	/*
	want to add the new entry after current entry?
	*/
	if (operation == xrolo_ADD) {
		/*
		if already at the end, point to end of the database:
		*/
		if ((temp_entry = xrolo_db_next_entry()) == NULL) {
			current_entry = xrolo_db_past_last_entry();
		}
		else
			current_entry = temp_entry;
	}
}	/* new_entry */


/*
save_current_text() handles both (1) text entered in the
viewport after pressing "Insert", etc., and (2) changes
to the current/existing entry.  It is called by every
module that initiates a high-level operation.  It imposes
the following implicit policy:  If the user has pressed
"Insert", but doesn't enter any text, an empty entry is
NOT added to the database.
*/

static int save_current_text()
{
/*	listShell_deactivate(entryIndex);*/	/* must be rebuilt */
	if (!index_stay_up)
		listShell_deactivate(entryIndex);
	if (!entry_modified) {
		editing_new_entry = FALSE; /* may have been set, but no text entered */
		return FALSE;
	}
	if (editing_new_entry || !current_entry)
		insert_new_db_entry();
	else
		save_current_db_entry();
	editing_new_entry = FALSE;
	if (listShell_is_active(entryIndex) &&
	!xrolo_index_build(char_set, index_row))
		user_message("Can't (re)build index.");
	return TRUE;
}	/* save_current_text */


/*
insert_new_db_entry() inserts the text from the viewport into the
rolodex file WITHOUT updating the entry viewport--useful after
adding new text with "New".
*/

static void insert_new_db_entry()
{
	if ((temp_entry =
	xrolo_db_insert_new_entry(editor_get_text(entryEditWindow))) == NULL) {
		fprintf(stderr, "xrolodex: can't insert new entry!\n");
		user_message("Can't insert new entry!");
	}
	else {
		current_entry = temp_entry;
		entry_modified = FALSE;
	}
	editing_new_entry = FALSE;
}	/* insert_new_db_entry */


/*
save_current_db_entry() saves the current viewport text by deleting
the current entry from the rolodex file and then inserting the text
from the viewport.  xrolo_db_delete_current_entry_no_undo() does NOT
affect the deletion history.
*/

static void save_current_db_entry()
{
	current_entry = xrolo_db_delete_current_entry_no_undo();
	insert_new_db_entry();
}	/* save_current_db_entry */


/*
find_entry_containing_text_forward() searches forward through the
rolodex entries for the first entry containing the specified text.
*/

static void find_entry_containing_text_forward()
{
	char *search_text = XmTextGetString(findText);

	save_current_text();
	if (!wraparound_search && xrolo_db_current_entry_is_last_entry()) {
		user_message("Search forward:  currently at last entry.");
		XtFree(search_text);
		return;
	}
	if ((temp_entry =
	xrolo_db_find_entry_forward(search_text, case_sensitive_search,
	wraparound_search)) != NULL)
		update_current_entry(temp_entry);
	else
		user_message("Search forward:  no match.");
	XtFree(search_text);
}	/* find_entry_containing_text_forward */


/*
find_entry_containing_text_reverse() searches backward through the
rolodex entries for the first entry containing the specified text.
*/

static void find_entry_containing_text_reverse()
{
	char *search_text = XmTextGetString(findText);

	save_current_text();
	if (!wraparound_search && xrolo_db_current_entry_is_first_entry()) {
		user_message("Search backward:  currently at first entry.");
		XtFree(search_text);
		return;
	}
	if ((temp_entry =
	xrolo_db_find_entry_reverse(search_text, case_sensitive_search,
	wraparound_search)) != NULL)
		update_current_entry(temp_entry);
	else
		user_message("Search backward:  no match.");
	XtFree(search_text);
}	/* find_entry_containing_text_reverse */


/*
print_entry_or_file() handles printing operations.
*/

static void print_entry_or_file(operation)
int operation;
{
	static char *text = NULL;
	char *command, *spacing = (print_formfeed) ? "\n\f" : "\n";

	XtFree(text); /* free existing text, if any */
	text = XmTextGetString(printText);
	command = (*text) ? text : print_command;
	switch (operation) {
		case xrolo_db_PRINT_CURRENT:
			operation = (print_standard_output) ?
				xrolo_db_PRINT_SO : xrolo_db_PRINT_PTR;
			if (xrolo_db_print(operation, command, spacing) != TRUE)
				user_message("Print current entry:  unknown printing error.");
			break;
		case xrolo_db_PRINT_ALL:
			operation = (print_standard_output) ?
				xrolo_db_PRINT_ALL_SO : xrolo_db_PRINT_ALL_PTR;
			if (xrolo_db_print(operation, command, spacing) != TRUE)
				user_message("Print all entries:  unknown printing error.");
			break;
		default:
			break;
	}
}	/* print_entry_or_file */


/*
open_rolodex_file() opens the file and loads the first entry if
the file exists; otherwise, it clears the current rolodex display.
*/

static void open_rolodex_file(filename)
char *filename;
{
	Arg args[3];
	int i;

	initialize_critical_variables();
	i = 0;
	XtSetArg(args[i], XmNalignment, (XtArgVal) XmALIGNMENT_CENTER); i++;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR(filename, char_set)); i++;
	XtSetValues(databaseLabel, args, i);
	current_entry = xrolo_db_create(filename, delimiter);
	if (current_entry)
		editor_set_text(entryEditWindow, xrolo_db_get_text(current_entry));
	else
		editor_set_text(entryEditWindow, "");	/* file doesn't yet exist */
	entry_modified = FALSE;
	if (listShell_is_active(entryIndex) &&
	!xrolo_index_build(char_set, index_row))
		user_message("Can't (re)build index.");
}	/* open_rolodex_file */


/*
save_as_rolodex_file() manages the save-as process for saving
the rolodex file under a new filename.
*/
static void save_as_rolodex_file()
{
	if (file_exists(save_as_filename))
		dialog_activate(overwriteDialog);
	else
		do_save_as_operation();
}	/* save_as_rolodex_file */


/*
do_save_as_operation() completes the save-as task, if there
is no file-write error, and updates the filename label.
*/

static void do_save_as_operation()
{
	Arg args[3];
	int i;

	if (!xrolo_db_save_as(save_as_filename)) {
		user_message("Can't save file.");
		return;
	}
	temp_entry = current_entry;
	initialize_critical_variables();	/* except for `current_entry' */
	current_entry = temp_entry;
	i = 0;
	XtSetArg(args[i], XmNalignment, (XtArgVal) XmALIGNMENT_CENTER); i++;
	XtSetArg(args[i], XmNlabelString,
		XmStringCreateLtoR(save_as_filename, char_set)); i++;
	XtSetValues(databaseLabel, args, i);
}	/* do_save_as_operation */


/*
save_first_time() is called before updating
the rolodex file to ensure that a back-up
is created.
*/

static void save_first_time()
{
	if (first_time) {
		first_time = FALSE;
		xrolo_db_save_backup();
	}
}	/* save_first_time */


/*
update_current_entry() sets up the new current entry.
*/

static void update_current_entry(new_entry)
EntryDB new_entry;
{
	current_entry = new_entry;
	editor_set_text(entryEditWindow, xrolo_db_get_text(current_entry));
	entry_modified = FALSE;	/* saved it before setting text */
}	/* update_current_entry */


/*
is_rolodex_active() is called before every operation that leads
to rolodex modification.
*/

static int is_rolodex_active()
{
	if (!xrolo_db_is_active()) {
		user_message(
"First, use \"New\"/\"Open\" to establish a (new or existing) rolodex file.");
		return FALSE;
	}
	return TRUE;
}	/* is_rolodex_active */


/*
initialize_critical_variables() is called before opening
a new rolodex file.
*/

static void initialize_critical_variables()
{
	editing_new_entry = FALSE;
	current_entry = NULL;
	entry_modified = FALSE;
	first_time = TRUE;
/*	listShell_deactivate(entryIndex);*/
	if (!index_stay_up)
		listShell_deactivate(entryIndex);
}	/* initialize_critical_variables */


/*
cleanup_and_exit() is called by several functions
when the application terminates.
*/

static void cleanup_and_exit(w)
Widget w;
{
	if (save_as_filename)
		XtFree(save_as_filename);
	xrolo_index_destroy();
	xrolo_db_destroy();
	ctrlPanel_destroy(entryPanel);
	editor_destroy(entryEditWindow);
	listShell_destroy(entryIndex);
	XFreePixmap(XtDisplay(w), icon_pixmap);
	exit(0);
}	/* cleanup_and_exit */


/*
user_message() sets the message dialog box to
have the specified message and then maps
the dialog box.
*/

static void user_message(msg)
char *msg;
{
	dialog_set_prompt(messageDialog, msg);
	dialog_activate(messageDialog);
}	/* user_message */


/*
initialize_shell_icons() sets the icon for top-level windows.
*/

static void initialize_shell_icons(w, icon_name)
Widget w;
char *icon_name;
{

#include "xrolo.icon"

	Arg args[3];
	int num_args = 1;

	icon_pixmap = XCreateBitmapFromData(XtDisplay(w),
		RootWindowOfScreen(XtScreen(w)), xrolo_bits,
		xrolo_width, xrolo_height);
	XtSetArg(args[0], XtNiconPixmap, (XtArgVal) icon_pixmap);
	if (icon_name) {
		XtSetArg(args[1], XtNiconName, (XtArgVal) icon_name);
		num_args++;
	}
	XtSetValues(w, args, num_args);
	XtSetValues(findShell, args, num_args);
	XtSetValues(printShell, args, num_args);
	XtSetValues(help_shell(), args, num_args);
	XtSetValues(listShell_listShell(entryIndex), args, num_args);
	XtSetValues(editor_replaceShell(entryEditWindow), args, num_args);
	XtSetValues(dialog_dialogShell(aboutDialog), args, num_args);
	XtSetValues(dialog_dialogShell(messageDialog), args, num_args);
	XtSetValues(dialog_dialogShell(overwriteDialog), args, num_args);
	XtSetValues(dialog_dialogShell(delimiterDialog), args, num_args);
	XtSetValues(dialog_dialogShell(sortDialog), args, num_args);
	XtSetValues(dialog_dialogShell(indexDialog), args, num_args);
	XtSetValues(dialog_dialogShell(openUnsavedDialog), args, num_args);
	XtSetValues(dialog_dialogShell(exitUnsavedDialog), args, num_args);
/*	XtSetValues(XtParent(fileSelectionDialog), args, num_args);*/
}	/* initialize_shell_icons */


/*
handle_close_buttons() accommodates the "Close" buttons.  The actions
should be hardcoded -- the user should not be allowed to override the
XmUNMAP default for pop-ups.
*/

static void handle_close_buttons(w)
Widget w;
{
	Atom xrolo_DELETE_WINDOW;
	Arg arg;

	xrolo_DELETE_WINDOW = XmInternAtom(XtDisplay(w),
		"WM_DELETE_WINDOW", False);
	XtSetArg(arg, XmNdeleteResponse, (XtArgVal) XmDO_NOTHING);
	XtSetValues(w, &arg, 1);
	XtSetValues(findShell, &arg, 1);
	XtSetValues(printShell, &arg, 1);
	XtSetValues(help_shell(), &arg, 1);
	XtSetValues(listShell_listShell(entryIndex), &arg, 1);
	XtSetValues(editor_replaceShell(entryEditWindow), &arg, 1);
	XtSetValues(dialog_dialogShell(aboutDialog), &arg, 1);
	XtSetValues(dialog_dialogShell(messageDialog), &arg, 1);
	XtSetValues(dialog_dialogShell(overwriteDialog), &arg, 1);
	XtSetValues(dialog_dialogShell(delimiterDialog), &arg, 1);
	XtSetValues(dialog_dialogShell(sortDialog), &arg, 1);
	XtSetValues(dialog_dialogShell(indexDialog), &arg, 1);
	XtSetValues(dialog_dialogShell(openUnsavedDialog), &arg, 1);
	XtSetValues(dialog_dialogShell(exitUnsavedDialog), &arg, 1);
	XtSetValues(XtParent(fileSelectionDialog), &arg, 1);
	XmAddWMProtocolCallback(w, xrolo_DELETE_WINDOW,
		Close, (caddr_t) w);
	XmAddWMProtocolCallback(findShell, xrolo_DELETE_WINDOW,
		UnmapWindow, (caddr_t) findShell);
	XmAddWMProtocolCallback(printShell, xrolo_DELETE_WINDOW,
		UnmapWindow, (caddr_t) printShell);
	XmAddWMProtocolCallback(help_shell(), xrolo_DELETE_WINDOW,
		UnmapWindow, (caddr_t) help_shell());
	XmAddWMProtocolCallback(listShell_listShell(entryIndex),
		xrolo_DELETE_WINDOW, UnmapWindow,
		(caddr_t) listShell_listShell(entryIndex));
	XmAddWMProtocolCallback(editor_replaceShell(entryEditWindow),
		xrolo_DELETE_WINDOW, UnmapWindow,
		(caddr_t) editor_replaceShell(entryEditWindow));
	XmAddWMProtocolCallback(dialog_dialogShell(aboutDialog),
		xrolo_DELETE_WINDOW, DialogCancel, (caddr_t) aboutDialog);
	XmAddWMProtocolCallback(dialog_dialogShell(messageDialog),
		xrolo_DELETE_WINDOW, DialogCancel, (caddr_t) messageDialog);
	XmAddWMProtocolCallback(dialog_dialogShell(overwriteDialog),
		xrolo_DELETE_WINDOW, DialogCancel, (caddr_t) overwriteDialog);
	XmAddWMProtocolCallback(dialog_dialogShell(delimiterDialog),
		xrolo_DELETE_WINDOW, DialogCancel, (caddr_t) delimiterDialog);
	XmAddWMProtocolCallback(dialog_dialogShell(sortDialog),
		xrolo_DELETE_WINDOW, DialogCancel, (caddr_t) sortDialog);
	XmAddWMProtocolCallback(dialog_dialogShell(indexDialog),
		xrolo_DELETE_WINDOW, DialogCancel, (caddr_t) indexDialog);
	XmAddWMProtocolCallback(dialog_dialogShell(openUnsavedDialog),
		xrolo_DELETE_WINDOW, DialogCancel, (caddr_t) openUnsavedDialog);
	XmAddWMProtocolCallback(dialog_dialogShell(exitUnsavedDialog),
		xrolo_DELETE_WINDOW, DialogCancel, (caddr_t) exitUnsavedDialog);
	XmAddWMProtocolCallback(XtParent(fileSelectionDialog), xrolo_DELETE_WINDOW,
		UnmanageWindow, (caddr_t) fileSelectionDialog);
#ifdef EDITRES
    XtAddEventHandler(w, (EventMask) 0, True, _XEditResCheckMessages, NULL);
    XtAddEventHandler(findShell, (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(printShell, (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(help_shell(), (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(listShell_listShell(entryIndex), (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(editor_replaceShell(entryEditWindow), (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(dialog_dialogShell(aboutDialog), (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(dialog_dialogShell(messageDialog), (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(dialog_dialogShell(overwriteDialog), (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(dialog_dialogShell(delimiterDialog), (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(dialog_dialogShell(sortDialog), (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(dialog_dialogShell(indexDialog), (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(dialog_dialogShell(openUnsavedDialog), (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(dialog_dialogShell(exitUnsavedDialog), (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
    XtAddEventHandler(XtParent(fileSelectionDialog), (EventMask) 0, True,
    	_XEditResCheckMessages, NULL);
#endif
}	/* handle_close_buttons */


/*
convert_delimiter_string() interprets C-like constants
in the delimiter sequence.
*/

static int convert_delimiter_string(out, in)
char *out, *in;
{
	int i, j;

	for (i = 0, j = 0; in[i]; i++, j++) {
		switch (in[i]) {
			case '\\':
				switch (in[i + 1]) {
					case 'n':
						out[j] = '\n';
						break;
					case 't':
						out[j] = '\t';
						break;
					case 'v':
						out[j] = '\v';
						break;
					case 'b':
						out[j] = '\b';
						break;
					case 'r':
						out[j] = '\r';
						break;
					case 'f':
						out[j] = '\f';
						break;
					case '\\':
						out[j] = '\\';
						break;
					case '\'':
						out[j] = '\'';
						break;
					case '\"':
						out[j] = '\"';
						break;
					default:
						/* no error detection */
						if (isdigit(in[i + 1])) {
							char oct_str[4];
							int oct_value;

							oct_str[0] = in[i + 1];
							oct_str[1] = in[i + 2];
							oct_str[2] = in[i + 3];
							oct_str[3] = '\0';
							sscanf(oct_str, "%o", &oct_value);
							out[j] = (char) oct_value;
							i += 2; /* skip two *extra* chars for octal */
						}
						else {
							out[0] = '\0';
							return FALSE;
						}
						break;
				}
				i++; /* skip one *extra* char for backslash */
				break;
			default:
				out[j] = in[i]; /* no special interpretation */
				break;
		}
	}
	out[j] = '\0';
	return TRUE;
}	/* convert_delimiter_string */


/*
app_error_handler() is a do-nothing alternative to Xt's default
error handler, which terminates the application.
*/

#ifdef ALT_ERROR_HANDLER
static void app_error_handler()
{
	/* do nothing*/
}	/* app_error_handler */
#else
	/* add nothing */
#endif

