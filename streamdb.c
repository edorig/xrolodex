/**** streamdb.c ****/  /**** formatted with 4-column tabs ****/

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


/*************************************************************************
A `StreamDB' object encapsulates the data and operations required to
provide basic creation, deletion, storage, and manipulation of a byte-
oriented stream database.  During processing, it is organized as a
doubly-linked list where each element in the list points to a string
that contains the text for one database entry.  A database is created
with streamdb_create(), which takes a filename as one of its arguments.
Also, the application can specify the delimiter that serves as a
separator between database entries in the file.

streamdb_insert() adds entries to the database and streamdb_delete()
removes entries.  Deletion must be done by position or with the special
constant `streamdb_CURRENT'.  With insertions, you can specify a
position or one of two special constants `streamdb_CURRENT' and
`streamdb_APPEND'.  Position references are zero-based, as with arrays
in C.  Note that you can't insert at/before the current position until
you establish a current entry with streamdb_set_current().

In addition to inserting and deleting database entries, you can load
a database from a disk file with streamdb_load() and save a database
to disk with either streamdb_save() or streamdb_save_alt().  The
former save operation uses the filename specified in streamdb_create()
and updates the file modification status, while the latter takes an
alternate filename as an argument and does not update the file
modification status.

You can create database entries with either NULL text fields or null
strings for their text.  This allows a two-stage operation for
(1) creating an entry, and (2) setting its text field to a legitimate
value.  During save operations, however, entries with NULL text fields
or zero-length text fields will be skipped.

There are several convenience functions for getting and setting
current entries, and text fields and for retrieving the first and
last entries in the doubly-linked list.

Sample usage:

void main(argc, argv)
int argc;
char *argv[];
{
	StreamDB db;
	EntryDB current;

	db = streamdb_create("teststream.db", "####\n");
	current = streamdb_insert(db, "third entry\n", streamdb_APPEND);
	streamdb_set_current(db, current);
	streamdb_insert(db, "first entry\n", 0);
	streamdb_insert(db, "second entry\n", streamdb_CURRENT);
	streamdb_insert(db, "fourth entry\n", 3);
	streamdb_save(db, streamdb_MOD_SAVE);
	printf("Database modified? %s\n", streamdb_modified(db) ? "Yes" : "No");
	streamdb_delete(db, 0);
	streamdb_delete(db, 2);
	streamdb_save_alt(db, "teststream.alt");
	printf("Database modified? %s\n", streamdb_modified(db) ? "Yes" : "No");
	streamdb_destroy(db);
	exit(0);
}

*************************************************************************/


#include "streamdb.h"


/*
Private globals using by the sorting functions only:
*/

static EntryDB first_entry, last_entry;
static int num_nodes;


/*
Private support functions:
*/

static int build_list_from_buffer();
static EntryDB get_new_node();
static int offset_without_delimiter();
static int load_buffer();
static int read_from_disk();
static int write_streamdb_to_disk();
/*static int write_text_to_disk();*/
static int pipe_text_to_ptr();
static int file_size();
static int scan_for_next_entry();
static int build_search_tree();
static int insert_into_tree();
static int go_left();
static char *move_forward_n_lines();
static void traverse_tree();
static void destroy_tree();
static void update_linked_list();


/*
Public functions:
*/

/*
streamdb_create() allocates a data structure that holds
information about an active stream-oriented database.
*/

StreamDB streamdb_create(filename, delimiter)
char *filename, *delimiter;
{
	StreamDB sObject;

	if (!(sObject = (StreamDB) malloc(sizeof(_StreamDB))))
		return NULL;
	sObject->self = sObject;
	if (!filename)
		*sObject->filename = EOS;
	else
		strncpy(sObject->filename, filename, streamdb_MAX_FILENAME_SPEC);
	sObject->file_modified = FALSE;
	sObject->num_entries = 0;
	sObject->delimiter_len = strlen(delimiter);
	sObject->first = sObject->last = sObject->current = NULL;
	sObject->delimiter = delimiter;
	if (!delimiter || !*delimiter) {
		free(sObject->self);
		return NULL;
	}
	else
		return sObject;
}	/* streamdb_create */


/*
streamdb_destroy() frees the storage for a `StreamDB' object.
*/

