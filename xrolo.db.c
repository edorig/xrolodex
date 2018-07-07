/**** xrolo.db.c ****/  /**** formatted with 4-column tabs ****/

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
to the "current" database entry in `entry', and formally sets the
current entry pointer in `streamdb.c' only when necessary as an
intermediate step in performing a database operation, e.g.,
deleting the current database entry.
*******************************************************************/


#include "xrolo.db.h"


/*
Private globals (these maintain the current database state):
*/

static StreamDB xrolo = NULL;
static EntryDB entry, deleted_entry, temp_entry;

static char **index_array = NULL;


/*
xrolo_db_create() uses streamdb_create() to create a stream-
oriented rolodex database.  The `StreamDB' object `xrolo'
maintains the state of the database per se, including the
filename associated with the database.
*/

EntryDB xrolo_db_create(filename, delimiter)
char *filename, *delimiter;
{
	if (xrolo) {					/* policy:  support one DB only */
		xrolo_db_destroy();
		deleted_entry = NULL;
	}
	xrolo = streamdb_create(filename, delimiter);
	entry = streamdb_load(xrolo);	/* NULL for a new file */
	return streamdb_set_current(xrolo, entry);
}	/* xrolo_db_create */


/*
xrolo_db_destroy() frees the stream-oriented database,
as well as the entries maintained by this module.
*/

void xrolo_db_destroy()
{
	if (deleted_entry)
		streamdb_free_solitaire(xrolo, deleted_entry);
	if (xrolo)
		streamdb_destroy(xrolo);
	xrolo_db_free_index_array();
}	/* xrolo_db_destroy */


/*
xrolo_db_is_active() provides information about whether
or not the database has been activated.
*/

int xrolo_db_is_active()
{
	return (xrolo) ? TRUE : FALSE;
}	/* xrolo_db_is_active */


/*
xrolo_db_is_modified() provides information about whether
or not the database has been modified.
*/

int xrolo_db_is_modified()
{
	if (!xrolo)
		return FALSE;
	else
		return streamdb_modified(xrolo);
}	/* xrolo_db_is_modified */


/*
xrolo_db_save() updates the database to disk using
information stored in the `StreamDB' object, `xrolo'.
*/

int xrolo_db_save(force_save)
int force_save;
{
	int status;

	force_save = (force_save == xrolo_db_FORCE_SAVE) ?
		streamdb_FORCE_SAVE : streamdb_MOD_SAVE;
	if (!xrolo)
		return FALSE;
	else if ((status = streamdb_save(xrolo, force_save)) < 0)
		return FALSE;
	else
		return status;
}	/* xrolo_db_save */


/*
xrolo_db_save_as() updates the database to disk using
information stored in the `StreamDB' object, `xrolo'
and the filename given as an argument.
*/

int xrolo_db_save_as(filename)
char *filename;
{
	int status;

	if (!xrolo)
		return FALSE;
	else if ((status = streamdb_save_as(xrolo, filename)) < 0)
		return FALSE;
	else
		return status;
}	/* xrolo_db_save_as */


/*
xrolo_db_save_backup() makes a copy of the rolodex file under
an alternate filename.
*/

int xrolo_db_save_backup()
{
	if (!xrolo || !file_exists(xrolo->filename))
		return FALSE;
	else {
#ifdef BACKUP_CP
		char command[streamdb_MAX_FILENAME_SPEC + 10];

		sprintf(command,
			"cp -p %s %s%s", xrolo->filename, xrolo->filename, ".bak");
		system(command);	/* don't check result at present */
#else
		char file_spec[streamdb_MAX_FILENAME_SPEC + 10];

		sprintf(file_spec, "%s%s", xrolo->filename, ".bak");
		rename(xrolo->filename, file_spec);
		xrolo_db_save(xrolo_db_FORCE_SAVE);
#endif
		return TRUE;
	}
}	/* xrolo_db_save_backup */


/*
xrolo_db_current_entry_is_first_entry() allows the application
to determine if it's already processing the first entry.
*/

int xrolo_db_current_entry_is_first_entry()
{
	if (!xrolo || !entry)
		return FALSE;
	else 
		return (entry == streamdb_get_first(xrolo));
}	/* xrolo_db_current_entry_is_first_entry */


/*
xrolo_db_current_entry_is_last_entry() allows the application
to determine if it's already processing the last entry.
*/

