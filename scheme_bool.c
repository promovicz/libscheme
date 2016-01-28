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
#include <string.h>

/* globals */
Scheme_Object *scheme_true;
Scheme_Object *scheme_false;
Scheme_Object *scheme_true_type;
Scheme_Object *scheme_false_type;

/* locals */
static Scheme_Object *scheme_make_true (void);
static Scheme_Object *scheme_make_false (void);
static Scheme_Object *not_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *boolean_p_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *eq_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *eqv_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *equal_prim (int argc, Scheme_Object *argv[]);

static int list_equal (Scheme_Object *lst1, Scheme_Object *lst2);
static int vector_equal (Scheme_Object *vec1, Scheme_Object *vec2);

void
scheme_init_bool (Scheme_Env *env)
{
  scheme_true_type = scheme_make_type ("<true>");
  scheme_false_type = scheme_make_type ("<false>");
  scheme_add_global ("<true>", scheme_true_type, env);
  scheme_add_global ("<false>", scheme_false_type, env);
  scheme_true = scheme_make_true ();
  scheme_false = scheme_make_false ();
  scheme_add_global ("not", scheme_make_prim (not_prim), env);
  scheme_add_global ("boolean?", scheme_make_prim (boolean_p_prim), env);
  scheme_add_global ("eq?", scheme_make_prim (eq_prim), env);
  scheme_add_global ("eqv?", scheme_make_prim (eqv_prim), env);
  scheme_add_global ("equal?", scheme_make_prim (equal_prim), env);
}

static Scheme_Object *
scheme_make_true (void)
{
  Scheme_Object *true;

  true = scheme_alloc_object ();
  SCHEME_TYPE (true) = scheme_true_type;
  return (true);
}

static Scheme_Object *
scheme_make_false (void)
{
  Scheme_Object *false;

  false = scheme_alloc_object ();
  SCHEME_TYPE (false) = scheme_false_type;
  return (false);
}

static Scheme_Object *
not_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT (argc == 1, "not: wrong number of args");
  if (argv[0] == scheme_false)
    {
      return (scheme_true);
    }
  else
    {
      return (scheme_false);
    }
}

static Scheme_Object *
boolean_p_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "boolean?: wrong number of args");
  if ((argv[0] == scheme_false) || (argv[0] == scheme_true))
    {
      return (scheme_true);
    }
  else
    {
      return (scheme_false);
    }
}

static Scheme_Object *
eq_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 2), "eq?: wrong number of args");
  if (argv[0] == argv[1])
    {
      return (scheme_true);
    }
  else
    {
      return (scheme_false);
    }
}

static Scheme_Object *
eqv_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc==2), "eqv?: wrong number of args");
  if (scheme_eqv (argv[0], argv[1]))
    {
      return (scheme_true);
    }
  else
    {
      return (scheme_false);
    }
}

static Scheme_Object *
equal_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 2), "equal?: wrong number of args");
  if (scheme_equal (argv[0], argv[1]))
    {
      return (scheme_true);
    }
  else
    {
      return (scheme_false);
    }
}

int 
scheme_eq (Scheme_Object *obj1, Scheme_Object *obj2)
{
  return (obj1 == obj2);
}

int
scheme_eqv (Scheme_Object *obj1, Scheme_Object *obj2)
{
  if (obj1 == obj2)
    {
      return 1;
    }
  else if (SCHEME_TYPE(obj1) != SCHEME_TYPE(obj2))
    {
      return 0;
    }
  else if (SCHEME_TYPE(obj1) == scheme_integer_type &&
	   SCHEME_INT_VAL(obj1) == SCHEME_INT_VAL(obj2))
    {
      return 1;
    }
  else if (SCHEME_TYPE(obj1)== scheme_double_type &&
	   SCHEME_DBL_VAL(obj1) == SCHEME_DBL_VAL(obj2))
    {
      return 1;
    }
  else if (SCHEME_TYPE(obj1) == scheme_char_type &&
	   SCHEME_CHAR_VAL(obj1) == SCHEME_CHAR_VAL(obj2))
    {
      return 1;
    }
  else if (SCHEME_SYMBOLP(obj1))
    {
      if (strcmp (SCHEME_STR_VAL(obj1), SCHEME_STR_VAL(obj2)) == 0)
	return 1;
      else
	return 0;
    }
  else
    {
      return 0;
    }
}

int
scheme_equal (Scheme_Object *obj1, Scheme_Object *obj2)
{
  if (scheme_eqv (obj1, obj2))
    {
      return 1;
    }
  else if (SCHEME_TYPE(obj1) != SCHEME_TYPE(obj2))
    {
      return 0;
    }
  else if (SCHEME_TYPE(obj1) == scheme_pair_type &&
	   list_equal(obj1, obj2))
    {
      return 1;
    }
  else if (SCHEME_TYPE(obj1) == scheme_vector_type &&
	   vector_equal(obj1, obj2))
    {
      return 1;
    }
  else if (SCHEME_TYPE(obj1) == scheme_string_type &&
	   (strcmp(SCHEME_STR_VAL(obj1), SCHEME_STR_VAL(obj2)) == 0))
    {
      return 1;
    }
  else 
    {
      return 0;
    }
}

static int
list_equal (Scheme_Object *lst1, Scheme_Object *lst2)
{
  if ((lst1 == scheme_null) && (lst2 == scheme_null))
    {
      return 1;
    }
  else
    {
      return (scheme_equal (SCHEME_CAR (lst1), SCHEME_CAR (lst2)) &&
	      scheme_equal (SCHEME_CDR (lst1), SCHEME_CDR (lst2)));
    }
}

static int
vector_equal (Scheme_Object *vec1, Scheme_Object *vec2)
{
  int i;

  for ( i=0 ; i<SCHEME_VEC_SIZE(vec1) ; ++i )
    {
      if (! scheme_equal (SCHEME_VEC_ELS(vec1)[i], SCHEME_VEC_ELS(vec2)[i]))
	{
	  return 0;
	}
    }
  return 1;
}



