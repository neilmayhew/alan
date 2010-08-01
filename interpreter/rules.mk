#######################################################################
# This is a set of common rules for building Alan Interpreters
# It is included from Makefile to complement rules and settings
# specific to the host it is run on, set in such files as
# Makefile.thoni, Makefile.cygwin, Makefile.Darwin, also included from
# Makefile
# This file will build a standard command line arun and all unittests

CFLAGS	= $(COMPILEFLAGS) $(WARNINGFLAGS)
LDFLAGS = $(LINKFLAGS)

#######################################################################
# Standard console Arun
ARUNOBJDIR = .arun
ARUNOBJECTS = $(addprefix $(ARUNOBJDIR)/,${ARUNSRCS:.c=.o})

# Dependencies
-include $(ARUNOBJECTS:.o=.d)

# Rule to compile objects to subdirectory
$(ARUNOBJECTS): $(ARUNOBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -MMD -o $@ -c $<

$(ARUNOBJDIR):
	@mkdir $(ARUNOBJDIR)

arun: $(ARUNOBJDIR) $(ARUNOBJECTS)
	$(LINK) -o $@ $(LINKFLAGS) $(ARUNOBJECTS) $(LIBS)
	cp $@ ../bin/

#######################################################################
#
# CGreen unit tests
#
# Define
#	CGREEN to something ("yes") to build with cgreen unit tests
#	CGREENINCLUDE so that #include "cgreen/cgreen.h" works if needed
#	CGREENLIB to something to link with (e.g. -lcgreen)

UNITTESTSOBJDIR = .unittests
UNITTESTSOBJECTS = $(addprefix $(UNITTESTSOBJDIR)/,${UNITTESTSSRCS:.c=.o})

# Dependencies, if they exist yet
-include $(UNITTESTSOBJECTS:.o=.d)

# Rule to compile objects to subdirectory
$(UNITTESTSOBJECTS): $(UNITTESTSOBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -MMD -o $@ -c $<

#Create directory if it doesn't exist
$(UNITTESTSOBJDIR):
	@mkdir $(UNITTESTSOBJDIR)

unittests: CFLAGS += $(CGREENINCLUDE)
unittests: LIBS = $(CGREENLIB)
unittests: $(UNITTESTSOBJDIR) $(UNITTESTSOBJECTS)
	$(LINK) -o $@ $(LINKFLAGS) $(UNITTESTSOBJECTS) $(LIBS)

.PHONY: unit
ifneq ($(CGREEN),yes)
unit:
	echo "No unit tests run, cgreen not available"
else
unit: unittests
	@echo
	@echo "        Unit Tests"
	@echo
	@./unittests
	@echo
	@echo
endif

#######################################################################
.PHONY: clean
clean:
	-rm *.o .*/*.o


###################################################################
#
# Version number file generation
#
alan.version.h : ../alan.version.h
	cp ../alan.version.h .

alan.version.c:  ../alan.version.c
	cp ../alan.version.c .

version.h : ../version.h
	cp ../version.h .

../alan.version.c ../alan.version.h ../version.h: ../alan.version

../alan.version:
	cd ..; venum alan time


###################################################################
#
# Run all tests!
# No tests except unit tests are available
#
.PHONY: test
test: unit


###################################################################
# NB! We are moving towards using the gcc -MMD auto-dependency!!
# So the manual depend target is getting obsolete
# Automatically generate dependency rules 
%.d : %.c 
	$(CC) $(CCFLAGS) -MF"$@" -MG -MM -MP -MT"$@" -MT"$(<:.c=.o)" "$<" 

# -MF  write the generated dependency rule to a file 
# -MG  assume missing headers will be generated and don't stop with an error 
# -MM  generate dependency rule for prerequisite, skipping system headers 
# -MP  add phony target for each header to prevent errors when header is missing 
# -MT  add a target to the generated dependency 

# Extra dependencies for WinGLK case, really needed? How to make them work in subdirs?
readline.o : resources.h
glkstart.o: glkstart.c args.h types.h sysdep.h acode.h main.h \
  glkstart.h glkio.h resources.h utils.h
glkio.o: glkio.c glkio.h
