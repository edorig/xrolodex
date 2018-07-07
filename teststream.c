/**** teststream.c ****/  /**** formatted with 4-column tabs ****/


#include "streamdb.h"



/*
main() initializes the stream database, loads/builds a database in
memory, adds entries, deletes entries, saves the database to an
alternate file, and terminates, printing the database at each stage.
*/

void main(argc, argv)
int argc;
char *argv[];
{
	char *str1 = "First insertion--appended to end.\n";
	char *str2 = "Second insertion,\noccupying two lines--at pos. 2.\n";
	char *str3 = "Third insertion--at current pos.\n";
	char *str4 = "Fourth insertion--at pos. 0.\n";
	char *str5 = "Fifth insertion--at pos. 1.\n";
	char *str6 = "Sixth insertion--at current pos.\n";
	char *spacing = "";
	StreamDB db;
	EntryDB current;

	db = streamdb_create("teststream.file", "####\n");
	streamdb_load(db);

	streamdb_insert(db, str1, streamdb_APPEND);
	printf("After first insertion the contents are:\n");
	streamdb_print(db, spacing);
	printf("\n");

	current = streamdb_set_current(db, streamdb_insert(db, str2, 2));
	printf("Current entry's text is: %s",
		streamdb_get_current_text(db));
	printf("\n");
	streamdb_set_current_text(db, "Replacement text for 2nd insertion.\n");
	printf("Current entry's text is: %s",
		streamdb_get_current_text(db));
	printf("\n");
	printf("After second insertion the contents are:\n");
	streamdb_print(db, spacing);
	printf("\n");

	streamdb_insert(db, str3, streamdb_CURRENT);
	printf("After third insertion the contents are:\n");
	streamdb_print(db, spacing);
	printf("\n");

	streamdb_insert(db, str4, 0);
	printf("After fourth insertion the contents are:\n");
	streamdb_print(db, spacing);
	printf("\n");

	streamdb_insert(db, str5, 1);
	printf("After fifth insertion the contents are:\n");
	streamdb_print(db, spacing);
	printf("\n");

	streamdb_insert(db, str6, streamdb_CURRENT);
	printf("After sixth insertion the contents are:\n");
	streamdb_print(db, spacing);
	printf("\n");

	streamdb_delete(db, 0);
	printf("After deleting entry 0 the contents are:\n");
	streamdb_print(db, spacing);
	printf("\n");

	streamdb_delete(db, 3);
	printf("After deleting entry 3 the contents are:\n");
	streamdb_print(db, spacing);
	printf("\n");

	streamdb_delete(db, 100);
	printf("After deleting entry 100 the contents are:\n");
	streamdb_print(db, spacing);
	printf("\n");

	streamdb_save_alt(db, "teststream.new");
	printf("The database has been saved to an alternate disk file.\n");
	printf("Database modified? %s\n", streamdb_modified(db) ? "Yes" : "No");
	printf("\n");
	streamdb_save(db, streamdb_MOD_SAVE);
	printf("The database has been saved.\n");
	printf("Database modified? %s\n", streamdb_modified(db) ? "Yes" : "No");
	printf("\n");
	streamdb_destroy(db);
	exit(0);
}	/* main */