void streamdb_destroy(sObject)
StreamDB sObject;
{
	if (sObject->first) {
		EntryDB entry = sObject->first;

		while (entry) {
			if (entry->text)
				free(entry->text);
			sObject->first = entry; /* borrow `first' for temp. storage */
			entry = entry->next;
			free(sObject->first);
		}
	}
	free(sObject->self);
}	/* streamdb_destroy */


/*
streamdb_modified() returns TRUE or FALSE depending on whether
or not the stream has been modified since the most recent
load/save operation.
*/

int streamdb_modified(sObject)
StreamDB sObject;
{
	return sObject->file_modified;
}	/* streamdb_modified */


/*
streamdb_get_num_entries() returns the total number of entries
in the database -- the number of nodes in the linked list.
*/

int streamdb_get_num_entries(sObject)
StreamDB sObject;
{
	return sObject->num_entries;
}	/* streamdb_get_num_entries */


/*
streamdb_get_first() returns a pointer to
the list of entries.
*/

EntryDB streamdb_get_first(sObject)
StreamDB sObject;
{
	return sObject->first;
}	/* streamdb_get_first */


/*
streamdb_get_last() returns a pointer to
the tail of the list of entries.
*/

EntryDB streamdb_get_last(sObject)
StreamDB sObject;
{
	return sObject->last;
}	/* streamdb_get_last */


/*
streamdb_get_previous() returns a pointer to
the previous entry.
*/
/*ARGSUSED*/
EntryDB streamdb_get_previous(sObject, entry)
StreamDB sObject;	/* not currently used */
EntryDB entry;
{
	return (entry) ? entry->previous : NULL;
}	/* streamdb_get_previous */


/*
streamdb_get_next() returns a pointer to
the next entry.
*/
/*ARGSUSED*/
EntryDB streamdb_get_next(sObject, entry)
StreamDB sObject;	/* not currently used */
EntryDB entry;
{
	return (entry) ? entry->next : NULL;
}	/* streamdb_get_next */


/*
streamdb_get_entry_by_position() returns a pointer to
the entry in the specified position--zero-based.
*/

EntryDB streamdb_get_entry_by_position(sObject, position)
StreamDB sObject;
int position;
{
	EntryDB entry = sObject->first;

	if (position < 0 || position >= sObject->num_entries)
		return NULL;
	while (entry && position--)
		entry = entry->next;
	return entry;
}	/* streamdb_get_entry_by_position */


/*
streamdb_get_current() returns a pointer to
the current entry.
*/

EntryDB streamdb_get_current(sObject)
StreamDB sObject;
{
	return sObject->current;
}	/* streamdb_get_current */


/*
streamdb_set_current() updates the pointer to
the current entry.
*/

EntryDB streamdb_set_current(sObject, entry)
StreamDB sObject;
EntryDB entry;
{
	return sObject->current = entry;
}	/* streamdb_set_current */


/*
streamdb_get_current_text() returns a pointer to
the text for the current entry.
*/

char *streamdb_get_current_text(sObject)
StreamDB sObject;
{
	if (!sObject->current)
		return NULL;
	return sObject->current->text;
}	/* streamdb_get_current_text */


/*
streamdb_set_current_text() returns a pointer to
the new text for the current entry.  `new_text' can
be NULL, and `new_text' can point to a null string.
*/

char *streamdb_set_current_text(sObject, new_text)
StreamDB sObject;
char *new_text;
{
	char *text;

	if (!sObject->current) 
		return NULL;
	sObject->file_modified = TRUE;
	if ((text = sObject->current->text) != NULL)
		free(text);
	if (!new_text)
		return sObject->current->text = NULL;
	else if ((text = (char *) malloc((unsigned) (strlen(new_text) + 1)))
			!= NULL)
		strcpy(text, new_text);
	return sObject->current->text = text;
}	/* streamdb_set_current_text */


/*
streamdb_get_entry_text() returns a pointer to
the text for the current entry.
*/
/*ARGSUSED*/
char *streamdb_get_entry_text(sObject, entry)
StreamDB sObject;	/* not currently used */
EntryDB entry;
{
	return (entry) ? entry->text : NULL;
}	/* streamdb_get_entry_text */


/*
streamdb_get_entry_text_begin_line() returns a pointer to
the text for the current entry.
*/
/*ARGSUSED*/
char *streamdb_get_entry_text_begin_line(text, line)
char *text;
int line;
{
	return move_forward_n_lines(text, line - 1);
}	/* streamdb_get_entry_text_begin_line */


/*
streamdb_get_delimiter() returns a pointer to
the text for the current delimiter.
*/

