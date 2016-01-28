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

/* globals */
Scheme_Object *scheme_syntax_type;
Scheme_Object *scheme_macro_type;

/* locals */
static Scheme_Object *lambda_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *define_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *quote_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *if_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *set_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *cond_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *case_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *and_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *or_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *let_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *let_star_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *letrec_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *begin_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *do_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *delay_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *quasiquote_syntax (Scheme_Object *form, Scheme_Env *env);
/* non-standard */
static Scheme_Object *defmacro_syntax (Scheme_Object *form, Scheme_Env *env);

/* symbols */
static Scheme_Object *scheme_quasiquote;
static Scheme_Object *scheme_unquote;
static Scheme_Object *scheme_unquote_splicing;
static Scheme_Object *scheme_define;
static Scheme_Object *scheme_lambda;

#define CONS(a,b) scheme_make_pair(a,b)

void 
scheme_init_syntax (Scheme_Env *env)
{
  scheme_syntax_type = scheme_make_type ("<syntax>");
  scheme_add_global ("<syntax>", scheme_syntax_type, env);
  scheme_macro_type = scheme_make_type ("<macro>");
  scheme_add_global ("<macro>", scheme_macro_type, env);
  scheme_quasiquote = scheme_intern_symbol ("quasiquote");
  scheme_unquote = scheme_intern_symbol ("unquote");
  scheme_unquote_splicing = scheme_intern_symbol ("unquote-splicing");
  scheme_define = scheme_intern_symbol ("define");
  scheme_lambda = scheme_intern_symbol ("lambda");
  scheme_add_global ("lambda", scheme_make_syntax (lambda_syntax), env);
  scheme_add_global ("define", scheme_make_syntax (define_syntax), env);
  scheme_add_global ("quote", scheme_make_syntax (quote_syntax), env);
  scheme_add_global ("if", scheme_make_syntax (if_syntax), env);
  scheme_add_global ("set!", scheme_make_syntax (set_syntax), env);
  scheme_add_global ("cond", scheme_make_syntax (cond_syntax), env);
  scheme_add_global ("case", scheme_make_syntax (case_syntax), env);
  scheme_add_global ("and", scheme_make_syntax (and_syntax), env);
  scheme_add_global ("or", scheme_make_syntax (or_syntax), env);
  scheme_add_global ("let", scheme_make_syntax (let_syntax), env);
  scheme_add_global ("let*", scheme_make_syntax (let_star_syntax), env);
  scheme_add_global ("letrec", scheme_make_syntax (letrec_syntax), env);
  scheme_add_global ("begin", scheme_make_syntax (begin_syntax), env);
  scheme_add_global ("do", scheme_make_syntax (do_syntax), env);
  scheme_add_global ("delay", scheme_make_syntax (delay_syntax), env);
  scheme_add_global ("quasiquote", scheme_make_syntax (quasiquote_syntax), env);
  scheme_add_global ("defmacro", scheme_make_syntax (defmacro_syntax), env);
}

Scheme_Object *
scheme_make_syntax (Scheme_Syntax *proc)
{
  Scheme_Object *syntax;

  syntax = scheme_alloc_object ();
  SCHEME_TYPE (syntax) = scheme_syntax_type;
  SCHEME_SYNTAX (syntax) = proc;
  return (syntax);
}

/* builtin syntax */

static Scheme_Object *
lambda_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *params;

  SCHEME_ASSERT (SCHEME_PAIRP(form), "badly formed lambda");
  SCHEME_ASSERT (SCHEME_PAIRP(SCHEME_CDR(form)), "badly formed lambda");
  return (scheme_make_closure (env, SCHEME_CDR(form)));
}

static Scheme_Object *
define_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *var, *val, *sec, *params, *forms;

  sec = SCHEME_CAR (SCHEME_CDR (form));

  if (SCHEME_TYPE(sec) == scheme_pair_type)
    {
      var = SCHEME_CAR (sec);
      params = SCHEME_CDR (sec);
      forms = SCHEME_CDR (SCHEME_CDR (form));
      val = scheme_make_closure (env, scheme_make_pair (params, forms));
    }
  else if (SCHEME_TYPE(sec) == scheme_symbol_type)
    {
      var = sec;
      val = SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (form)));
      val = scheme_eval (val, env);
    }
  else
    {
      scheme_signal_error ("define: second arg must be symbol or list");
    }
  scheme_add_global (SCHEME_STR_VAL(var), val, env);
  return (var);
}

