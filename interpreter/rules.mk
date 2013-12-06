#######################################################################
# This is a set of common rules for building Alan Interpreters
# It is included from Makefile to complement rules and settings
# specific to the host it is run on, set in such files as
# Makefile.thoni, Makefile.cygwin, Makefile.Darwin, also included from
# Makefile
# This file will build a standard command line arun and all unittests

BUILD := $(shell if [ -f ../BUILD_NUMBER ] ; then cat ../BUILD_NUMBER; else echo 0; fi)

CC = $(COMPILER)
CFLAGS	= $(COMPILEFLAGS) $(EXTRA_COMPILER_FLAGS) $(WARNINGFLAGS) -DBUILD=$(BUILD) $(OSFLAGS)

LINK = $(LINKER)
LDFLAGS = $(LINKFLAGS) $(EXTRA_LINKER_FLAGS) $(OSFLAGS)

# Default top rule if platform specific makefile doesn't add a default
# that is found before this
all: unit arun

#######################################################################
build: arun

#######################################################################
.PHONY: unit
ifneq ($(CGREEN),yes)
unit:
	echo "No unit tests run, cgreen not available"
else
unit: cgreenrunnertests isolated_unittests
endif

#######################################################################
.PHONY: clean
clean:
	-rm *.o .*/*.o .*/*.d


###################################################################
#
# Run all tests!
# Interpreter is tested through the regressions tests
ifneq ($(EMACS),)
JREGROUTPUT = -noansi
endif

.PHONY: test
test:
	@cd ..;java -jar bin/jregr.jar -bin bin -dir regression $(JREGROUTPUT)
#	@cd ..;java -jar bin/jregr.jar -bin bin -dir regression/versions/compiler $(JREGROUTPUT)
	@cd ..;java -jar bin/jregr.jar -bin bin -dir regression/versions/interpreter $(JREGROUTPUT)
	@cd ..;java -jar bin/jregr.jar -bin bin -dir regression/tracing $(JREGROUTPUT)
	@cd ..;java -jar bin/jregr.jar -bin bin -dir regression/saving $(JREGROUTPUT)
	@cd ..;java -jar bin/jregr.jar -dir regression/restore $(JREGROUTPUT)		# Uses sh not the executables

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

# Dependencies, if they don't exist yet
-include $(UNITTESTS_USING_MAIN_OBJECTS:.o=.d)

# Rule to compile objects to subdirectory
$(UNITTESTSOBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -MMD -o $@ -c $<

# Create directory if it doesn't exist
$(UNITTESTSOBJDIR):
	@mkdir $(UNITTESTSOBJDIR)

# Build the DLL...
unittests.dll: LIBS = $(CGREENLIB)
unittests.dll: $(UNITTESTSOBJDIR) $(UNITTESTS_USING_RUNNER_OBJECTS) sources.mk
	$(LINK) -shared -o $@ $(LDFLAGS) $(UNITTESTS_USING_RUNNER_OBJECTS) $(LINKFLAGS) $(LIBS)

# ... that can be run with the cgreen runner
cgreenrunnertests: CFLAGS += $(CGREENINCLUDE)
cgreenrunnertests: LIBS = $(CGREENLIB) $(ALLOCLIBS)
cgreenrunnertests: unittests.dll
ifeq ($(shell uname), Darwin)
	arch -i386 cgreen-runner $^ --suite interpreter_unit_tests $(UNITOUTPUT)
else
	cgreen-runner ./$^ --suite interpreter_unit_tests $(UNITOUTPUT)
endif

# Here we try to build a runnable DLL for each module where it can be 
# tested in total isolation (with everything else mocked away,
# except lists.c and memory.c)

-include $(addprefix $(UNITTESTSOBJDIR)/,$(patsubst %,%.d,$(MODULES_WITH_ISOLATED_UNITTESTS)))
-include $(addprefix $(UNITTESTSOBJDIR)/,$(patsubst %,%_tests.d,$(MODULES_WITH_ISOLATED_UNITTESTS)))

ISOLATED_UNITTESTS_EXTRA_OBJS = $(addprefix $(UNITTESTSOBJDIR)/, $(addsuffix .o, lists utils options))

# A test .dll for a module is built from its .o and the _test.o (and some extras)
$(UNITTESTSOBJDIR)/%_tests.dll: $(UNITTESTSOBJDIR)/%.o $(UNITTESTSOBJDIR)/%_tests.o
	$(LINK) -shared -o $@ $(ISOLATED_UNITTESTS_EXTRA_OBJS) $^ $(LDFLAGS) $(LIBS)

ISOLATED_UNITTESTS_DLLS = $(addprefix $(UNITTESTSOBJDIR)/,$(patsubst %,%_tests.dll,$(MODULES_WITH_ISOLATED_UNITTESTS)))

# Then run all _tests.dll's with the cgreen-runner
isolated_unittests: CFLAGS += $(CGREENINCLUDE)
isolated_unittests: LIBS = $(CGREENLIB)
isolated_unittests: $(UNITTESTSOBJDIR) $(ISOLATED_UNITTESTS_EXTRA_OBJS) $(ISOLATED_UNITTESTS_DLLS)
ifeq ($(shell uname), Darwin)
	@for f in $(ISOLATED_UNITTESTS_DLLS) ; do \
		arch -i386 cgreen-runner $$f --suite Interpreter $(UNITOUTPUT) ; \
	done
else
	@for f in $(ISOLATED_UNITTESTS_DLLS) ; do \
		cgreen-runner $$f --suite Interpreter $(UNITOUTPUT) ; \
	done
endif


# Extra dependencies for WinGLK case, really needed? How to make them work in subdirs?
readline.o : resources.h
glkstart.o: glkstart.c args.h types.h sysdep.h acode.h main.h \
  glkstart.h glkio.h resources.h utils.h
glkio.o: glkio.c glkio.h