char *streamdb_get_delimiter(sObject)
StreamDB sObject;
{
	return sObject->delimiter;
}	/* streamdb_get_delimiter */


/*
streamdb_set_delimiter() sets and returns a pointer to
the text for the new delimiter.
*/

char *streamdb_set_delimiter(sObject, new_delimiter)
StreamDB sObject;
char *new_delimiter;
{
	if (!new_delimiter || !*new_delimiter)
		return NULL;
/*	sObject->file_modified = TRUE;*/
	sObject->delimiter_len = strlen(new_delimiter);
	sObject->delimiter = new_delimiter;
	return sObject->delimiter;
}	/* streamdb_set_delimiter */


/*
streamdb_create_solitaire() returns a pointer to a
standalone copy of an existing entry.  This function
is useful in providing undo services, i.e., you can
make a copy of an entry before deleting it.  It is
not part of the database structure.
*/

EntryDB streamdb_create_solitaire(sObject, entry)
StreamDB sObject;
EntryDB entry;
{
	EntryDB new_entry;

	if (!entry) 
		return NULL;
	if ((new_entry = get_new_node()) == NULL)
		return NULL;
	if (!entry->text)
		new_entry->text = NULL;
	else {
		if ((new_entry->text =
		(char *) malloc((unsigned) (strlen(entry->text) + 1))) == NULL) {
			streamdb_free_solitaire(sObject, new_entry);
			return NULL;
		}
	}
	strcpy(new_entry->text, entry->text);
	new_entry->previous = entry->previous;
	new_entry->next = entry->next;
	return new_entry;
}	/* streamdb_create_solitaire */


/*
streamdb_free_solitaire() deletes the storage for a
standalone database node.
*/
/*ARGSUSED*/
void streamdb_free_solitaire(sObject, entry)
StreamDB sObject;	/* not currently used */
EntryDB entry;
{
	if (!entry) 
		return;
	if (entry->text)
		free(entry->text);
	free(entry);
}	/* streamdb_free_solitaire */


/*
streamdb_load() oversees the loading of the database's text
into a list of dynamically-allocated buffers, one for each
entry.  It returns a pointer to the head of the list.
*/

EntryDB streamdb_load(sObject)
StreamDB sObject;
{
	int size;
	char *buffer;

	if ((size = file_size(sObject->filename)) == streamdb_NO_FILE)
		return NULL;
	if ((buffer = (char *) malloc((unsigned) (size + 1))) == NULL)
		return NULL;
	if (!(size = load_buffer(sObject, buffer)))
		return NULL;
	if (build_list_from_buffer(sObject, buffer, size)) {
		free(buffer);
		return sObject->first;
	}
	else {
		free(buffer);
		return NULL;
	}
}	/* streamdb_load */


/*
streamdb_save() oversees the storage of a memory-resident,
stream-oriented database as a text file. It returns TRUE if
the disk write operation succeeded.
*/

int streamdb_save(sObject, force_save)
StreamDB sObject;
int force_save;
{
	if (force_save != streamdb_FORCE_SAVE && !sObject->file_modified)
		return FALSE;
	if (!write_streamdb_to_disk(sObject, sObject->filename))
		return FALSE;
	sObject->file_modified = FALSE;
	return TRUE;
}	/* streamdb_save */


/*
streamdb_save_as() oversees the storage of a memory-resident,
stream-oriented database as a named text file. It returns TRUE
if the disk write operation succeeded.
*/

int streamdb_save_as(sObject, filename)
StreamDB sObject;
char *filename;
{
	if (!write_streamdb_to_disk(sObject, filename))
		return FALSE;
	sObject->file_modified = FALSE;
	streamdb_set_filename(sObject, filename);
	return TRUE;
}	/* streamdb_save_as */


/*
streamdb_save_alt() oversees the storage of a memory-resident,
stream-oriented database as a text file. It returns TRUE if
the disk write operation succeeded.  This save function takes
a filename allowing you to save the database to an alternate
file, and IT DOES NOT UPDATE THE FILE MODIFICATION STATUS.
*/

int streamdb_save_alt(sObject, filename)
StreamDB sObject;
char *filename;
{
	return write_streamdb_to_disk(sObject, filename);
}	/* streamdb_save_alt */


/*
streamdb_set_filename() updates the filename for the
existing stream database.
*/

