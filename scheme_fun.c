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
#include <setjmp.h>

/* globals */
Scheme_Object *scheme_prim_type;
Scheme_Object *scheme_closure_type;
Scheme_Object *scheme_cont_type;

/* locals */
static Scheme_Object *scheme_collect_rest (int num_rest, Scheme_Object **rest);
static Scheme_Object *procedure_p (int argc, Scheme_Object *argv[]);
static Scheme_Object *apply (int argc, Scheme_Object *argv[]);
static Scheme_Object *map (int argc, Scheme_Object *argv[]);
static Scheme_Object *for_each (int argc, Scheme_Object *argv[]);
static Scheme_Object *call_cc (int argc, Scheme_Object *argv[]);

#define CONS(a,b) scheme_make_pair(a,b)

void
scheme_init_fun (Scheme_Env *env)
{
  scheme_prim_type = scheme_make_type ("<primitive>");
  scheme_closure_type = scheme_make_type ("<closure>");
  scheme_cont_type = scheme_make_type ("<continuation>");
  scheme_add_global ("<primitive>", scheme_prim_type, env);
  scheme_add_global ("<closure>", scheme_closure_type, env);
  scheme_add_global ("<continuation>", scheme_cont_type, env);
  scheme_add_global ("procedure?", scheme_make_prim (procedure_p), env);
  scheme_add_global ("apply", scheme_make_prim (apply), env);
  scheme_add_global ("map", scheme_make_prim (map), env);
  scheme_add_global ("for-each", scheme_make_prim (for_each), env);
  scheme_add_global ("call-with-current-continuation", scheme_make_prim (call_cc), env);
  scheme_add_global ("call/cc", scheme_make_prim (call_cc), env);
}

Scheme_Object *
scheme_make_prim (Scheme_Prim *fun)
{
  Scheme_Object *prim;

  prim = scheme_alloc_object ();
  SCHEME_TYPE (prim) = scheme_prim_type;
  SCHEME_PRIM (prim) = fun;
  return (prim);
}

Scheme_Object *
scheme_make_closure (Scheme_Env *env, Scheme_Object *code)
{
  Scheme_Object *closure;

  closure = scheme_alloc_object ();
  SCHEME_TYPE (closure) = scheme_closure_type;
  SCHEME_CLOS_ENV (closure) = env;
  SCHEME_CLOS_CODE (closure) = code;
  return (closure);
}

Scheme_Object *
scheme_make_cont (jmp_buf buf)
{
  Scheme_Object *cont;

  cont = scheme_alloc_object ();
  SCHEME_TYPE (cont) = scheme_cont_type;
  SCHEME_PTR_VAL (cont) = buf;
  return (cont);
}

