#### xrolo.make ####    #### formatted with 4-column tabs ####
####
#### This is a manually-pick-what-you-like, pedestrian-style make file.
#### Alternatively, you can install xrolodex in your MIT source tree and
#### use `Imakefile' to build xrolodex.
####

# The C compiler:
CC = cc

# This preprocessor constant M U S T be defined; see REQD_CFLAGS below:
#
# MANUAL_CLIP is in `editor.c'
#

# EDITRES compiler flag (in `xrolo.c' to include `editres' support -- X11R5/6
# only.  If you enable this flag, you'll probably need to link in the X
# miscellaneous-utilities library; see "Generic Xmu library flags" below):
X_FLAGS = 
#X_FLAGS = -DEDITRES

# REQD_CFLAGS -- required compiler flags:
REQD_CFLAGS = -D_NO_PROTO -DMANUAL_CLIP

# EXTRA_CFLAGS -- extra compiler flags:
EXTRA_CFLAGS = 
# Whether or not the application physically restricts the action
# area paned windows, e.g., in dialogs, the help window, etc. in
# lieu of using `XmNskipAdjust'.  Enabling this code causes start-up
# resize problems with Motif 2.0:
#EXTRA_CFLAGS = -DRESTRICT_ACTION_AREA
# Use cp(1) and system(3) instead of rename(2) to create back-up files:
#EXTRA_CFLAGS = -DBACKUP_CP
# Substitute an alternate, do-nothing error handler for the Xt standard:
#EXTRA_CFLAGS = -DALT_ERROR_HANDLER
# Enable the fallback resources (this may cause core dumps on some
# platforms):
EXTRA_CFLAGS = -DFALLBACK_RES

# INCLUDES -- extra compiler flags:
INCLUDES = -I/usr/include/Xm
# HP includes:
#INCLUDES = -I/usr/include/Motif1.2 -I/usr/include/X11R5

# Sample generic compiler flags:
CFLAGS = $(INCLUDES) $(REQD_CFLAGS) $(EXTRA_CFLAGS) $(X_FLAGS)
# Sample HP compiler flags:
#CFLAGS = -Wp,-H300000 $(INCLUDES) $(REQD_CFLAGS) $(EXTRA_CFLAGS) $(X_FLAGS)
# Sample SGI compiler flags:
#CFLAGS = -Wf,-XNl12000 $(INCLUDES) $(REQD_CFLAGS) $(EXTRA_CFLAGS) $(X_FLAGS)

# Generic loader flags:
LDFLAGS = 
# Sample HP loader flags:
#LDFLAGS = -L/usr/lib/Motif1.2 -L/usr/lib/X11R5
# Sample Linux loader flags:
#LDFLAGS = -L/usr/X11R6/lib

# Generic Xt library flags:
XTLIB = -lXt
# If it's not `Xt', name your Xt library, e.g.,:
#XTLIB = -lXtm

# Generic Xmu library flags:
XMULIB = 
# If you need 'Xmu', uncomment this line:
#XMULIB = -lXmu

# Generic Xpm library flags for Linux:
XPMLIB = 
# If you need 'Xpm', uncomment this line:
#XPMLIB = -lXpm

# If you need other libraries, name them:
OTHERLIBS = 
# Sample DG, SGI other libraries:
#OTHERLIBS = -lPW

# HP static loader flags:
#STATIC = -Wl,-a,archive
# Sun static loader flags:
#STATIC = -Bstatic


xrolodex:	xrolo.o xrolo.db.o xrolo.index.o help.o streamdb.o \
			ctrlpanel.o editor.o listshell.o dialog.o strstr.o motif.o
			$(CC) $(LDFLAGS) -o xrolodex xrolo.o xrolo.db.o xrolo.index.o \
			help.o streamdb.o ctrlpanel.o editor.o \
			listshell.o dialog.o strstr.o motif.o -lXm $(XPMLIB) $(XMULIB) \
			$(XTLIB) -lX11 $(OTHERLIBS)

xrolodex.static:	xrolo.o xrolo.db.o xrolo.index.o help.o streamdb.o \
					ctrlpanel.o editor.o listshell.o dialog.o strstr.o motif.o
					$(CC) $(STATIC) $(LDFLAGS) -o xrolodex xrolo.o xrolo.db.o \
					xrolo.index.o help.o streamdb.o ctrlpanel.o editor.o \
					listshell.o dialog.o strstr.o motif.o -lXm $(XPMLIB) \
					$(XMULIB) $(XTLIB) -lX11 $(OTHERLIBS)
