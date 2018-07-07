#### formatted with 4-column tabs ####

X_FLAGS = 

# REQD_CFLAGS -- required compiler flags:
REQD_CFLAGS = -D_NO_PROTO

# EXTRA_CFLAGS -- extra compiler flags:
EXTRA_CFLAGS = 

# INCLUDES -- extra compiler flags:
INCLUDES = -I/usr/include/Xm
# HP includes:
#INCLUDES = -I/usr/include/Motif1.2 -I/usr/include/X11R5

# Sample generic compiler flags:
CFLAGS = $(INCLUDES) $(REQD_CFLAGS) $(EXTRA_CFLAGS) $(X_FLAGS)

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
# Sample DG,SGI other libraries:
#OTHERLIBS = -lPW

testctrl:	testctrl.o ctrlpanel.o
			cc $(LDFLAGS) -o testctrl testctrl.o ctrlpanel.o \
			-lXm $(XPMLIB) $(XMULIB) $(XTLIB) -lX11 $(OTHERLIBS)