Scheme_Object *
scheme_apply (Scheme_Object *rator, int num_rands, Scheme_Object **rands)
{
  Scheme_Object *fun_type;

  fun_type = SCHEME_TYPE (rator);
  if (fun_type == scheme_closure_type)
    {
      Scheme_Env *env, *frame;
      Scheme_Object *params, *forms, *ret, *t1, *t2;
      Scheme_Object *vars, *vals, *vars_last, *vals_last, *pair, *aform;
      int num_int_defs, num_params, i, has_rest;

      env = SCHEME_CLOS_ENV (rator);
      params = SCHEME_CAR (SCHEME_CLOS_CODE (rator));
      num_params = scheme_list_length (params);
      frame = scheme_new_frame (num_params);
      has_rest = 0;
      for ( i=0 ; i<num_params ; ++i )
	{
	  if (! SCHEME_PAIRP (params))
	    {
	      Scheme_Object *rest_vals;

	      rest_vals = scheme_collect_rest ((num_rands - i), (rands + i));
	      scheme_add_binding (i, params, rest_vals, frame);
	      has_rest = 1;
	    }
	  else
	    {
	      scheme_add_binding (i, SCHEME_CAR (params), rands[i], frame);
	      params = SCHEME_CDR (params);
	    }
	}
      if ( has_rest )
	{
	  if (num_rands < (num_params - 1))
	    {
	      scheme_signal_error ("too few arguments to procedure");
	    }
	}
      else
	{
	  if (num_rands < num_params)
	    {
	      scheme_signal_error ("too few arguments to procedure");
	    }
	  if (num_rands > num_params)
	    {
	      scheme_signal_error ("too many arguments to procedure");
	    }
	}
      env = scheme_extend_env (frame, env);
      forms = SCHEME_CDR (SCHEME_CLOS_CODE (rator));
      SCHEME_ASSERT (forms != scheme_null, "apply: no forms in closure body");

      /* process internal defines */
      num_int_defs = 0;
      vars = vals = vars_last = vals_last = scheme_null;
      while (!SCHEME_NULLP(forms) && 
	     SCHEME_PAIRP (SCHEME_CAR(forms)) &&
	     SCHEME_CAR (SCHEME_CAR (forms)) == scheme_intern_symbol ("define"))
	{
	  num_int_defs++;
	  aform = SCHEME_CAR (forms);
	  /* get var */
	  if (SCHEME_PAIRP (SCHEME_CAR (SCHEME_CDR (aform))))
	    pair = scheme_make_pair (SCHEME_CAR (SCHEME_CAR (SCHEME_CDR (aform))), scheme_null);
	  else
	    pair = scheme_make_pair (SCHEME_CAR (SCHEME_CDR (aform)), scheme_null);
	  
	  if (SCHEME_NULLP (vars))
	    vars = vars_last = pair;
	  else
	    {
	      SCHEME_CDR (vars_last) = pair;
	      vars_last = pair;
	    }
	  /* get val */
	  if (SCHEME_PAIRP (SCHEME_CAR (SCHEME_CDR (aform))))
	    pair = CONS (CONS (scheme_intern_symbol("lambda"),
			       CONS (SCHEME_CDR (SCHEME_CAR (SCHEME_CDR (aform))),
				     SCHEME_CDR (SCHEME_CDR (aform)))),
			 scheme_null);
	  else
	    pair = scheme_make_pair (SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (aform))), scheme_null);

	  if (SCHEME_NULLP (vals))
	    vals = vals_last = pair;
	  else
	    {
	      SCHEME_CDR (vals_last) = pair;
	      vals_last = pair;
	    }
	  forms = SCHEME_CDR (forms);
	}
      if ( num_int_defs )
	{
	  env = scheme_add_frame (vars, scheme_alloc_list (num_int_defs), env);
	  while (! SCHEME_NULLP (vars))
	    {
	      scheme_set_value (SCHEME_CAR(vars),
				scheme_eval (SCHEME_CAR (vals), env),
				env);
	      vars = SCHEME_CDR (vars);
	      vals = SCHEME_CDR (vals);
	    }
	}

      while (forms != scheme_null)
	{
	  ret = scheme_eval (SCHEME_CAR (forms), env);
	  forms = SCHEME_CDR (forms);
	}
      /* pop internal define frame */
      if ( num_int_defs )
	{
	  env = scheme_pop_frame (env);
	}
      /* pop regular binding frame */
      env = scheme_pop_frame (env);
      return (ret);
    }
  else if (fun_type == scheme_prim_type)
    {
      return (SCHEME_PRIM(rator)(num_rands, rands));
    }
  else if (fun_type == scheme_cont_type)
    {
      SCHEME_ASSERT ((num_rands == 1),
		     "apply: wrong number of args to continuation procedure");
      longjmp ((int *)SCHEME_PTR_VAL(rator), (int)rands[0]);
    }
  else if (fun_type == scheme_struct_proc_type)
    {
      return (scheme_apply_struct_proc (rator, 
					scheme_collect_rest (num_rands, rands)));
    }
  else
    {
      scheme_signal_error ("apply: bad procedure");
    }
}

Scheme_Object *
scheme_apply_to_list (Scheme_Object *rator, Scheme_Object *rands)
{
  int num_rands, i;
  Scheme_Object *rands_vec[SCHEME_MAX_ARGS];

  num_rands = scheme_list_length (rands);
  for ( i=0 ; i<num_rands ; ++i )
    {
      rands_vec[i] = SCHEME_CAR (rands);
      rands = SCHEME_CDR (rands);
    }
  return (scheme_apply (rator, num_rands, rands_vec));
}

/* locals */

static Scheme_Object *
scheme_collect_rest (int num_rest, Scheme_Object **rest)
{
  Scheme_Object *rest_list, *last, *pair;
  int i;

  rest_list = last = scheme_null;
  for ( i=0 ; i<num_rest ; ++i )
    {
      pair = scheme_make_pair (rest[i], scheme_null);
      if (SCHEME_NULLP (rest_list))
	{
	  rest_list = last = pair;
	}
      else
	{
	  SCHEME_CDR (last) = pair;
	  last = pair;
	}
    }
  return (rest_list);
}