char *streamdb_set_filename(sObject, filename)
StreamDB sObject;
char *filename;
{
	strncpy(sObject->filename, filename, streamdb_MAX_FILENAME_SPEC);
	return sObject->filename;
}	/* streamdb_set_filename */


/*
streamdb_insert() oversees the insertion of a string into a
stream-oriented database.  It returns a pointer to the new
entry.  The position of insertion is zero-based, like a
character array.  The insertion is performed like a text
editor insertion, e.g., an insertion at the current entry
is placed just before the current entry and the current
entry pointer is not modified.  `text' can be NULL, and
`text' can point to a null string.
*/

EntryDB streamdb_insert(sObject, text, position)
StreamDB sObject;
char *text;
int position;
{
	EntryDB new_entry, entry;

	if (position > sObject->num_entries)
		return NULL;
	if (position == streamdb_CURRENT && !sObject->current) {
		if (!sObject->first)
			/* first entry into an empty database--allow it */;
		else
			return NULL;
	}
	if ((new_entry = get_new_node()) == NULL)
		return NULL;
	if (!text)
		new_entry->text = NULL;
	else {
		if ((new_entry->text = (char *) malloc((unsigned) (strlen(text) + 1)))
			== NULL)
		return NULL;
		strcpy(new_entry->text, text);	/* could be zero-length string */
	}
	sObject->file_modified = TRUE;
	entry = sObject->first;
	if (position == 0 || (position == streamdb_APPEND && !entry) ||
	(position == streamdb_CURRENT && sObject->current == sObject->first)) {
		new_entry->next = entry; /* could be NULL */
		if (entry)
			entry->previous = new_entry;
		sObject->first = new_entry;
		if (!entry)
			sObject->last = new_entry;
		if (position == streamdb_CURRENT && !sObject->current)
			sObject->current = new_entry;
	}
	else if (position == streamdb_APPEND || position == sObject->num_entries) {
		entry = sObject->last;
		entry->next = new_entry;
		new_entry->previous = entry;
		sObject->last = new_entry;
	}
	else if (position == streamdb_CURRENT) {
		entry = sObject->current->previous;
		entry->next = new_entry;
		new_entry->next = sObject->current;
		new_entry->previous = entry;
		sObject->current->previous = new_entry;
	}
	else {
		while (entry->next && position > 1) {	/* count down to position	*/
			entry = entry->next;				/* just prior to insert pt.	*/
			position--;
		}
		new_entry->next = entry->next;
		entry->next->previous = new_entry;
		entry->next = new_entry;
		new_entry->previous = entry;
	}
	sObject->num_entries++;
	return new_entry;
}	/* streamdb_insert */


/*
streamdb_delete() oversees the deletion of a text entry
from stream-oriented database.  It returns a pointer to the
next entry, i.e., the entry after the deleted entry.  The
position specification should be zero-based.
*/

EntryDB streamdb_delete(sObject, position)
StreamDB sObject;
int position;
{
	EntryDB entry;

	if (!sObject->first || !sObject->num_entries ||
			position >= sObject->num_entries)
		return NULL;
	if (position == streamdb_CURRENT && !sObject->current)
		return NULL;
	sObject->file_modified = TRUE;
	entry = sObject->first;
	sObject->num_entries--;
	if (position == 0 ||
	(position == streamdb_CURRENT && sObject->first == sObject->current)) {
		if (sObject->first == sObject->current)
			sObject->current = NULL;
		sObject->first = entry->next;		/* could be NULL */
		if (!sObject->first)
			sObject->current = sObject->last = NULL;
		if (entry->text)
			free(entry->text);
		free(entry);
		if (sObject->first)
			sObject->first->previous = NULL;/* could be NULL already */
		return sObject->first;
	}
	else {
		EntryDB next_entry;

		if (position == streamdb_CURRENT)
			entry = sObject->current->previous;
		else
			while (position > 1) {			/* count down to position	*/
				entry = entry->next;		/* just prior to delete pt.	*/
				position--;
			}
		next_entry = entry->next->next;
		if (!next_entry)
			sObject->last = entry;
		if (entry->next->text)
			free(entry->next->text);
		free(entry->next);
		entry->next = next_entry;
		if (next_entry)
			next_entry->previous = entry;
		return next_entry;
	}
}	/* streamdb_delete */


/*
streamdb_search_forward() traverses the list beginning at the
current entry for the next entry with text that matches the
search string.
*/

