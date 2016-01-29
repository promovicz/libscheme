#
# Makefile for libscheme
#

#
# This must be an ANSI C compiler.
#
CC=cc

#
# Optimization and debugging flags go here.
#
CFLAGS=-O

#
# The math library is needed for the numeric functions
# in scheme_number.c.
#
LIBS=-lm

#
# If your system needs ranlib, put it here.  Otherwise,
# use a colon.
#
RANLIB=:

OBJS =  scheme_alloc.o \
	scheme_bool.o \
	scheme_char.o \
	scheme_env.o \
	scheme_error.o \
	scheme_eval.o \
	scheme_fun.o \
	scheme_hash.o \
	scheme_list.o \
	scheme_number.o \
	scheme_port.o \
	scheme_print.o \
	scheme_promise.o \
	scheme_read.o \
	scheme_string.o \
	scheme_struct.o \
	scheme_symbol.o \
	scheme_syntax.o \
	scheme_type.o \
	scheme_vector.o

SRCS =  scheme_alloc.c \
	scheme_bool.c \
	scheme_char.c \
	scheme_env.c \
	scheme_error.c \
	scheme_eval.c \
	scheme_fun.c \
	scheme_hash.c \
	scheme_list.c \
	scheme_number.c \
	scheme_port.c \
	scheme_print.c \
	scheme_promise.c \
	scheme_read.c \
	scheme_string.c \
	scheme_struct.c \
	scheme_symbol.c \
	scheme_syntax.c \
	scheme_type.c \
	scheme_vector.c

libscheme.a: $(OBJS) gc/gc.a
	$(AR) rv libscheme.a $(OBJS) gc/*.o
	$(RANLIB) libscheme.a

gc/gc.a:
	cd gc; $(MAKE)

test: libscheme.a main.o 
	$(CC) $(CFLAGS) -o test main.o libscheme.a -lm

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

clean:
	/bin/rm -f $(OBJS) main.o libscheme.a test *~ \
	libscheme.aux libscheme.dvi libscheme.log tmp1 tmp2 tmp3
	cd gc; $(MAKE) clean

# DO NOT DELETE THIS LINE -- make depend depends on it.