static Scheme_Object *
quote_syntax (Scheme_Object *form, Scheme_Env *env)
{
  SCHEME_ASSERT ((scheme_list_length (form) == 2), "quote: wrong number of args");
  return (SCHEME_CAR (SCHEME_CDR (form)));
}

static Scheme_Object *
if_syntax (Scheme_Object *form, Scheme_Env *env)
{
  int len;
  Scheme_Object *test, *thenp, *elsep;

  len = scheme_list_length (form);
  SCHEME_ASSERT (((len == 3) || (len == 4)), "badly formed if statement");
  test = SCHEME_CAR (SCHEME_CDR (form));
  test = scheme_eval (test, env);
  if (test != scheme_false)
    {
      thenp = SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (form)));
      return (scheme_eval (thenp, env));
    }
  else
    {
      if (len == 4)
	{
	  elsep = SCHEME_CAR (SCHEME_CDR (SCHEME_CDR ((SCHEME_CDR (form)))));
	  return (scheme_eval (elsep, env));
	}
      else
	{
	  return (scheme_false);
	}
    }
}

static Scheme_Object *
set_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *var, *val;

  SCHEME_ASSERT ((scheme_list_length (form) == 3), "bad set! form");
  var = SCHEME_CAR (SCHEME_CDR (form));
  SCHEME_ASSERT (SCHEME_TYPE (var) == scheme_symbol_type,
		 "second arg to `set!' must be symbol");
  val = scheme_eval (SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (form))), env);
  scheme_set_value (var, val, env);
  return (val);
}

static Scheme_Object *
cond_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *clauses, *clause, *test, *forms, *ret;

  clauses = SCHEME_CDR (form);
  ret = scheme_false;
  while (clauses != scheme_null)
    {
      clause = SCHEME_CAR (clauses);
      test = SCHEME_CAR (clause);
      if (test == scheme_intern_symbol ("else"))
	{
	  test = scheme_true;
	}
      else
	{
	  test = scheme_eval (test, env);
	}
      if (test != scheme_false)
	{
	  forms = SCHEME_CDR (clause);
	  if (!SCHEME_NULLP (forms) && 
	      (SCHEME_CAR(forms) == scheme_intern_symbol ("=>")))
	    {
	      Scheme_Object *proc;

	      forms = SCHEME_CDR (forms);
	      SCHEME_ASSERT (!SCHEME_NULLP(forms), "cond: bad `=>' clause");
	      proc = scheme_eval (SCHEME_CAR (forms), env);
	      SCHEME_ASSERT (SCHEME_PROCP(proc), "cond: form after `=>' must evaluate to a procedure");
	      ret = scheme_apply_to_list (proc, scheme_make_pair (test, scheme_null));
	    }
	  else
	    {
	      while (forms != scheme_null)
		{
		  ret = scheme_eval (SCHEME_CAR (forms), env);
		  forms = SCHEME_CDR (forms);
		}
	    }
	  return (ret);
	}
      clauses = SCHEME_CDR (clauses);
    }
  return (ret);
}

static Scheme_Object *
case_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *key, *clauses, *clause;
  Scheme_Object *data, *exprs, *res;

  key = SCHEME_CAR (SCHEME_CDR (form));
  clauses = SCHEME_CDR (SCHEME_CDR (form));

  key = scheme_eval (key, env);
  while (! SCHEME_NULLP (clauses))
    {
      clause = SCHEME_CAR (clauses);
      data = SCHEME_CAR (clause);
      if (SCHEME_SYMBOLP (data) && (scheme_intern_symbol("else") == data))
	{
	  exprs = SCHEME_CDR (clause);
	  while (! SCHEME_NULLP (exprs))
	    {
	      res = scheme_eval (SCHEME_CAR (exprs), env);
	      exprs = SCHEME_CDR (exprs);
	    }
	  return (res);
	}
      SCHEME_ASSERT (SCHEME_PAIRP(data), "case: first thing in clause must be a list");
      while (! SCHEME_NULLP (data))
	{
	  if (scheme_eqv (SCHEME_CAR (data), key))
	    {
	      exprs = SCHEME_CDR (clause);
	      while (! SCHEME_NULLP (exprs))
		{
		  res = scheme_eval (SCHEME_CAR (exprs), env);
		  exprs = SCHEME_CDR (exprs);
		}
	      return (res);
	    }
	  data = SCHEME_CDR (data);
	}
      clauses = SCHEME_CDR (clauses);
    }
  return (scheme_false);
}