EntryDB streamdb_search_forward(sObject, search_text, case_mode, wrap_mode)
StreamDB sObject;
char *search_text;
int case_mode, wrap_mode;
{
	EntryDB entry = sObject->current;

	if (!sObject->current)
		return NULL;
	if (case_mode == string_INSENSITIVE) {
		while (entry) {
			if (string_search_insensitive(entry->text, search_text, 0) !=
			string_NO_MATCH)
				return entry;
			entry = entry->next;
		}
		if (wrap_mode == streamdb_WRAPAROUND && sObject->num_entries > 1) {
			entry = sObject->first;
			while (entry && entry != sObject->current) {
				if (string_search_insensitive(entry->text, search_text, 0) !=
				string_NO_MATCH)
					return entry;
				entry = entry->next;
			}
		}
	}
	else {
		while (entry) {
			if (string_search(entry->text, search_text, 0) !=
			string_NO_MATCH)
				return entry;
			entry = entry->next;
		}
		if (wrap_mode == streamdb_WRAPAROUND && sObject->num_entries > 1) {
			entry = sObject->first;
			while (entry && entry != sObject->current) {
				if (string_search(entry->text, search_text, 0) !=
				string_NO_MATCH)
					return entry;
				entry = entry->next;
			}
		}
	}
	return NULL;
}	/* streamdb_search_forward */


/*
streamdb_search_backward() traverses the list beginning at the
current entry for the next entry with text that matches the
search string.
*/

EntryDB streamdb_search_backward(sObject, search_text, case_mode, wrap_mode)
StreamDB sObject;
char *search_text;
int case_mode, wrap_mode;
{
	EntryDB entry = sObject->current;

	if (!sObject->current)
		return NULL;
	if (case_mode == string_INSENSITIVE) {
		while (entry) {
			if (string_search_insensitive(entry->text, search_text, 0) !=
			string_NO_MATCH)
				return entry;
			entry = entry->previous;
		}
		if (wrap_mode == streamdb_WRAPAROUND && sObject->num_entries > 1) {
			entry = sObject->last;
			while (entry && entry != sObject->current) {
				if (string_search_insensitive(entry->text, search_text, 0) !=
				string_NO_MATCH)
					return entry;
				entry = entry->previous;
			}
		}
	}
	else {
		while (entry) {
			if (string_search(entry->text, search_text, 0) !=
			string_NO_MATCH)
				return entry;
			entry = entry->previous;
		}
		if (wrap_mode == streamdb_WRAPAROUND && sObject->num_entries > 1) {
			entry = sObject->last;
			while (entry && entry != sObject->current) {
				if (string_search(entry->text, search_text, 0) !=
				string_NO_MATCH)
					return entry;
				entry = entry->previous;
			}
		}
	}
	return NULL;
}	/* streamdb_search_backward */


/*
streamdb_sort() builds a binary tree of pointers to the database
entries.  An inorder traversal is used to retrieve the entries in
sorted order.
*/

EntryDB streamdb_sort(sObject, order, line)
StreamDB sObject;
int order, line;
{
	TreeNode root;

	first_entry = NULL;
	root = NULL;
	num_nodes = sObject->num_entries;
	if (build_search_tree(&root, sObject->first, order, line)) {
		sObject->file_modified = TRUE;
		traverse_tree(root);
		sObject->first = first_entry;
		sObject->last = last_entry;
	}
	destroy_tree(root);
	return sObject->first;
}	/* streamdb_sort */


/*
streamdb_print() prints the text for the current entry to standard output.
*/

void streamdb_print(sObject, spacing)
StreamDB sObject;
char *spacing;
{
	if (!spacing)
		spacing = "";
	if (sObject->current && (int) strlen(sObject->current->text) > 0) {
		printf("%s%s", sObject->current->text, spacing);
	}
}	/* streamdb_print */


/*
streamdb_print_all() traverses the list and prints the text
for each entry to standard output.
*/

void streamdb_print_all(sObject, spacing)
StreamDB sObject;
char *spacing;
{
	if (sObject->first) {
		EntryDB entry = sObject->first;

		if (!spacing)
			spacing = "";
		while (entry) {
			printf("%s%s", entry->text, spacing);
			entry = entry->next;
		}
	}
}	/* streamdb_print_all */


/*
streamdb_print_ptr() prints the text for the current entry.
*/

int streamdb_print_ptr(sObject, print_command, spacing)
StreamDB sObject;
char *print_command, *spacing;
{
	return pipe_text_to_ptr(sObject, print_command, spacing,
		streamdb_WRITE_CURRENT);
}	/* streamdb_print_ptr */


