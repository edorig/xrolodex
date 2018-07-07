/**** streamdb.h ****/  /**** formatted with 4-column tabs ****/

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


#ifndef _streamdb_h
#define _streamdb_h

#include <stdio.h>

#include "strstr.h"
#include "streamdbP.h"


/*
Constants:
*/

#define streamdb_NO_FILE		-1
#define streamdb_CANT_READ		-2
#define streamdb_CANT_WRITE		-3
#define streamdb_NO_MATCH		-4
#define streamdb_MOD_SAVE		-5
#define streamdb_FORCE_SAVE		-6
#define streamdb_WRAPAROUND		-7
#define streamdb_NO_WRAPAROUND	-8
#define streamdb_WRITE_CURRENT	-9
#define streamdb_WRITE_ALL		-10

/*
Used to request that a text segment be inserted
at the end of the stream.
*/

#define streamdb_CURRENT	-101
#define streamdb_APPEND		-102

#define streamdb_ASCEND		-201
#define streamdb_DESCEND	-202


/*
Public opaque pointers:
*/

typedef _StreamDB *StreamDB;
typedef _EntryDB *EntryDB;


/*
Public functions:
*/

extern StreamDB streamdb_create();
extern void streamdb_destroy();
extern int streamdb_modified();
extern int streamdb_get_num_entries();
extern EntryDB streamdb_get_first();
extern EntryDB streamdb_get_last();
extern EntryDB streamdb_get_previous();
extern EntryDB streamdb_get_next();
extern EntryDB streamdb_get_entry_by_position();
extern EntryDB streamdb_get_current();
extern EntryDB streamdb_set_current();
extern char *streamdb_get_current_text();
extern char *streamdb_set_current_text();
extern char *streamdb_get_entry_text();
extern char *streamdb_get_entry_text_begin_line();
extern char *streamdb_get_delimiter();
extern char *streamdb_set_delimiter();
extern EntryDB streamdb_create_solitaire();
extern void streamdb_free_solitaire();
extern EntryDB streamdb_load();
extern int streamdb_save();
extern int streamdb_save_as();
extern int streamdb_save_alt();
extern char *streamdb_set_filename();
extern EntryDB streamdb_insert();
extern EntryDB streamdb_delete();
extern EntryDB streamdb_search_forward();
extern EntryDB streamdb_search_backward();
extern EntryDB streamdb_sort();
extern void streamdb_print();
extern void streamdb_print_all();
extern int streamdb_print_ptr();
extern int streamdb_print_ptr_all();
extern int file_exists();



#endif /* _streamdb_h */
