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

scheme_alloc.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_alloc.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_alloc.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_alloc.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_alloc.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_alloc.o: /usr/include/_ctype.h /usr/include/string.h
scheme_bool.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_bool.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_bool.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_bool.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_bool.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_bool.o: /usr/include/_ctype.h /usr/include/string.h
scheme_char.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_char.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_char.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_char.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_char.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_char.o: /usr/include/_ctype.h /usr/include/ctype.h
scheme_env.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_env.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_env.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_env.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_env.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_env.o: /usr/include/_ctype.h
scheme_error.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_error.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_error.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_error.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_error.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_error.o: /usr/include/_ctype.h
scheme_eval.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_eval.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_eval.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_eval.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_eval.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_eval.o: /usr/include/_ctype.h
scheme_fun.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_fun.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_fun.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_fun.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_fun.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_fun.o: /usr/include/_ctype.h
scheme_hash.o: scheme_hash.h /usr/include/string.h /usr/include/sys/_defs.h
scheme_hash.o: /usr/include/sys/_size_t.h
scheme_list.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_list.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_list.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_list.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_list.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_list.o: /usr/include/_ctype.h
scheme_number.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_number.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_number.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_number.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_number.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_number.o: /usr/include/_ctype.h scheme_nummacs.h /usr/include/math.h
scheme_number.o: /usr/include/string.h
scheme_port.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_port.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_port.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_port.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_port.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_port.o: /usr/include/_ctype.h
scheme_print.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_print.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_print.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_print.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_print.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_print.o: /usr/include/_ctype.h
scheme_promise.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_promise.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_promise.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_promise.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_promise.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_promise.o: /usr/include/_ctype.h
scheme_read.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_read.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_read.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_read.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_read.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_read.o: /usr/include/_ctype.h /usr/include/ctype.h
scheme_string.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_string.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_string.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_string.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_string.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_string.o: /usr/include/_ctype.h /usr/include/string.h
scheme_struct.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_struct.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_struct.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_struct.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_struct.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_struct.o: /usr/include/_ctype.h
scheme_symbol.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_symbol.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_symbol.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_symbol.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_symbol.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_symbol.o: /usr/include/_ctype.h /usr/include/string.h
scheme_syntax.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_syntax.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_syntax.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_syntax.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_syntax.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_syntax.o: /usr/include/_ctype.h
scheme_type.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_type.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_type.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_type.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_type.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_type.o: /usr/include/_ctype.h /usr/include/string.h
scheme_vector.o: scheme.h scheme_hash.h /usr/include/stdio.h
scheme_vector.o: /usr/include/sys/_defs.h /usr/include/_va_list.h
scheme_vector.o: /usr/include/sys/_size_t.h /usr/include/stdio_defs.h
scheme_vector.o: /usr/include/setjmp.h /usr/include/stdarg.h
scheme_vector.o: /usr/include/stdlib.h /usr/include/sys/_wchar_t.h
scheme_vector.o: /usr/include/_ctype.h
