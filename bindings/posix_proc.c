/*
  posix_proc.c
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

   (posix-fork) => (or <integer> #f)
*/

#include "posix.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* static function declarations */
static Scheme_Value posix_fork (int argc, Scheme_Value argv[]);
static Scheme_Value posix_exit (int argc, Scheme_Value argv[]);
static Scheme_Value posix_wait (int argc, Scheme_Value argv[]);
static Scheme_Value posix_waitpid (int argc, Scheme_Value argv[]);
static Scheme_Value posix_execl (int argc, Scheme_Value argv[]);
static Scheme_Value posix_execv (int argc, Scheme_Value argv[]);

/* exported functions */

void
scheme_init_posix_proc (Scheme_Env *env)
{
  scheme_add_prim ("posix-fork", posix_fork, env);
  scheme_add_prim ("posix-exit", posix_exit, env);
  scheme_add_prim ("posix-wait", posix_wait, env);
  scheme_add_prim ("posix-waitpid", posix_waitpid, env);
  scheme_add_prim ("posix-execl", posix_execl, env);
  scheme_add_prim ("posix-execv", posix_execv, env);
}

/* static functions */

static Scheme_Value
posix_fork (int argc, Scheme_Value argv[])
{
  int ret;

  SCHEME_ASSERT ((argc == 0), "posix-fork: wrong number of args");
  ret = fork ();
  SCHEME_ASSERT ((ret != -1), "posix-fork: could not fork");
  if (ret == 0)
    {
      return (scheme_false);
    }
  else
    {
      return (scheme_make_integer (ret));
    }
}

static Scheme_Value
posix_exit (int argc, Scheme_Value argv[])
{
  int status;

  SCHEME_ASSERT ((argc == 1), "posix-exit: wrong number of args");
  SCHEME_ASSERT (SCHEME_INTP (argv[0]), "posix-exit: arg must be an integer");
  status = SCHEME_INT_VAL (argv[0]);
  exit (status);
}

static Scheme_Value
posix_wait (int argc, Scheme_Value argv[])
{
  int ret;

  SCHEME_ASSERT ((argc == 0), "posix-wait: wrong number of args");
  ret = wait (NULL);
  SCHEME_ASSERT ((ret != -1), "posix-wait: could not wait");
  return (scheme_make_integer (ret));
}

static Scheme_Value
posix_waitpid (int argc, Scheme_Value argv[])
{
  int ret;
  int pid;

  SCHEME_ASSERT ((argc == 1), "posix-waitpid: wrong number of args");
  SCHEME_ASSERT (SCHEME_INTP (argv[0]), "posix-waitpid: arg must be an integer");
  pid = SCHEME_INT_VAL (argv[0]);
  ret = waitpid (pid, NULL, 0);
  SCHEME_ASSERT ((ret != -1), "posix-waitpid: could not wait");
  return (scheme_make_integer (ret));
}

static Scheme_Value
posix_execl (int argc, Scheme_Value argv[])
{
  char *path;
  char **exec_argv;
  int i;

  SCHEME_ASSERT ((argc >= 1), "posix-execl: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP (argv[0]), "posix-execl: first arg must be a string");
  path = SCHEME_STR_VAL (argv[0]);
  exec_argv = (char **) scheme_malloc (sizeof (char *) * (argc + 1));
  for ( i=0; i<argc ; ++i )
    {
      SCHEME_ASSERT (SCHEME_STRINGP (argv[i]), "posix-execl: all arguments must be strings");
      exec_argv[i] = SCHEME_STR_VAL (argv[i]);
    }
  exec_argv[argc] = NULL;
  execv (path, exec_argv);
  /* execv only returns on error */
  scheme_signal_error ("posix-execl: could not exec `%s'", path);
}

static Scheme_Value
posix_execv (int argc, Scheme_Value argv[])
{
  char *path;
  char **exec_argv;
  int i, num_extra_args;
  Scheme_Value arg_list, arg;

  SCHEME_ASSERT ((argc == 2), "posix-execv: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP (argv[0]), "posix-execv: first arg must be a string");
  SCHEME_ASSERT (SCHEME_LISTP (argv[1]), "posix-execv: second arg must be a list");
  path = SCHEME_STR_VAL (argv[0]);
  arg_list = argv[1];
  num_extra_args = scheme_list_length (arg_list);
  exec_argv = (char **) scheme_malloc (sizeof (char *) * (num_extra_args + 2));
  exec_argv[0] = path;
  arg_list = argv[1];
  for ( i=1; i<(num_extra_args+1) ; ++i )
    {
      arg = SCHEME_CAR (arg_list);
      SCHEME_ASSERT (SCHEME_STRINGP (arg), "posix-execv: all elements of arg list must be strings");
      exec_argv[i] = SCHEME_STR_VAL (arg);
      arg_list = SCHEME_CDR (arg_list);
    }
  exec_argv[num_extra_args+1] = NULL;
  execv (path, exec_argv);
  /* execv only returns on error */
  scheme_signal_error ("posix-execv: could not exec `%s'", path);
}