int xrolo_db_current_entry_is_last_entry()
{
	if (!xrolo || !entry)
		return FALSE;
	else
		return (entry == streamdb_get_last(xrolo));
}	/* xrolo_db_current_entry_is_last_entry */


/*
xrolo_db_get_text() retrieves the text for the specified entry.
*/

char *xrolo_db_get_text(entry)
EntryDB entry;
{
	if (!xrolo)
		return NULL;
	return streamdb_get_entry_text(xrolo, entry);
}	/* xrolo_db_get_text */


/*
xrolo_db_nth_entry() locates the nth entry, if possible.
*/

EntryDB xrolo_db_nth_entry(position)
int position;
{
	if (!xrolo)
		return NULL;
	if ((temp_entry = streamdb_get_entry_by_position(xrolo, position))
			== NULL)
		return NULL;
	return entry = temp_entry;
}	/* xrolo_db_nth_entry */


/*
xrolo_db_next_entry() moves forward one entry in the
rolodex database, if possible.
*/

EntryDB xrolo_db_next_entry()
{
	if (!xrolo)
		return NULL;
	if (!entry) {
		/*
		if pointer past last entry, return last entry:
		*/
		if ((temp_entry = streamdb_get_last(xrolo)) == NULL)
			return NULL;
	}
	else if ((temp_entry = streamdb_get_next(xrolo, entry)) == NULL)
		return NULL;
	return entry = temp_entry;
}	/* xrolo_db_next_entry */


/*
xrolo_db_current_entry() gets the current entry from
the stream database, as maintained by this module in `entry'.
*/

EntryDB xrolo_db_current_entry()
{
	if (!xrolo)
		return NULL;
	return entry;
}	/* xrolo_db_current_entry */


/*
xrolo_db_previous_entry() moves backward one entry in the
rolodex database, if possible.
*/

EntryDB xrolo_db_previous_entry()
{
	if (!xrolo)
		return NULL;
	if (!entry) {
		/*
		if pointer past last entry, return last entry:
		*/
		if ((temp_entry = streamdb_get_last(xrolo)) == NULL)
			return NULL;
	}
	else if ((temp_entry = streamdb_get_previous(xrolo, entry)) == NULL)
		return NULL;
	return entry = temp_entry;
}	/* xrolo_db_previous_entry */


/*
xrolo_db_first_entry() moves to the beginning of the
rolodex database, if possible.
*/

EntryDB xrolo_db_first_entry()
{
	if (!xrolo)
		return NULL;
	if ((temp_entry = streamdb_get_first(xrolo)) == NULL)
		return NULL;
	return entry = temp_entry;
}	/* xrolo_db_first_entry */


/*
xrolo_db_last_entry() moves to the end of the
rolodex database, if possible.
*/

EntryDB xrolo_db_last_entry()
{
	if (!xrolo)
		return NULL;
	if ((temp_entry = streamdb_get_last(xrolo)) == NULL)
		return NULL;
	return entry = temp_entry;
}	/* xrolo_db_last_entry */


/*
xrolo_db_past_last_entry() simply sets the current entry to
NULL, implying that insertions will be appended to the end
of the database.  See xrolo_db_insert_new_entry();
*/

EntryDB xrolo_db_past_last_entry()
{
	if (!xrolo)
		return NULL;
	if ((temp_entry = streamdb_get_last(xrolo)) == NULL)
		return NULL;
	return entry = NULL;
}	/* xrolo_db_past_last_entry */


/*
xrolo_db_insert_new_entry() inserts a new entry in the
rolodex database at the current database offset (before
the current entry) -- a "paste" operation.
*/

EntryDB xrolo_db_insert_new_entry(text)
char *text;
{
	char *temp = NULL;

	if (!xrolo)
		return NULL;
	if (!text)	/* but, OK if it's an empty string */
		return NULL;
	if (!*text || text[strlen(text) - 1] != '\n') {
		if ((temp = (char *) malloc((unsigned) (strlen(text) + 2))) != NULL) {
			strcpy(temp, text);
			strcat(temp, "\n");
			text = temp;
		}
		else
			return NULL;
	}
	if (!entry) /* occur when empty after last deletion or if past last entry */
		temp_entry = streamdb_insert(xrolo, text, streamdb_APPEND);
	else {
		streamdb_set_current(xrolo, entry);
		temp_entry = streamdb_insert(xrolo, text, streamdb_CURRENT);
	}
	if (temp)
		free(temp);
	if (temp_entry)
		return entry = temp_entry;	/* update the current entry (`entry') */
	else
		return NULL;
}	/* 	xrolo_db_insert_new_entry */