static Scheme_Object *
and_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *forms, *ret;

  forms = SCHEME_CDR (form);
  ret = scheme_true;
  while (forms != scheme_null)
    {
      ret = scheme_eval (SCHEME_CAR (forms), env);
      if (ret == scheme_false)
	{
	  return (scheme_false);
	}
      forms = SCHEME_CDR (forms);
    }
  return (ret);
}

static Scheme_Object *
or_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *forms, *ret;

  forms = SCHEME_CDR (form);
  ret = scheme_false;
  while (forms != scheme_null)
    {
      ret = scheme_eval (SCHEME_CAR (forms), env);
      if (ret != scheme_false)
	{
	  return (ret);
	}
      forms = SCHEME_CDR (forms);
    }
  return (ret);
}

static int internal_def_p (Scheme_Object *form);
static Scheme_Object *named_let_syntax (Scheme_Object *form, Scheme_Env *env);

static Scheme_Object *
let_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *bindings, *binding, *vars, *vals, *ev_vals, *forms, *ret;
  Scheme_Object *vars_last, *vals_last, *pair, *aform;
  int num_int_defs, num_bindings, i;
  Scheme_Env *frame;

  if (SCHEME_SYMBOLP (SCHEME_CAR (SCHEME_CDR (form))))
    {
      return (named_let_syntax (form, env));
    }
  SCHEME_ASSERT ((scheme_list_length(form) >= 3), "badly formed `let' form");
  bindings = SCHEME_CAR (SCHEME_CDR (form));
  num_bindings = scheme_list_length (bindings);
  frame = scheme_new_frame (num_bindings);
  for ( i=0 ; i<num_bindings; ++i )
    {
      binding = SCHEME_CAR (bindings);
      scheme_add_binding (i, SCHEME_CAR (binding), scheme_eval (SCHEME_CADR (binding), env), frame);
      bindings = SCHEME_CDR (bindings);
    }
  env = scheme_extend_env (frame, env);
  forms = SCHEME_CDR (SCHEME_CDR (form));

  /* process internal defines */
  num_int_defs = 0;
  vars = vals = vars_last = vals_last = scheme_null;
  while (!SCHEME_NULLP(forms) && internal_def_p (SCHEME_CAR(forms)))
    {
      num_int_defs++;
      aform = SCHEME_CAR (forms);
      /* get var */
      if (SCHEME_PAIRP (SCHEME_CAR (SCHEME_CDR (aform))))
	{
	  pair = scheme_make_pair (SCHEME_CAR (SCHEME_CAR (SCHEME_CDR (aform))), scheme_null);
	}
      else
	{
	  pair = scheme_make_pair (SCHEME_CAR (SCHEME_CDR (aform)), scheme_null);
	}
      if (SCHEME_NULLP (vars))
	{
	  vars = vars_last = pair;
	}
      else
	{
	  SCHEME_CDR (vars_last) = pair;
	  vars_last = pair;
	}
      /* get val */
      if (SCHEME_PAIRP (SCHEME_CAR (SCHEME_CDR (aform))))
	{
	  pair = CONS (CONS (scheme_lambda, CONS (SCHEME_CDR (SCHEME_CAR (SCHEME_CDR (aform))),
						  SCHEME_CDR (SCHEME_CDR (aform)))),
		       scheme_null);
	}
      else
	{
	  pair = scheme_make_pair (SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (aform))), scheme_null);
	}
      if (SCHEME_NULLP (vals))
	{
	  vals = vals_last = pair;
	}
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

static int 
internal_def_p (Scheme_Object *form)
{
  return (SCHEME_PAIRP(form) && (SCHEME_CAR(form) == scheme_define));
}

static Scheme_Object *
named_let_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *name, *bindings, *vars, *vals, *forms;
  Scheme_Object *proc, *init, *ret;

  name = SCHEME_CAR (SCHEME_CDR (form));
  bindings = SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (form)));
  vars = scheme_map_1 (scheme_car, bindings);
  vals = scheme_map_1 (scheme_cadr, bindings);
  forms = SCHEME_CDR (SCHEME_CDR (SCHEME_CDR (form)));

  proc = CONS(scheme_intern_symbol ("lambda"), CONS(vars, forms));
  init = CONS(name, vals);

  env = scheme_add_frame (CONS(name, scheme_null), CONS(scheme_null, scheme_null), env);
  proc = scheme_eval (proc, env);
  scheme_set_value (name, proc, env);
  ret = scheme_eval (init, env);
  env = scheme_pop_frame (env);
  return (ret);
}

