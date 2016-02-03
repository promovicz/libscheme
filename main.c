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

void load_file(Scheme_Env *env, const char *path) {
  Scheme_Object *in_port, *obj;
  FILE *fp;

  fp = fopen (path, "r");
  if (! fp)
    {
      fprintf (stderr, "could not open file for loading: %s\n", path);
    }
  else
    {
      /* skip `#!' line if present */
      fscanf (fp, "#!%*s\n");
      /* read each expression and evaluate it */
      in_port = scheme_make_file_input_port (fp);
      while ((obj = scheme_read (in_port)) != scheme_eof)
        {
          obj = SCHEME_CATCH_ERROR (scheme_eval (obj, env), 0);
        }
      scheme_close_input_port (in_port);
    }
}

void read_eval_print(Scheme_Env *env) {
  Scheme_Object *obj;
  do
    {
      printf ("> ");
      obj = scheme_read (scheme_stdin_port);
      if (obj == scheme_eof)
        {
          printf ("\n; done\n");
          exit (0);
        }
      obj = SCHEME_CATCH_ERROR(scheme_eval (obj, env), 0);
      if (obj)
        {
          scheme_write (obj, scheme_stdout_port);
          printf ("\n");
        }
    }
  while ( 1 );
}

int
main(int argc, char *argv[])
{
  Scheme_Env *env;
  int i;

  env = scheme_basic_env ();

  /* load any files given on the command line */
  for ( i=1 ; i<argc ; ++i )
    {
      load_file(env, argv[i]);
    }

  /* enter read-eval-print loop */
  read_eval_print(env);

  return 0;
}
