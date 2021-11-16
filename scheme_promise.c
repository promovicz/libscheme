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
  Scheme_Value val;
  Scheme_Env *env;
};
typedef struct Scheme_Promise Scheme_Promise;

/* globals */
Scheme_Value scheme_promise_type;

/* locals */
static Scheme_Value force (int argc, Scheme_Value argv[]);

void
scheme_init_promise (Scheme_Env *env)
{
  scheme_promise_type = scheme_make_type ("<promise>");
  scheme_add_global ("<promise>", scheme_promise_type, env);
  scheme_add_prim ("force", force, env);
}

Scheme_Value
scheme_make_promise (Scheme_Value expr, Scheme_Env *env)
{
  Scheme_Value obj;
  Scheme_Promise *promise;

  obj = scheme_alloc_object (scheme_promise_type, sizeof(Scheme_Promise));
  promise = (Scheme_Promise *) SCHEME_PTR_VAL(obj);
  promise->forced = 0;
  promise->val = expr;
  promise->env = env;
  return (obj);
}

static Scheme_Value
force (int argc, Scheme_Value argv[])
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
