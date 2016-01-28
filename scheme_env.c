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

#define GLOBAL_TABLE_SIZE 100313
#define MAX_SYMBOL_SIZE 1023

/* globals */
Scheme_Env *scheme_env;

/* locals */
static Scheme_Env *scheme_make_env (void);

Scheme_Env *
scheme_basic_env (void)
{
  Scheme_Env *env;

  /* The ordering of the first few init calls is important. 
     Add to the end of the list, not the beginning. */
  env = scheme_make_env ();
  scheme_init_type (env);
  scheme_init_fun (env);
  scheme_init_symbol (env);
  scheme_init_list (env);
  scheme_init_number (env);
  scheme_init_port (env);
  scheme_init_string (env);
  scheme_init_vector (env);
  scheme_init_char (env);
  scheme_init_bool (env);
  scheme_init_syntax (env);
  scheme_init_eval (env);
  scheme_init_error (env);
  scheme_init_promise (env);
  scheme_init_struct (env);
  scheme_env = env;
  return (env);
}

static Scheme_Env *
scheme_make_env (void)
{
  Scheme_Env *env;

  env = (Scheme_Env *) scheme_malloc (sizeof (Scheme_Env));
  env->globals = scheme_hash_table (GLOBAL_TABLE_SIZE);
  env->next = NULL;
  return (env);
}

void
scheme_add_global (char *name, Scheme_Object *obj, Scheme_Env *env)
{
  char lower_name[MAX_SYMBOL_SIZE];
  int i;
  
  i = 0;
  while ( name[i] )
    {
      lower_name[i] = tolower (name[i]);
      i++;
    }
  lower_name[i] = '\0';
  scheme_add_to_table (env->globals, lower_name, obj);
}

Scheme_Env *
scheme_new_frame (int num_bindings)
{
  Scheme_Env *frame;
  
  frame = (Scheme_Env *) scheme_malloc (sizeof (Scheme_Env));
  frame->num_bindings = num_bindings;
  frame->symbols = (Scheme_Object **) scheme_malloc (num_bindings * sizeof (Scheme_Object*));
  frame->values = (Scheme_Object **) scheme_malloc (num_bindings * sizeof (Scheme_Object*));
  return (frame);
}

void
scheme_add_binding (int index, Scheme_Object *sym, Scheme_Object *val, Scheme_Env *frame)
{
  if ((index >= frame->num_bindings) || (index < 0))
    {
      scheme_signal_error ("internal error: scheme_add_binding() index out of range: %d", index);
    }
  frame->symbols[index] = sym;
  frame->values[index] = val;
}

Scheme_Env *
scheme_extend_env (Scheme_Env *frame, Scheme_Env *env)
{
  frame->globals = env->globals;
  frame->next = env;
  return (frame);
}

Scheme_Env *
scheme_add_frame (Scheme_Object *syms, Scheme_Object *vals, Scheme_Env *env)
{
  Scheme_Env *frame;
  int len, i;

  frame = (Scheme_Env *) scheme_malloc (sizeof (Scheme_Env));
  len = scheme_list_length (syms);
  frame->num_bindings = len;
  frame->symbols = (Scheme_Object **) scheme_malloc (len * sizeof (Scheme_Object*));
  frame->values = (Scheme_Object **) scheme_malloc (len * sizeof (Scheme_Object*));
  for ( i=0 ; i<len ; ++i )
    {
      if (SCHEME_SYMBOLP(syms))
	{
	  frame->symbols[i] = syms;
	  frame->values[i] = vals;
	}
      else
	{
	  frame->symbols[i] = SCHEME_CAR (syms);
	  frame->values[i] = SCHEME_CAR (vals);
	  syms = SCHEME_CDR (syms);
	  vals = SCHEME_CDR (vals);
	}
    }
  frame->globals = env->globals;
  frame->next = env;
  scheme_env = frame;
  return (frame);
}

Scheme_Env *
scheme_pop_frame (Scheme_Env *env)
{
  SCHEME_ASSERT ((env->next), "trying to pop a frame from an empty environment");
  scheme_env = env->next;
  return (env->next);
}

void 
scheme_set_value (Scheme_Object *symbol, Scheme_Object *val, Scheme_Env *env)
{
  Scheme_Env *frame;

  frame = env;
  while ( frame->next != NULL )
    {
      int i;

      for ( i=0 ; i<frame->num_bindings ; ++i )
	{
	  if (symbol == frame->symbols[i])
	    {
	      frame->values[i] = val;
	      return;
	    }
	}
      frame = frame->next;
    }
  if (scheme_lookup_global (symbol, frame))
    {
      scheme_change_in_table (frame->globals, SCHEME_STR_VAL (symbol), val);
    }
  else
    {
      scheme_signal_error ("set!: var unbound: %s", SCHEME_STR_VAL(symbol));
    }
}

Scheme_Object *
scheme_lookup_value (Scheme_Object *symbol, Scheme_Env *env)
{
  Scheme_Env *frame;

  frame = env;
  while ( frame->next != NULL )
    {
      int i;

      for ( i=0 ; i<frame->num_bindings ; ++i )
	{
	  if (symbol == frame->symbols[i])
	    {
	      return (frame->values[i]);
	    }
	}
      frame = frame->next;
    }
  return (scheme_lookup_global (symbol, frame));
}

Scheme_Object *
scheme_lookup_global (Scheme_Object *symbol, Scheme_Env *env)
{
  return (scheme_lookup_in_table (env->globals, SCHEME_STR_VAL(symbol)));
}
