args.o: args.c args.h arun.h types.h sysdep.h acode.h
arun.o: arun.c sysdep.h types.h acode.h arun.h readline.h version.h \
 args.h parse.h inter.h rules.h debug.h stack.h exe.h term.h c25to26.h
c25to26.o: c25to26.c types.h sysdep.h acode.h arun.h c25to26.h
debug.o: debug.c types.h sysdep.h acode.h version.h readline.h inter.h \
 arun.h parse.h exe.h debug.h
decode.o: decode.c arun.h types.h sysdep.h acode.h decode.h
dumpacd.o: dumpacd.c types.h sysdep.h acode.h spa.h
exe.o: exe.c types.h sysdep.h acode.h readline.h arun.h parse.h \
 inter.h stack.h decode.h exe.h
genalan.o: genalan.c spa.h
inter.o: inter.c types.h sysdep.h acode.h arun.h parse.h exe.h stack.h \
 inter.h
params.o: params.c types.h sysdep.h acode.h params.h
parse.o: parse.c types.h sysdep.h acode.h readline.h arun.h inter.h \
 exe.h term.h debug.h params.h parse.h
readline.o: readline.c readline.h types.h sysdep.h acode.h arun.h
reverse.o: reverse.c types.h sysdep.h acode.h arun.h reverse.h
rules.o: rules.c types.h sysdep.h acode.h arun.h inter.h debug.h exe.h \
 stack.h rules.h
spa.o: spa.c spa.h
stack.o: stack.c types.h sysdep.h acode.h arun.h stack.h
sysdep.o: sysdep.c sysdep.h
term.o: term.c arun.h types.h sysdep.h acode.h term.h
version.o: version.c version.h