/*
xrolo_db_delete_current_entry() removes the current entry
from the rolodex database.  The deleted entry is saved for
subsequent recovery.
*/

EntryDB xrolo_db_delete_current_entry()
{
	int delete_last_entry;

	if (!xrolo)
		return NULL;
	if (deleted_entry)
		streamdb_free_solitaire(xrolo, deleted_entry);
	deleted_entry = streamdb_create_solitaire(xrolo, entry);
	delete_last_entry = (streamdb_get_next(xrolo, entry) == NULL);
	streamdb_set_current(xrolo, entry);
	temp_entry = streamdb_delete(xrolo, streamdb_CURRENT);
	if (delete_last_entry)
		return entry = streamdb_get_last(xrolo);
	else
		return entry = temp_entry;
}	/* 	xrolo_db_delete_current_entry */


/*
xrolo_db_delete_current_entry_no_undo() removes the current entry
from the rolodex database.  The deleted entry is not saved for
subsequent recovery.
*/

EntryDB xrolo_db_delete_current_entry_no_undo()
{

	if (!xrolo)
		return NULL;
	streamdb_set_current(xrolo, entry);
	temp_entry = streamdb_delete(xrolo, streamdb_CURRENT);
	return entry = temp_entry;
}	/* 	xrolo_db_delete_current_entry_no_undo */


/*
xrolo_db_recover_deleted_entry() "pastes" the most recently deleted
entry into the rolodex database before the current entry.
*/

EntryDB xrolo_db_recover_deleted_entry()
{
	if (!xrolo || !deleted_entry)
		return NULL;
	streamdb_set_current(xrolo, entry);
	if ((temp_entry =
	streamdb_insert(xrolo, deleted_entry->text, streamdb_CURRENT)) == NULL)
		return NULL;
	streamdb_free_solitaire(xrolo, deleted_entry);
	deleted_entry = NULL;
	return entry = temp_entry;
}	/* 	xrolo_db_recover_deleted_entry */


/*
xrolo_db_find_entry_forward() finds the next rolodex entry with
text that matches the search text.
*/

EntryDB xrolo_db_find_entry_forward(search_text, case_sensitive_search,
	wraparound_search)
char *search_text;
int case_sensitive_search, wraparound_search;
{
	EntryDB current_entry = streamdb_get_current(xrolo);

	if (!xrolo || !entry)
		return NULL;
	if ((temp_entry = streamdb_get_next(xrolo, entry)) == NULL) {
		if (!wraparound_search)
			return NULL;
		else if ((temp_entry = streamdb_get_first(xrolo)) == NULL)
			return NULL;
	}
	streamdb_set_current(xrolo, temp_entry);
	if ((temp_entry =
	streamdb_search_forward(xrolo,
	search_text,
	case_sensitive_search ? string_SENSITIVE : string_INSENSITIVE,
	wraparound_search ? streamdb_WRAPAROUND : streamdb_NO_WRAPAROUND
	)) == NULL) {
		streamdb_set_current(xrolo, current_entry);
		return NULL;
	}
	return entry = temp_entry;
}	/* xrolo_db_find_entry_forward */


/*
xrolo_db_find_entry_reverse() finds the previous rolodex entry with
text that matches the search text.
*/

EntryDB xrolo_db_find_entry_reverse(search_text, case_sensitive_search,
wraparound_search)
char *search_text;
int case_sensitive_search, wraparound_search;
{
	EntryDB current_entry = streamdb_get_current(xrolo);

	if (!xrolo)
		return NULL;
	if (!entry) {
		/*
		if pointer past last entry, get last entry:
		*/
		if ((temp_entry = streamdb_get_last(xrolo)) == NULL)
			return NULL;
	}
	else {
		if ((temp_entry = streamdb_get_previous(xrolo, entry)) == NULL) {
			if (!wraparound_search)
				return NULL;
			else if ((temp_entry = streamdb_get_last(xrolo)) == NULL)
				return NULL;
		}
	}
	streamdb_set_current(xrolo, temp_entry);
	if ((temp_entry =
	streamdb_search_backward(xrolo,
	search_text,
	case_sensitive_search ? string_SENSITIVE : string_INSENSITIVE,
	wraparound_search ? streamdb_WRAPAROUND : streamdb_NO_WRAPAROUND
	)) == NULL) {
		streamdb_set_current(xrolo, current_entry);
		return NULL;
	}
	return entry = temp_entry;
}	/* xrolo_db_find_entry_reverse */