/*
streamdb_print_ptr_all() traverses the list and prints the text
for each entry.
*/

int streamdb_print_ptr_all(sObject, print_command, spacing)
StreamDB sObject;
char *print_command, *spacing;
{
	return pipe_text_to_ptr(sObject, print_command, spacing,
		streamdb_WRITE_ALL);
}	/* streamdb_print_ptr_all */


/*
file_exists() determines whether or not a file exists.
*/

int file_exists(file_spec)
char *file_spec;
{
	return (file_size(file_spec) != streamdb_NO_FILE);
}   /* file_exists */


/*
Private support functions:
*/

/*
build_list_from_buffer() constructs a linked list of the
entries in the byte stream, based on the specified delimiter.
*/

static int build_list_from_buffer(sObject, buffer, size)
StreamDB sObject;
char *buffer;
int size;
{
	int first_time = TRUE, next = 0;
	int current, i, text_len;
	EntryDB new_entry, prev_entry;

	sObject->num_entries = 0;
	sObject->first = sObject->last = NULL;
	do {
		current = offset_without_delimiter(sObject, buffer, next);
		next = scan_for_next_entry(sObject, buffer, current);
		if (next == string_NO_MATCH)
			next = size;						/* end of file */
		if ((text_len = next - current) > 0) {
			if ((new_entry = get_new_node()) == NULL)
				return FALSE;
			if (first_time) {
				first_time = FALSE;
				sObject->first = new_entry;
			}
			else {
				/*
				`prev_entry' is set at the end of the first cycle:
				*/
				prev_entry->next = new_entry;
				new_entry->previous = prev_entry;
			}
			new_entry->text = (char *) malloc((unsigned) (next - current + 1));
			for (i = 0; i < text_len; )
				new_entry->text[i++] = buffer[current++];
			new_entry->text[i] = EOS;
			prev_entry = new_entry;
			sObject->num_entries++;
		}
	} while (current < size);
	sObject->last = new_entry;
	return TRUE;
}	/* build_list_from_buffer */


/*
get_new_node() allocates a new node and initializes its storage.
*/

static EntryDB get_new_node()
{
	EntryDB new_entry;

	if ((new_entry = (EntryDB) malloc(sizeof(_EntryDB))) == NULL)
		return NULL;
	new_entry->text = NULL;
	new_entry->previous = new_entry->next = NULL;
	return new_entry;
}	/* get_new_node */


/*
offset_without_delimiter() is used to "get at" the real entry,
not counting the arbitrary delimiter.
*/

static int offset_without_delimiter(sObject, buffer, offset)
StreamDB sObject;
char *buffer;
int offset;
{
	/*
	simply skip over the delimiter if it's there:
	*/
	if (strncmp(&buffer[offset], sObject->delimiter, sObject->delimiter_len)
			== 0)
		offset += sObject->delimiter_len;
	return offset;
}	/* offset_without_delimiter */


/*
load_buffer() loads text into a character buffer.
For efficiency, it assumes that the application has
already determined that the file is NOT too large for
the buffer.  It returns the length of the buffer/string.
*/

static int load_buffer(sObject, buffer)
StreamDB sObject;
char *buffer;
{
	int len;

	if ((len = read_from_disk(buffer, sObject->filename)) ==
				streamdb_CANT_READ) {
		return FALSE;
	}
	return len;
}    /* load_buffer */


/*
read_from_disk() loads a buffer from a disk file.  It
null-terminates the buffer/string and returns the number
of bytes read, not counting the end-of-string marker.
You may want to use fread() here.
*/

static int read_from_disk(buffer, file_spec)
register char *buffer;
char *file_spec;
{
	FILE *db_file;
	register int len;

	if ((db_file = fopen(file_spec, "r")) == NULL) {
		*buffer = EOS;
		return streamdb_CANT_READ;
	}
	for (len = 0; (buffer[len] = fgetc(db_file)) != (char) EOF; len++)
		;
	buffer[len] = EOS;
	fclose(db_file);
	return len;
}   /* read_from_disk */


/*
write_streamdb_to_disk() writes a stream database to disk and returns
TRUE, if no errors occur.  You may want to use fwrite() or fputs() here.
*/