static Scheme_Object *
let_star_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *bindings, *vars, *vals, *ev_vals, *forms, *ret;
  Scheme_Object *vars_last, *vals_last, *pair, *aform;
  Scheme_Env *frame;
  int num_int_defs, num_bindings, i;

  SCHEME_ASSERT ((scheme_list_length(form) >= 3), "badly formed `let*' form");
  bindings = SCHEME_CAR (SCHEME_CDR (form));
  num_bindings = scheme_list_length (bindings);
  vars = scheme_map_1 (scheme_car, bindings);
  vals = scheme_map_1 (scheme_cadr, bindings);
  frame = scheme_new_frame (num_bindings);

  /* first install dummy bindings */
  for ( i=0 ; i < num_bindings ; ++i )
    {
      scheme_add_binding (i, scheme_false, scheme_false, frame);
    }
  env = scheme_extend_env (frame, env);
  i = 0;
  while ( vars != scheme_null )
    {
      scheme_add_binding (i, SCHEME_CAR(vars), scheme_eval (SCHEME_CAR (vals), env), env);
      vars = SCHEME_CDR (vars);
      vals = SCHEME_CDR (vals);
      i++;
    }
  forms = SCHEME_CDR (SCHEME_CDR (form));
  /* process internal defines */
  num_int_defs = 0;
  vars = vals = vars_last = vals_last = scheme_null;
  while (!SCHEME_NULLP(forms) && internal_def_p (SCHEME_CAR(forms)))
    {
      num_int_defs++;
      aform = SCHEME_CAR (forms);
      /* get var */
      if (SCHEME_PAIRP (SCHEME_CAR (SCHEME_CDR (aform))))
	{
	  pair = scheme_make_pair (SCHEME_CAR (SCHEME_CAR (SCHEME_CDR (aform))), scheme_null);
	}
      else
	{
	  pair = scheme_make_pair (SCHEME_CAR (SCHEME_CDR (aform)), scheme_null);
	}
      if (SCHEME_NULLP (vars))
	{
	  vars = vars_last = pair;
	}
      else
	{
	  SCHEME_CDR (vars_last) = pair;
	  vars_last = pair;
	}
      /* get val */
      if (SCHEME_PAIRP (SCHEME_CAR (SCHEME_CDR (aform))))
	{
	  pair = CONS (CONS (scheme_lambda, CONS (SCHEME_CDR (SCHEME_CAR (SCHEME_CDR (aform))),
						  SCHEME_CDR (SCHEME_CDR (aform)))),
		       scheme_null);
	}
      else
	{
	  pair = scheme_make_pair (SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (aform))), scheme_null);
	}
      if (SCHEME_NULLP (vals))
	{
	  vals = vals_last = pair;
	}
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
  /* evaluate non-internal-definition forms */
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

static Scheme_Object *
letrec_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *bindings, *vars, *vals, *forms, *res;
  Scheme_Object *vars_last, *vals_last, *pair, *aform;
  int num_int_defs;

  bindings = SCHEME_CAR (SCHEME_CDR (form));
  vars = scheme_map_1 (scheme_car, bindings);
  vals = scheme_map_1 (scheme_cadr, bindings);

  env = scheme_add_frame (vars, scheme_alloc_list (scheme_list_length (vars)), env);
  while (! SCHEME_NULLP (vars))
    {
      scheme_set_value (SCHEME_CAR (vars),
			scheme_eval (SCHEME_CAR (vals), env),
			env);
      vars = SCHEME_CDR (vars);
      vals = SCHEME_CDR (vals);
    }
  forms = SCHEME_CDR (SCHEME_CDR (form));

  /* process internal defines */
  num_int_defs = 0;
  vars = vals = vars_last = vals_last = scheme_null;
  while (!SCHEME_NULLP(forms) && internal_def_p (SCHEME_CAR(forms)))
    {
      num_int_defs++;
      aform = SCHEME_CAR (forms);
      /* get var */
      if (SCHEME_PAIRP (SCHEME_CAR (SCHEME_CDR (aform))))
	{
	  pair = scheme_make_pair (SCHEME_CAR (SCHEME_CAR (SCHEME_CDR (aform))), scheme_null);
	}
      else
	{
	  pair = scheme_make_pair (SCHEME_CAR (SCHEME_CDR (aform)), scheme_null);
	}
      if (SCHEME_NULLP (vars))
	{
	  vars = vars_last = pair;
	}
      else
	{
	  SCHEME_CDR (vars_last) = pair;
	  vars_last = pair;
	}
      /* get val */
      if (SCHEME_PAIRP (SCHEME_CAR (SCHEME_CDR (aform))))
	{
	  pair = CONS (CONS (scheme_lambda, CONS (SCHEME_CDR (SCHEME_CAR (SCHEME_CDR (aform))),
						  SCHEME_CDR (SCHEME_CDR (aform)))),
		       scheme_null);
	}
      else
	{
	  pair = scheme_make_pair (SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (aform))), scheme_null);
	}
      if (SCHEME_NULLP (vals))
	{
	  vals = vals_last = pair;
	}
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
  while (! SCHEME_NULLP (forms))
    {
      res = scheme_eval (SCHEME_CAR (forms), env);
      forms = SCHEME_CDR (forms);
    }
  /* pop internal define frame */
  if ( num_int_defs )
    {
      env = scheme_pop_frame (env);
    }
  /* pop regular binding frame */
  env = scheme_pop_frame (env);
  return (res);
}

