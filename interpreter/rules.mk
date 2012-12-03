#######################################################################
# This is a set of common rules for building Alan Interpreters
# It is included from Makefile to complement rules and settings
# specific to the host it is run on, set in such files as
# Makefile.thoni, Makefile.cygwin, Makefile.Darwin, also included from
# Makefile
# This file will build a standard command line arun and all unittests

BUILD := $(shell if [ -f ../BUILD_NUMBER ] ; then cat ../BUILD_NUMBER; else echo 0; fi)

CFLAGS	= $(COMPILEFLAGS) $(EXTRA_COMPILER_FLAGS) $(WARNINGFLAGS) -DBUILD=$(BUILD) $(OSFLAGS)
LDFLAGS = $(LINKFLAGS) $(EXTRA_LINKER_FLAGS) $(OSFLAGS)

#######################################################################
# Standard console Arun
ARUNOBJDIR = .arun
ARUNOBJECTS = $(addprefix $(ARUNOBJDIR)/,${ARUNSRCS:.c=.o}) $(ARUNOBJDIR)/alan.version.o

# Dependencies
-include $(ARUNOBJECTS:.o=.d)

# Rule to compile objects to subdirectory
$(ARUNOBJECTS): $(ARUNOBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -MMD -o $@ -c $<

$(ARUNOBJDIR):
	@mkdir $(ARUNOBJDIR)

arun: $(ARUNOBJDIR) $(ARUNOBJECTS)
	$(LINK) -o $@ $(LDFLAGS) $(ARUNOBJECTS) $(LIBS)
	cp $@ ../bin/

#######################################################################
# Standard console Arun with gcov
gcov: EXTRA_COMPILER_FLAGS = -fprofile-arcs -ftest-coverage
gcov: EXTRA_LINKER_FLAGS = -fprofile-arcs -ftest-coverage
GCOVOBJDIR = .gcov
GCOVOBJECTS = $(addprefix $(GCOVOBJDIR)/,${ARUNSRCS:.c=.o}) $(GCOVOBJDIR)/alan.version.o

# Dependencies
-include $(GCOVOBJECTS:.o=.d)

# Rule to compile objects to subdirectory
$(GCOVOBJECTS): $(GCOVOBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -MMD -o $@ -c $<

$(GCOVOBJDIR):
	@mkdir $(GCOVOBJDIR)

gcov: $(GCOVOBJDIR) $(GCOVOBJECTS)
	$(LINK) -o $@ $(LDFLAGS) $(GCOVOBJECTS) $(LIBS)
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
UNITTESTS_USING_MAIN_OBJECTS = $(addprefix $(UNITTESTSOBJDIR)/,${UNITTESTS_USING_MAIN_SRCS:.c=.o}) $(UNITTESTSOBJDIR)/alan.version.o
UNITTESTS_USING_RUNNER_OBJECTS = $(addprefix $(UNITTESTSOBJDIR)/,${UNITTESTS_USING_RUNNER_SRCS:.c=.o}) $(UNITTESTSOBJDIR)/alan.version.o
UNITTESTS_ALL_OBJECTS = $(addprefix $(UNITTESTSOBJDIR)/,${UNITTESTS_ALL_SRCS:.c=.o}) $(UNITTESTSOBJDIR)/alan.version.o

# Dependencies, if they exist yet
-include $(UNITTESTS_USING_MAIN_OBJECTS:.o=.d)

# Rule to compile objects to subdirectory
$(UNITTESTS_ALL_OBJECTS): $(UNITTESTSOBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -MMD -o $@ -c $<

# Create directory if it doesn't exist
$(UNITTESTSOBJDIR):
	@mkdir $(UNITTESTSOBJDIR)

unittests: CFLAGS += $(CGREENINCLUDE)
unittests: LIBS = $(CGREENLIB)
unittests: $(UNITTESTSOBJDIR) $(UNITTESTS_USING_MAIN_OBJECTS)
	$(LINK) -o $@ $(LDFLAGS) $(UNITTESTS_USING_MAIN_OBJECTS) $(LIBS)
	@./unittests $(UNITOUT)

cgreenrunnertests: CFLAGS += $(CGREENINCLUDE)
cgreenrunnertests: LIBS = $(CGREENLIB) $(ALLOCLIBS)
cgreenrunnertests: $(UNITTESTSOBJDIR) $(UNITTESTS_USING_RUNNER_OBJECTS) run_runner_tests
	$(LINK) -shared -o unittests.dll $(LDFLAGS) $(UNITTESTS_USING_RUNNER_OBJECTS) $(LINKFLAGS) $(LIBS)

.PHONY: run_runner_tests
run_runner_tests:
ifeq ($(shell uname), Darwin)
	arch -i386 cgreen-runner unittests.dll $(UNITOUT)
else
	cgreen-runner ./unittests.dll $(UNITOUT)
endif

.PHONY: unit
ifneq ($(CGREEN),yes)
unit:
	echo "No unit tests run, cgreen not available"
else
unit: unittests cgreenrunnertests
endif

#######################################################################
.PHONY: clean
clean:
	-rm *.o .*/*.o .*/*.d


###################################################################
#
# Run all tests!
# No tests except unit tests are available
#
.PHONY: test
test: unit


# Extra dependencies for WinGLK case, really needed? How to make them work in subdirs?
readline.o : resources.h
glkstart.o: glkstart.c args.h types.h sysdep.h acode.h main.h \
  glkstart.h glkio.h resources.h utils.h
glkio.o: glkio.c glkio.h
