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

#
# Use local includes.
#
CFLAGS?=-I.

#
# Optimization and debugging flags go here.
#
CFLAGS+=-O3 -g

#
# We want lots of warnings.
#
CFLAGS+=-Wall -Wextra -Wno-unused-parameter -Wno-unused-function

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
# The size program for some stats
#
SIZE=size

#
# We currently use good old makedepend.
#
MAKEDEPEND=makedepend

#
# Source files
#
SRCS = \
	scheme_alloc.c \
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

#
# Object files
#
OBJS = $(patsubst %.c,%.o,$(SRCS))

# Build main executable
scheme: libscheme.a main.o
	$(CC) $(CFLAGS) $(LIBS) -o scheme main.o libscheme.a
	$(SIZE) scheme

# Build static library
libscheme.a: $(OBJS)
	$(AR) rv libscheme.a $(OBJS)
	$(RANLIB) libscheme.a
	$(SIZE) libscheme.a

# Run benchmark
bench: scheme
	time ./scheme bench.scm
.PHONY: bench

# Run tests
test: scheme
	./scheme run-tests.scm
	rm -f tmp1 tmp2 tmp3
.PHONY: test

# Generate index
cscope: $(SRCS)
	cscope -b $(CCINCLUDEFLAGS) $(^)
.PHONY: cscope

# Update dependencies
-include Makedepends
depend: $(SRCS)
	touch Makedepends
	$(MAKEDEPEND) -f Makedepends $(CCINCLUDEFLAGS) -- $(CFLAGS) -- $(^)
	rm Makedepends.bak
.PHONY: depend

# Clean build
clean:
	$(RM) -f \
		Makedepends $(OBJS) scheme main.o \
		libscheme.a libscheme.aux libscheme.dvi libscheme.log \
		tmp1 tmp2 tmp3
.PHONY: clean