static Scheme_Object *
begin_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *forms, *ret;

  ret = scheme_false;
  forms = SCHEME_CDR (form);
  while (forms != scheme_null)
    {
      ret = scheme_eval (SCHEME_CAR (forms), env);
      forms = SCHEME_CDR (forms);
    }
  return (ret);
}

static Scheme_Object *
do_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *second, *third;
  Scheme_Object *vars, *inits, *steps;
  Scheme_Object *test, *finals, *forms;
  Scheme_Object *ret, *temp, *temp2;
  Scheme_Object *step_first, *step_last, *clause, *pair;

  second = SCHEME_CAR (SCHEME_CDR (form));
  vars = scheme_map_1 (scheme_car, second);
  inits = scheme_map_1 (scheme_cadr, second);

  /* We can't just map across the steps because
     the could be missing. */
  temp = second;
  step_first = step_last = scheme_null;
  while (! SCHEME_NULLP (temp))
    {
      clause = SCHEME_CAR (temp);
      if (SCHEME_NULLP (SCHEME_CDR (SCHEME_CDR (clause))))
	{
	  pair = scheme_make_pair (SCHEME_CAR (clause), scheme_null);
	}
      else
	{
	  pair = scheme_make_pair (SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (clause))), scheme_null);
	}
      if (SCHEME_NULLP (step_first))
	{
	  step_first = step_last = pair;
	}
      else
	{
	  SCHEME_CDR (step_last) = pair;
	  step_last = pair;
	}
      temp = SCHEME_CDR (temp);
    }
  steps = step_first;

  third = SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (form)));
  test = SCHEME_CAR (third);
  finals = SCHEME_CDR (third);

  forms = SCHEME_CDR (SCHEME_CDR (SCHEME_CDR (form)));

  /* first evaluate the inits */
  temp = inits;
  while (! SCHEME_NULLP (temp))
    {
      SCHEME_CAR(temp) = scheme_eval (SCHEME_CAR(temp), env);
      temp = SCHEME_CDR (temp);
    }
  /* bind the vars to the initial values */
  env = scheme_add_frame (vars, inits, env);
  
  ret = scheme_null;
  while (scheme_eval (test, env) == scheme_false)
    {
      /* evaluate forms in body */
      temp = forms;
      while (! SCHEME_NULLP (temp))
	{
	  ret = scheme_eval (SCHEME_CAR(temp), env);
	  temp = SCHEME_CDR (temp);
	}
      /* evaluate step expressions (all in the old env)
	 and rebind the vars */
      temp = steps;
      step_first = step_last = scheme_null;
      while (! SCHEME_NULLP (temp))
	{
	  pair = scheme_make_pair (scheme_eval (SCHEME_CAR(temp), env), scheme_null);
	  if (step_first == scheme_null)
	    {
	      step_first = step_last = pair;
	    }
	  else
	    {
	      SCHEME_CDR (step_last) = pair;
	      step_last = pair;
	    }
	  temp = SCHEME_CDR (temp);
	}
      env = scheme_pop_frame (env);
      env = scheme_add_frame (vars, step_first, env);
    }
  /* evaluate the return forms */
  temp = finals;
  while (! SCHEME_NULLP (temp))
    {
      ret = scheme_eval (SCHEME_CAR (temp), env);
      temp = SCHEME_CDR (temp);
    }

  /* pop the frame we've added */
  env = scheme_pop_frame (env);
  return (ret);
}