static Scheme_Object *
procedure_p (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "procedure?: wrong number of args");
  return (SCHEME_PROCP (argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Object *
apply (int argc, Scheme_Object *argv[])
{
  Scheme_Object *rands, *rands_last, *pair;
  Scheme_Object *rand_vec[SCHEME_MAX_ARGS];
  int i, num_rands;

  SCHEME_ASSERT ((argc >= 2), "apply: two argument version only");
  SCHEME_ASSERT (SCHEME_PROCP (argv[0]), "apply: first arg must be a procedure");
  rands = rands_last = scheme_null;
  for ( i=1 ; i<(argc-1) ; ++i )
    {
      pair = scheme_make_pair (argv[i], scheme_null);
      if (SCHEME_NULLP (rands))
	{
	  rands = rands_last = pair;
	}
      else
	{
	  SCHEME_CDR (rands_last) = pair;
	  rands_last = pair;
	}
    }
  SCHEME_ASSERT (SCHEME_LISTP (argv[i]), "apply: last arg must be a list");
  if (SCHEME_NULLP (rands))
    {
      rands = argv[i];
    }
  else
    {
      SCHEME_CDR (rands_last) = argv[i];
    }
  num_rands = scheme_list_length (rands);
  for ( i=0 ; i<num_rands ; ++i )
    {
      rand_vec[i] = SCHEME_CAR (rands);
      rands = SCHEME_CDR (rands);
    }
  return (scheme_apply (argv[0], num_rands, rand_vec));
}

static Scheme_Object *map_help (Scheme_Object *fun, Scheme_Object *list);

static Scheme_Object *
map (int argc, Scheme_Object *argv[])
{
  int i, size;
  Scheme_Object *first, *last, *pair;
  Scheme_Object *retfirst, *retlast;

  SCHEME_ASSERT ((argc > 1), "map: wrong number of args");
  SCHEME_ASSERT (SCHEME_PROCP (argv[0]), "map: first arg must be a procedure");
  for ( i=1 ; i<argc ; ++i )
    {
      SCHEME_ASSERT (SCHEME_LISTP (argv[i]), "map: all args other than first must be lists");
      if (i == 1)
	{
	  size = scheme_list_length (argv[i]);
	}
      else
	{
	  if (size != scheme_list_length (argv[i]))
	    {
	      scheme_signal_error ("map: all lists must have same size");
	    }
	}
    }

  retfirst = retlast = scheme_null;
  while (! SCHEME_NULLP (argv[1]))
    {
      /* collect args to apply */
      first = last = scheme_null;
      for ( i=1 ; i<argc ; ++i )
	{
	  pair = scheme_make_pair (SCHEME_CAR (argv[i]), scheme_null);
	  if (SCHEME_NULLP (first))
	    {
	      first = last = pair;
	    }
	  else
	    {
	      SCHEME_CDR (last) = pair;
	      last = pair;
	    }
	  argv[i] = SCHEME_CDR (argv[i]);
	}
      pair = scheme_make_pair (scheme_apply_to_list (argv[0], first), scheme_null);
      if (SCHEME_NULLP (retfirst))
	{
	  retfirst = retlast = pair;
	}
      else
	{
	  SCHEME_CDR (retlast) = pair;
	  retlast = pair;
	}
    }
  return (retfirst);
}

static Scheme_Object *
map_help (Scheme_Object *fun, Scheme_Object *list)
{
  if (SCHEME_NULLP (list))
    {
      return (scheme_null);
    }
  else
    {
      return (scheme_make_pair 
	      (scheme_apply_to_list(fun,scheme_make_pair(SCHEME_CAR(list), scheme_null)),
	       map_help (fun, SCHEME_CDR (list))));
    }
}

static Scheme_Object *
for_each (int argc, Scheme_Object *argv[])
{
  Scheme_Object *ret, *list, *fun;

  SCHEME_ASSERT ((argc == 2), "for-each: two argument version only");
  SCHEME_ASSERT (SCHEME_PROCP (argv[0]), "for-each: first arg must be a procedure");
  SCHEME_ASSERT (SCHEME_LISTP (argv[1]), "for-each: second arg must be a list");
  fun = argv[0];
  list = argv[1];
  while (! SCHEME_NULLP (list))
    {
      ret = scheme_apply_to_list (fun, scheme_make_pair (SCHEME_CAR (list), scheme_null));
      list = SCHEME_CDR (list);
    }
  return (ret);
}

static Scheme_Object *
call_cc (int argc, Scheme_Object *argv[])
{
  jmp_buf buf;
  Scheme_Object *ret, *cont;

  SCHEME_ASSERT ((argc == 1), "call-with-current-continuation: wrong number of args");
  SCHEME_ASSERT (SCHEME_PROCP (argv[0]), 
		 "call-with-current-continuation: arg must be a procedure");
  if (ret = (Scheme_Object *)setjmp (buf))
    {
      return (ret);
    }
  else
    {
      cont = scheme_make_cont (buf);
      return (scheme_apply_to_list (argv[0], scheme_make_pair (cont, scheme_null)));
    }
}