static int write_streamdb_to_disk(sObject, file_spec)
StreamDB sObject;
char *file_spec;
{
	FILE *db_file;
	register int i, status = TRUE;

	if (!sObject)
		return FALSE;
	if (*file_spec == EOS)
		return FALSE;
	if ((db_file = fopen(file_spec, "w")) == NULL) {
		status = streamdb_CANT_WRITE;
		return status;
	}
	if (sObject->first) {
		EntryDB entry = sObject->first;

		while (entry) {
			if (entry->text && *entry->text) {
				fputs(sObject->delimiter, db_file);
				for (i = 0; entry->text[i] && status == TRUE; i++)
					status = (fputc(entry->text[i], db_file) != EOF);
			}
			entry = entry->next;
		}
	}
	fclose(db_file);
	return status;
}   /* write_streamdb_to_disk */


/*
write_text_to_disk() writes the text from a stream database to disk and
returns TRUE, if no errors occur.  You may want to use fwrite() or
fputs() here.
*/
/***********************************************************************
static int write_text_to_disk(sObject, file_spec, spacing, operation)
StreamDB sObject;
char *file_spec, *spacing;
int operation;
{
	FILE *db_file;
	register int i, status = TRUE;

	if (!sObject)
		return FALSE;
	if (!file_spec || !*file_spec)
	if (*file_spec == EOS)
		return FALSE;
	if ((db_file = fopen(file_spec, "w")) == NULL) {
		status = streamdb_CANT_WRITE;
		return status;
	}
	if (operation == streamdb_WRITE_CURRENT) {
		EntryDB entry = sObject->current;

		if (entry->text && *entry->text) {
			for (i = 0; entry->text[i] && status == TRUE; i++)
				status = (fputc(entry->text[i], db_file) != EOF);
		}
		if (spacing && *spacing) {
			for (i = 0; spacing[i] && status == TRUE; i++)
				status = (fputc(spacing[i], db_file) != EOF);
		}
	}
	else {
		if (sObject->first) {
			EntryDB entry = sObject->first;

			while (entry) {
				if (entry->text && *entry->text) {
					for (i = 0; entry->text[i] && status == TRUE; i++)
						status = (fputc(entry->text[i], db_file) != EOF);
				}
				if (spacing && *spacing) {
					for (i = 0; spacing[i] && status == TRUE; i++)
						status = (fputc(spacing[i], db_file) != EOF);
				}
				entry = entry->next;
			}
		}
	}
	fclose(db_file);
	return status;
}*/   /* write_text_to_disk */


/*
pipe_text_to_ptr() pipes the text from a stream database to the
specified print command and returns TRUE, if no errors occur.
*/

static int pipe_text_to_ptr(sObject, print_command, spacing, operation)
StreamDB sObject;
char *print_command, *spacing;
int operation;
{
	FILE *db_file;
	int status = TRUE;

	if (!sObject)
		return FALSE;
	if (!print_command || !*print_command)
		return FALSE;
	if ((db_file = popen(print_command, "w")) == NULL) {
		status = streamdb_CANT_WRITE;
		return status;
	}
	if (operation == streamdb_WRITE_CURRENT) {
		EntryDB entry = sObject->current;

		if (status == TRUE && entry->text && *entry->text)
			status = (fputs(entry->text, db_file) != EOF);
		if (status == TRUE && spacing && *spacing)
			status = (fputs(spacing, db_file) != EOF);
	}
	else {
		if (sObject->first) {
			EntryDB entry = sObject->first;

			while (entry) {
				if (status == TRUE && entry->text && *entry->text)
					status = (fputs(entry->text, db_file) != EOF);
				if (status == TRUE && spacing && *spacing)
					status = (fputs(spacing, db_file) != EOF);
				entry = entry->next;
			}
		}
	}
	pclose(db_file);
	return status;
}   /* pipe_text_to_ptr */


/*
file_size() checks for the existence of a file using stat(),
returning `streamdb_NO_FILE' for a nonexistent file, and the
file size otherwise.
*/

static int file_size(file_spec)
char *file_spec;
{
	struct stat	statbuf;

	if (stat(file_spec, &statbuf) < 0)
		return streamdb_NO_FILE;
	else
		return (int) statbuf.st_size;
}    /* file_size */


/*
scan_for_next_entry() finds the offset of the entry that's
one position closer to the end of the byte stream than the
current entry.
*/