static Scheme_Object *
delay_syntax (Scheme_Object *form, Scheme_Env *env)
{
  SCHEME_ASSERT ((scheme_list_length(form) == 2), "delay: bad form");
  return (scheme_make_promise  (SCHEME_CAR (SCHEME_CDR (form)), env));
}

static Scheme_Object *quasi (Scheme_Object *x, int level, Scheme_Env *env);

static Scheme_Object *
quasiquote_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *template;

  SCHEME_ASSERT ((scheme_list_length (form) == 2), "quasiquote(`): wrong number of args");
  return (quasi (SCHEME_CAR (SCHEME_CDR (form)), 0, env));
}

static Scheme_Object *
quasi (Scheme_Object *x, int level, Scheme_Env *env)
{
  Scheme_Object *form, *list, *tail, *cell, *qcar, *qcdr, *ret;

  if (SCHEME_VECTORP (x))
    {
      x = scheme_vector_to_list (x);
      x = quasi (x, level, env);
      return (scheme_list_to_vector (x));
    }
  if (! SCHEME_PAIRP (x))
    {
      return x;
    }
  if (SCHEME_CAR (x) == scheme_unquote)
    {
      x = SCHEME_CDR (x);
      SCHEME_ASSERT (SCHEME_PAIRP (x), "bad unquote form");
      if (level) 
	{
	  ret = scheme_make_pair (SCHEME_CAR (x), scheme_null);
	  ret = quasi (ret, level-1, env);
	  ret = scheme_make_pair (scheme_unquote, ret);
        } 
      else 
	{
	  ret = scheme_eval (SCHEME_CAR (x), env);
	}
      return ret;
    } 
  else if (SCHEME_PAIRP (SCHEME_CAR (x))
	   && SCHEME_CAR (SCHEME_CAR (x)) == scheme_unquote_splicing)
    {
      qcdr = SCHEME_CDR (x);
      form = list = tail = cell = scheme_null;
      x = SCHEME_CAR (x);
      SCHEME_ASSERT (SCHEME_PAIRP (SCHEME_CDR (x)), "bad unquote-splicing form");
      if (level) 
	{
	  list = quasi (SCHEME_CDR (x), level-1, env);
	  list = scheme_make_pair (scheme_unquote_splicing, list);
	  qcdr = quasi (qcdr, level, env);
	  list = scheme_make_pair (list, qcdr);
	  return list;
	}
      form = scheme_eval (SCHEME_CAR (SCHEME_CDR (x)), env);
      for ( ; SCHEME_PAIRP(form) ; tail = cell, form = SCHEME_CDR (form)) 
	{
	  cell = scheme_make_pair (SCHEME_CAR (form), scheme_null);
	  if (SCHEME_NULLP (list))
	    list = cell;
	  else
	    SCHEME_CDR(tail) = cell;
	}
      qcdr = quasi (qcdr, level, env);
      if (SCHEME_NULLP (list))
	{
	  return qcdr;
	}
      SCHEME_CDR (tail) = qcdr;
      return list;
    } 
  else 
    {
      qcar = qcdr = scheme_null;
      if (SCHEME_CAR (x) == scheme_quasiquote)   /* hack! */
	{
	  ++level;
	}
      qcar = quasi (SCHEME_CAR (x), level, env);
      qcdr = quasi (SCHEME_CDR (x), level, env);
      list = scheme_make_pair (qcar, qcdr);
      return list;
    }
}

static Scheme_Object *
defmacro_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *name, *code;
  Scheme_Object *fun, *macro;

  SCHEME_ASSERT ((scheme_list_length (form) > 3), "badly formed defmacro");
  name = SCHEME_CAR (SCHEME_CDR (form));
  SCHEME_ASSERT (SCHEME_SYMBOLP (name), "defmacro: second arg must be a symbol");
  code = SCHEME_CDR (SCHEME_CDR (form));
  fun = scheme_make_closure (env, code);
  
  macro = scheme_alloc_object ();
  SCHEME_TYPE (macro) = scheme_macro_type;
  SCHEME_PTR_VAL (macro) = fun;
  scheme_add_global (SCHEME_STR_VAL (name), macro, env);
  return (macro);
}
