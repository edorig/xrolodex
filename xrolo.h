/**** xrolo.h ****/  /**** formatted with 4-column tabs ****/


/********************************************************************
`xrolo' implements a simple rolodex application.  It is composed
of the following modules:
	`xrolo.c'		-- widget creation for the top-level interface,
					   plus callbacks and support functions.
	`xrolo.db.c'	-- rolodex entry manipulation functions and
					   the state of the database.
	`xrolo.help.t'	-- a module for providing help system text.
	`xrolo.index.c'	-- the rolodex index manipulation functions.

	`streamdb.c'	-- a stream- or byte-oriented database object.
	`strstr.c'		-- a string search module.
	`ctrlpanel.c'	-- a control panel pseudo-widget.
	`editor.c'		-- a self-contained editor pseudo-widget.
	`listshell.c'	-- a list selection box pseudo-widget.
	`help.c'		-- a generalized help system.
	`motif.c'		-- OSF/Motif miscellaneous functions.

See these modules for more details.
********************************************************************/

#include <X11/StringDefs.h>

#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/MainW.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Protocols.h>
#include <Xm/FileSB.h>

#include "ctrlpanel.h"
#include "listshell.h"
#include "editor.h"
#include "dialog.h"
#include "help.h"
#include "xrolo.db.h"
#include "xrolo.index.h"
#include "version.h"


/*
`xrolo' constants:
*/

#define xrolo_EXIT_WARNING			-1001
#define xrolo_OPEN_WARNING			-1002

#define xrolo_EXIT_WARNING_MSG		"Exit:  unsaved changes!"
#define xrolo_OPEN_WARNING_MSG		"Open:  unsaved changes!"

#define xrolo_OPEN					-2001
#define xrolo_SAVE_AS				-2002

#define xrolo_DIR_MASK_MAX_LEN		200
#define xrolo_DIR_MASK				"*.xrolo"

#define xrolo_INSERT				-1
#define xrolo_ADD					-2

#define xrolo_ASCEND				-1
#define xrolo_DESCEND				-2

#define xrolo_DEFAULT_VIEWPORT_ROWS			12
#define xrolo_DEFAULT_VIEWPORT_COLUMNS		40
#define xrolo_DEFAULT_INDEX_ROWS			15
#define xrolo_DEFAULT_INDEX_COLUMNS			30
#define xrolo_DEFAULT_DELIMITER				"####\n"
#define xrolo_DEFAULT_PRINT_COMMAND			"lp"
#define xrolo_DEFAULT_SORT_ROW				1
#define xrolo_DEFAULT_INDEX_ROW				1

#define xrolo_APP_CLASS				"XRolodex"


/*
Application resources:
*/

typedef struct {
	Boolean viewport_menubar;
	int viewport_rows;
	int viewport_columns;
	int index_rows;
	int index_columns;
	int index_row;
	int sort_row;
	Boolean center_dialogs;
	Boolean minimal_dialogs;
	Boolean force_save;
	Boolean index_stay_up;
	Boolean find_sensitive;
	Boolean find_wraparound;
	char *delimiter;
	char *print_command;
	Boolean print_standard_output;
	Boolean print_formfeed;
	char *dir_mask;
} ApplicationData, *ApplicationDataPtr;


#define XtNviewportMenuBar		"viewportMenuBar"
#define XtCViewportMenuBar		"ViewportMenuBar"
#define XtNviewportRows			"viewportRows"
#define XtCViewportRows			"ViewportRows"
#define XtNviewportColumns		"viewportColumns"
#define XtCViewportColumns		"ViewportColumns"
#define XtNindexRows			"indexRows"
#define XtCIndexRows			"IndexRows"
#define XtNindexColumns			"indexColumns"
#define XtCIndexColumns			"IndexColumns"
#define XtNindexStayUp			"indexStayUp"
#define XtNsortRow				"sortRow"
#define XtCSortRow				"SortRow"
#define XtNindexRow				"indexRow"
#define XtCIndexRow				"IndexRow"
#define XtCIndexStayUp			"IndexStayUp"
#define XtNcenterDialogs		"centerDialogs"
#define XtCCenterDialogs		"CenterDialogs"
#define XtNminimalDialogs		"minimalDialogs"
#define XtCMinimalDialogs		"MinimalDialogs"
#define XtNforceSave			"forceSave"
#define XtCForceSave			"ForceSave"
#define XtNfindSensitive		"findSensitive"
#define XtCFindSensitive		"FindSensitive"
#define XtNfindWraparound		"findWraparound"
#define XtCFindWraparound		"FindWraparound"
#define XtCMinimalDialogs		"MinimalDialogs"
#define XtNentryDelimiter		"entryDelimiter"
#define XtCEntryDelimiter		"EntryDelimiter"
#define XtNprintCommand			"printCommand"
#define XtCPrintCommand			"PrintCommand"
#define XtNprintStandardOutput	"printStandardOutput"
#define XtCPrintStandardOutput	"PrintStandardOutput"
#define XtNprintFormfeed		"printFormfeed"
#define XtCPrintFormfeed		"PrintFormfeed"
#define XtNdirectoryMask		"directoryMask"
#define XtCDirectoryMask		"DirectoryMask"


