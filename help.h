/**** help.h ****/  /**** formatted with 4-column tabs ****/

/*******************************************************************
Currently, there is no help object; that is, this module simply
provides functions for creating and activating the help system.
*******************************************************************/


#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>

#include <string.h>

#include "motif.h"


/*
Public structures:
*/

typedef struct {
	char *topic;
	char *text;
} HelpItem;


/*
Public functions:
*/

extern void help_create_dialog();
extern void help_realize();
extern Widget help_shell();
extern void help_create_pulldown_menu();
extern void Help();

