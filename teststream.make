#### formatted with 4-column tabs ####

CFLAGS =
LDFLAGS =

teststream:	teststream.o streamdb.o strstr.o
			cc $(LDFLAGS) -o teststream teststream.o streamdb.o strstr.o
