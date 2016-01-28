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

/* locals */
static Scheme_Object *scheme_eval_combination (Scheme_Object *comb, Scheme_Env *env);
static Scheme_Object *eval (int argc, Scheme_Object *argv[]);

void
scheme_init_eval (Scheme_Env *env)
{
  scheme_add_global ("eval", scheme_make_prim (eval), env);
}

Scheme_Object *
scheme_eval (Scheme_Object *obj, Scheme_Env *env)
{
  Scheme_Object *type;

  type = SCHEME_TYPE (obj);
  if (type == scheme_symbol_type)
    {
      Scheme_Object *val;

      val = scheme_lookup_value (obj, env);
      if (! val)
	{
	  scheme_signal_error ("reference to unbound symbol: %s", SCHEME_STR_VAL(obj));
	}
      return (val);
    }
  else if (type == scheme_pair_type)
    {
      return (scheme_eval_combination (obj, env));
    }
  else
    {
      return (obj);
    }
}

/* local functions */

static Scheme_Object *
scheme_eval_combination (Scheme_Object *comb, Scheme_Env *env)
{
  Scheme_Object *rator, *type, *rands;
  Scheme_Object *evaled_rands[SCHEME_MAX_ARGS];
  Scheme_Object *rand, *fun, *form;
  int num_rands, i;

  rator = scheme_eval (SCHEME_CAR (comb), env);
  type = SCHEME_TYPE (rator);
  if (type == scheme_syntax_type)
    {
      return (SCHEME_SYNTAX(rator)(comb, env));
    }
  else if (type == scheme_macro_type)
    {
      fun = (Scheme_Object *) SCHEME_PTR_VAL (rator);
      rands = SCHEME_CDR (comb);
      form = scheme_apply_to_list (fun, rands);
      return (scheme_eval (form, env));
    }
  else
    {
      rands = SCHEME_CDR (comb);
      num_rands = scheme_list_length (rands);
      i = 0;
      while (rands != scheme_null)
	{
	  evaled_rands[i] = scheme_eval (SCHEME_CAR (rands), env);
	  i++;
	  rands = SCHEME_CDR (rands);
	}
      return (scheme_apply (rator, num_rands, evaled_rands));
    }
}

static Scheme_Object *
eval (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "eval: wrong number of args");
  return (scheme_eval (argv[0], scheme_env));
}
