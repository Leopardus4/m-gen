# This script is part of 'm-gen' project.
# To compile it:
#    - select your compiler below,
#    - open console and type:
#
#        cd "/path/to/m-gen/directory/"
#
#        make all
#
#    - Linux / macOS:
#
#        sudo make install
#
#    - Windows - you must manually copy file bin/m-gen
#    to C:\new_folder\ and add it to PATH variable





# Select your compiler (uncomment)
COMPILER := cc
#COMPILER := gcc
#COMPILER := i686-w64-mingw32-gcc-win32
#COMPILER :=
# others...




# programs:

ECHO := echo

MKDIR := mkdir -p   # UNIX
# MKDIR := mkdir      # Windows

CP := cp

RM := rm -f

COWSAY := cowsay
# COWSAY := echo









# directories

OBJDIR := obj/$(COMPILER)

BINDIR := bin/$(COMPILER)

INSTALLPATH := /usr/local/bin/



# searching for all "target's" source files in targets/ directory

TARGETDIR := targets

TARGETS_C := $(wildcard $(TARGETDIR)/*.c)

TARGETS_H := $(TARGETS_C:%.c=%.h)

TARGETS_O := $(TARGETS_C:%.c=%.o)





# main file
MAIN := m-gen

# file with utilities
UTIL := gm-utils

# file with common functions
COMMON := gm-common


# Program name
PROGRAM := m-gen



_OBJS := $(MAIN).o $(COMMON).o $(UTIL).o $(TARGETS_O)
OBJS := $(_OBJS:%=$(OBJDIR)/%)



#ADDITIONAL FLAGS GET FORM COMMAND LINE (i.e. by 'make all CFLAGS_COMMAND_LINE=-...' )
CFLAGS_COMMAND_LINE :=


# compiler flags
CFLAGS := 	-Wall			\
			-O2 			\
			-std=c99		\
			-I$(TARGETDIR)	\
			-I.				\
			$(CFLAGS_COMMAND_LINE)





# linker flags
LFLAGS :=	$(CFLAGS)








salad:
	@ $(ECHO) "make salad: Give me a chicken"

salad_without_chicken:
	@ cowsay Creating vege salad ... Fatal error: cannot find potatoes.



# general routine

all: create_dirs $(BINDIR)/$(PROGRAM)


create_dirs:
	@ $(MKDIR) "$(OBJDIR)/$(TARGETDIR)"
	@ $(MKDIR) "$(BINDIR)"



$(BINDIR)/$(PROGRAM): $(OBJS)
	$(COMPILER) $(LFLAGS) $^ -o $@



# main file compilation

$(OBJDIR)/$(MAIN).o: $(MAIN).c $(MAIN).h $(UTIL).h $(COMMON).h $(TARGETS_H)
	$(COMPILER) -c $(CFLAGS) $< -o $@



# UTIL file compilation

$(OBJDIR)/$(UTIL).o: $(UTIL).c $(UTIL).h $(MAIN).h
	$(COMPILER) -c $(CFLAGS) $< -o $@



# COMMON file compilation

$(OBJDIR)/$(COMMON).o: $(COMMON).c $(COMMON).h $(MAIN).h
	$(COMPILER) -c $(CFLAGS) $< -o $@



# targets files compilation

$(OBJDIR)/$(TARGETDIR)/%.o: $(TARGETDIR)/%.c $(TARGETDIR)/%.h $(MAIN).h $(COMMON).h
	$(COMPILER) -c $(CFLAGS) $< -o $@




# installation - needs root ( 'sudo make install' )

install: $(BINDIR)/$(PROGRAM)
	$(CP) $< $(INSTALLPATH)
	@ $(ECHO) "$(PROGRAM) installed in $(INSTALLPATH)"


clean:
	$(RM) $(OBJS)
	$(RM) $(BINDIR)/$(PROGRAM)


comments_are_bad:
	$(COMPILER) -E $(MAIN).c -o tmp.txt
	@ cat tmp.txt | sed "s/(/h;<£70#+]/"
	rm -f tmp.txt