/*
xrolo_db_build_index_array() extracts the first line of text
from each entry and builds a dynamic array of strings.  It
is a null-terminated array.
*/

char **xrolo_db_build_index_array(line)
int line;
{
	int i, j, len, num_entries;
	char *text;
	EntryDB next_entry;

	if (!xrolo)
		return NULL;
	if ((num_entries = streamdb_get_num_entries(xrolo)) == 0)
		return NULL;
	index_array =
		(char **) malloc((unsigned) (sizeof(char *) * (num_entries + 1)));
	if (!index_array)
		return NULL;
	next_entry = streamdb_get_first(xrolo);
	for (i = 0; i < num_entries; i++) {
		text = streamdb_get_entry_text(xrolo, next_entry);
		text = streamdb_get_entry_text_begin_line(text, line);
		len = strlen(text);
		for (j = 0; j < len && text[j] != '\n'; j++)
			/* count the characters on the first line of the text */;
		len = j;
		index_array[i] = (char *) malloc((unsigned) (len + 1));
		if (!index_array[i]) {
			for (i--; i >= 0; )
				free(index_array[i--]);
			free(index_array);
			index_array = NULL;
			return NULL;
		}
		for (j = 0; j < len; j++)
			index_array[i][j] = text[j];
		index_array[i][j] = EOS;
		next_entry = streamdb_get_next(xrolo, next_entry);
	}
	index_array[num_entries] = NULL;
	return index_array;
}	/* xrolo_db_build_index_array */


/*
xrolo_db_free_index_array() releases storage for the
dynamic array of strings used in the index.
*/

void xrolo_db_free_index_array()
{
	if (index_array) {
		int i;

		for (i = 0; index_array[i]; i++)
			free(index_array[i]);
		free(index_array);
		index_array = NULL;
	}
}	/* xrolo_db_free_index_array */


/*
xrolo_db_sort_ascending() sorts the entries in ascending order.
*/

EntryDB xrolo_db_sort_ascending(line)
int line;
{
	if (!xrolo || !entry)
		return NULL;
	entry = streamdb_sort(xrolo, streamdb_ASCEND, line);
	streamdb_set_current(xrolo, entry);
	return entry;
}	/* xrolo_db_sort_ascending */


/*
xrolo_db_sort_descending() sorts the entries in descending order.
*/

EntryDB xrolo_db_sort_descending(line)
int line;
{
	if (!xrolo || !entry)
		return NULL;
	entry = streamdb_sort(xrolo, streamdb_DESCEND, line);
	streamdb_set_current(xrolo, entry);
	return entry;
}	/* xrolo_db_sort_descending */


/*
xrolo_db_set_delimiter() modifies the current delimiter for each
database entry.
*/

char *xrolo_db_set_delimiter(new_delimiter)
char *new_delimiter;
{
	if (!xrolo)
		return NULL;
	return streamdb_set_delimiter(xrolo, new_delimiter);
}	/* 	xrolo_db_set_delimiter */


/*
xrolo_db_print() dispatches print operations.
*/

int xrolo_db_print(operation, print_command, spacing)
int operation;
char *print_command, *spacing;
{
	int status = TRUE;

	if (!xrolo)
		return FALSE;
	streamdb_set_current(xrolo, entry);
	switch (operation) {
		case xrolo_db_PRINT_SO:
			streamdb_print(xrolo, spacing);
			break;
		case xrolo_db_PRINT_ALL_SO:
			streamdb_print_all(xrolo, spacing);
			break;
		case xrolo_db_PRINT_PTR:
			status = streamdb_print_ptr(xrolo, print_command, spacing);
			break;
		case xrolo_db_PRINT_ALL_PTR:
			status = streamdb_print_ptr_all(xrolo, print_command, spacing);
			break;
		default:
			break;
	}
	return status;
}	/* xrolo_db_print */