static int scan_for_next_entry(sObject, buffer, offset)
StreamDB sObject;
char *buffer;
int offset;
{
	return string_search(buffer, sObject->delimiter,
		offset_without_delimiter(sObject, buffer, offset));
}	/* scan_for_next_entry */


/*
build_search_tree() traverses the linked list of database entries
building a binary tree with pointers to the database entries in
the data fields.
*/

static int build_search_tree(root, entry, order, line)
TreeNode *root;
EntryDB entry;
int order, line;
{
	while (entry) {
		if (!insert_into_tree(root, entry, order, line))
			return FALSE;
		entry = entry->next;
	}
	return TRUE;
}	/* build_search_tree */


/*
insert_into_tree() adds the next node.
*/

static int insert_into_tree(root, entry, order, line)
TreeNode *root;
EntryDB entry;
int order, line;
{
	TreeNode new_node, previous_node, next_node;

	next_node = *root;
	previous_node = NULL;
	while (next_node) {
		previous_node = next_node;
		next_node = go_left(entry, next_node, order, line) ?
			next_node->left : next_node->right;
	}
	if ((new_node = (TreeNode) malloc(sizeof(_TreeNode))) == NULL)
		return FALSE;
	new_node->database_entry = entry;
	new_node->left = NULL;
	new_node->right = NULL;
	if (!previous_node)
		*root = new_node;
	else {
		if (go_left(entry, previous_node, order, line))
			previous_node->left = new_node;
		else
			previous_node->right = new_node;
	}
	return TRUE;
}	/* insert_into_tree */


/*
go_left() tests the data to determine if the next entry should be
placed in the left subtree.
*/

static int go_left(entry, node, order, line)
EntryDB entry;
TreeNode node;
int order, line;
{
	char *entry_ptr, *database_entry_ptr;

	entry_ptr = move_forward_n_lines(entry->text, line - 1);
	database_entry_ptr =
		move_forward_n_lines(node->database_entry->text, line - 1);
	if (order == streamdb_ASCEND)
		return (strcmp(entry_ptr, database_entry_ptr) <= 0);
	else
		return (strcmp(entry_ptr, database_entry_ptr) >= 0);
}	/* go_left */


/*
move_forward_n_lines() advances n lines in a text stream.
It leaves the cursor at the beginning of the line.  It returns
a pointer to the remaining text segment.  The actual number of
lines moved may be less than that requested.
*/

static char *move_forward_n_lines(buffer, num_lines)
char *buffer;
int num_lines;
{
	register char *start = buffer;

	if (num_lines <= 0)
		return buffer;
	while (*buffer && num_lines > 0) {
		while (*buffer && *buffer != '\n')
			buffer++;
		if (*buffer) { /* must be a linefeed */
			buffer++;
			num_lines--;
		}
	}
	if (!*buffer) { /* return to beginning of line */
		while (buffer > start && *buffer != '\n')
			buffer--;
		if (*buffer == '\n')
			buffer++;
	}
	return buffer;
}   /* move_forward_n_lines */


/*
traverse_tree() traverses a binary tree of database entries,
using an inorder traversal to update the pointers in the
database entries.
*/

static void traverse_tree(node)
TreeNode node;
{
	if (node) {
		traverse_tree(node->left);
		update_linked_list(node);
		traverse_tree(node->right);
	}
}	/* traverse_tree */


/*
destroy_tree() traverses a tree and frees each node.
*/

static void destroy_tree(node)
TreeNode node;
{
	if (node) {
		destroy_tree(node->left);
		destroy_tree(node->right);
		free(node);
	}
}	/* destroy_tree */


/*
update_linked_list() sets the previous and next pointers in each
database entry.  The previous entry is held in a local static
variable.  It sets to file-level global variables for the first
and last entries in the list.
*/

static void update_linked_list(node)
TreeNode node;
{
	static EntryDB prev_entry;

	/*
	if `first_entry' is NULL, traversal has descended to the
	first entry	in the database, not the first node in the tree:
	*/
	if (!first_entry) {
		first_entry = node->database_entry;
		prev_entry = NULL;
	}
	node->database_entry->previous = prev_entry;
	if (prev_entry)
		prev_entry->next = node->database_entry;
	prev_entry = node->database_entry;
	/*
	`num_nodes' is set in streamdb_sort() to the total number of
	entries.  When it equals 1, all tree nodes have been visited.
	*/
	if (num_nodes-- == 1) {
		last_entry = node->database_entry;
		last_entry->next = NULL;
	}
}	/* update_linked_list */

