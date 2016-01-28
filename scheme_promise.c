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

struct Scheme_Promise
{
  int forced;
  Scheme_Object *val;
  Scheme_Env *env;
};
typedef struct Scheme_Promise Scheme_Promise;

/* globals */
Scheme_Object *scheme_promise_type;

/* locals */
static Scheme_Object *force (int argc, Scheme_Object *argv[]);

void
scheme_init_promise (Scheme_Env *env)
{
  scheme_promise_type = scheme_make_type ("<promise>");
  scheme_add_global ("<promise>", scheme_promise_type, env);
  scheme_add_global ("force", scheme_make_prim (force), env);
}

Scheme_Object *
scheme_make_promise (Scheme_Object *expr, Scheme_Env *env)
{
  Scheme_Object *obj;
  Scheme_Promise *promise;

  promise = (Scheme_Promise *) scheme_malloc (sizeof (Scheme_Promise));
  promise->forced = 0;
  promise->val = expr;
  promise->env = env;
  obj = scheme_alloc_object ();
  SCHEME_TYPE (obj) = scheme_promise_type;
  SCHEME_PTR_VAL (obj) = promise;
  return (obj);
}

static Scheme_Object *
force (int argc, Scheme_Object *argv[])
{
  Scheme_Promise *promise;

  SCHEME_ASSERT ((argc == 1), "force: wrong number of args");
  SCHEME_ASSERT (SCHEME_PROMP(argv[0]), "force: arg must be a promise");
  promise = (Scheme_Promise *) SCHEME_PTR_VAL (argv[0]);
  if (promise->forced)
    {
      return (promise->val);
    }
  else
    {
      promise->val = scheme_eval (promise->val, promise->env);
      return (promise->val);
    }
}
