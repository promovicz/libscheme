/*
  libscheme
  Copyright (c) 1994 Brent Benson
  All rights reserved.

  Permission is hereby granted, without written agreement and without
  license or royalty fees, to use, copy, modify, and distribute this
  software and its documentation for any purpose, provided that the
  above copyright notice and the following two paragraphs appear in
  all copies of this software.
 
  IN NO EVENT SHALL BRENT BENSON BE LIABLE TO ANY PARTY FOR DIRECT,
  INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BRENT
  BENSON HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  BRENT BENSON SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT
  NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER
  IS ON AN "AS IS" BASIS, AND BRENT BENSON HAS NO OBLIGATION TO
  PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
  MODIFICATIONS.
*/

#include "scheme.h"
#include <stdio.h>

/* globals */
jmp_buf scheme_error_buf;

/* locals */
static Scheme_Object *error (int argc, Scheme_Object *argv[]);
static Scheme_Object *scheme_exit (int argc, Scheme_Object *argv[]);

void
scheme_init_error (Scheme_Env *env)
{
  scheme_add_global ("error", scheme_make_prim (error), env);
  scheme_add_global ("exit", scheme_make_prim (scheme_exit), env);
}

void
scheme_signal_error (char *msg, ...)
{
  va_list args;
  va_start (args, msg);
  /* fprintf (stderr, "error: "); */
  vfprintf (stderr, msg, args);
  fprintf (stderr, "\n");
  va_end (args);
  longjmp (scheme_error_buf, 1);
}

void
scheme_warning (char *msg, ...)
{
  va_list args;
  va_start (args, msg);
  vfprintf (stderr, msg, args);
  fprintf (stderr, "\n");
  va_end (args);
}

static Scheme_Object *
error (int argc, Scheme_Object *argv[])
{
  int i;

  SCHEME_ASSERT ((argc > 0), "error: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP (argv[0]), "error: first arg must be a string");
  fprintf (stderr, "error: %s:", SCHEME_STR_VAL (argv[0]));
  for ( i=1; i<argc ; ++i )
    {
      scheme_write (argv[i], scheme_stderr_port);
    }
  fprintf (stderr, "\n");
  longjmp (scheme_error_buf, 1);
}

void 
scheme_default_handler (void)
{
  if (setjmp (scheme_error_buf))
    {
      abort ();
    }
}

static Scheme_Object *
scheme_exit (int argc, Scheme_Object *argv[])
{
  int status;

  SCHEME_ASSERT ((argc == 0) || (argc == 1),
		 "exit: wrong number of arguments");
  if (argc == 1)
    {
      SCHEME_ASSERT (SCHEME_INTP (argv[0]),
		     "exit: arg must be an integer");
      status = SCHEME_INT_VAL (argv[0]);
    }
  else
    {
      status = 0;
    }
  exit (status);
}
