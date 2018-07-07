/**** streamdbP.h ****/  /**** formatted with 4-column tabs ****/

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
	streamdb_save(db);
	printf("Database modified? %s\n", streamdb_modified(db) ? "Yes" : "No");
	streamdb_delete(db, 0);
	streamdb_delete(db, 2);
	streamdb_save_alt(db, "teststream.alt");
	printf("Database modified? %s\n", streamdb_modified(db) ? "Yes" : "No");
	streamdb_destroy(db);
	exit(0);
}

*************************************************************************/


#ifndef _streamdbP_h
#define _streamdbP_h

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>


/*
Constants:
*/

#define streamdb_MAX_FILENAME_SPEC	1024

#define TRUE	1
#define FALSE	0
#define EOS		'\0'


/*
Private structures for the StreamDB object:
*/

typedef struct enode {
	char *text;
	struct enode *previous, *next;
} _EntryDB;


typedef struct {
	void *self;
	char filename[streamdb_MAX_FILENAME_SPEC + 1];
	int file_modified;
	int num_entries;
	int delimiter_len;
	_EntryDB *first;
	_EntryDB *last;
	_EntryDB *current;
	char *delimiter;
} _StreamDB;


/*
tree_node is used during sorting to build a binary search tree
for the elements of the linked list database.
*/

typedef struct tnode {
	_EntryDB *database_entry;
	struct tnode *left, *right;
} _TreeNode, *TreeNode;


#endif /* _streamdbP_h */
