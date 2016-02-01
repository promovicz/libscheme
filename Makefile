#
# Makefile for libscheme
#

#
# This must be an ANSI C compiler.
#
CC?=cc

#
# Determine compiler include paths.
#
CCINCLUDE:=$(shell ./conf-includes $(CC))
CCINCLUDEFLAGS:=$(CCINCLUDE:%=-I%)

CFLAGS?=-I.

#
# Optimization and debugging flags go here.
#
CFLAGS+=-O3 -g

#
# We want lots of warnings.
#
CFLAGS+=-Wall -Wextra -Wno-unused-parameter

#
# GCC can suggest pure, const and noreturn candidate functions.
#
#CFLAGS+=-Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn

#
# The math library is needed for the numeric functions
# in scheme_number.c.
#
LIBS+=-lm

#
# We also use the boehm-weiser garbage collector
#
LIBS+=-lgc

#
# If your system needs ranlib, put it here.  Otherwise,
# use a colon.
#
RANLIB=:

#
# We currently use good old makedepend.
#
MAKEDEPEND=makedepend

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

SRCS = $(patsubst %.o,%.c,$(OBJS))

scheme: libscheme.a main.o
	$(CC) $(CFLAGS) $(LIBS) -o scheme main.o libscheme.a

libscheme.a: $(OBJS)
	$(AR) rv libscheme.a $(OBJS)
	$(RANLIB) libscheme.a

depend:
	$(MAKEDEPEND) $(CCINCLUDEFLAGS) -- $(CFLAGS) -- $(SRCS)
test: scheme
	./scheme test.scm
	rm tmp1 tmp2 tmp3
.PHONY: test


clean:
	$(RM) -f $(OBJS) scheme main.o libscheme.a \
	libscheme.aux libscheme.dvi libscheme.log tmp1 tmp2 tmp3

# DO NOT DELETE THIS LINE -- make depend depends on it.
