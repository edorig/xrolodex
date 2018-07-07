/**** xrolo.db.h ****/  /**** formatted with 4-column tabs ****/

/*******************************************************************
This module imposes arbitrary rolodex policies on the generic,
stream-oriented database object, `StreamDB', that's implemented in
`steamdb.c'.  `xrolo.db.c' is not useful with applications other
than `xrolo'.

Overall, the idea is that `xrolo' manages only one active rolodex
database/file at a time.  Thus, the state of the rolodex file can
be managed with a small number of variables that are private to
this file; these variables include `xrolo', `entry', and
`deleted_entry'.

Although `streamdb.c' provides facilities for maintaining the
current database entry, `xrolo.db.c' maintains its own pointer
to the current database entry in `entry', and formally sets the
current entry pointer in `streamdb.c' only when necessary as an
intermediate step in performing a database operation, e.g.,
deleting the current database entry.
*******************************************************************/


#include "streamdb.h"


/*
Constants:
*/

#define  xrolo_db_MOD_SAVE		-1
#define  xrolo_db_FORCE_SAVE	-2
#define  xrolo_db_PRINT_CURRENT	-3
#define  xrolo_db_PRINT_ALL		-4
#define  xrolo_db_PRINT_SO		-5
#define  xrolo_db_PRINT_ALL_SO	-6
#define  xrolo_db_PRINT_PTR		-7
#define  xrolo_db_PRINT_ALL_PTR	-8


/*
Public functions:
*/

extern EntryDB xrolo_db_create();
extern void xrolo_db_destroy();
extern int xrolo_db_is_active();
extern int xrolo_db_is_modified();
extern int xrolo_db_save();
extern int xrolo_db_save_as();
extern int xrolo_db_save_backup();
extern int xrolo_db_current_entry_is_last_entry();
extern int xrolo_db_current_entry_is_first_entry();
extern char *xrolo_db_get_text();
extern EntryDB xrolo_db_nth_entry();
extern EntryDB xrolo_db_next_entry();
extern EntryDB xrolo_db_current_entry();
extern EntryDB xrolo_db_previous_entry();
extern EntryDB xrolo_db_first_entry();
extern EntryDB xrolo_db_last_entry();
extern EntryDB xrolo_db_past_last_entry();
extern EntryDB xrolo_db_insert_new_entry();
extern EntryDB xrolo_db_delete_current_entry();
extern EntryDB xrolo_db_delete_current_entry_no_undo();
extern EntryDB xrolo_db_recover_deleted_entry();
extern EntryDB xrolo_db_find_entry_forward();
extern EntryDB xrolo_db_find_entry_reverse();
extern char **xrolo_db_build_index_array();
extern void xrolo_db_free_index_array();
extern EntryDB xrolo_db_sort_ascending();
extern EntryDB xrolo_db_sort_descending();
extern char *xrolo_db_set_delimiter();
extern int xrolo_db_print();

