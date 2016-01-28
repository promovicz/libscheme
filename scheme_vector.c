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
Scheme_Object *scheme_vector_type;

/* locals */
static Scheme_Object *vector_p (int argc, Scheme_Object *argv[]);
static Scheme_Object *make_vector (int argc, Scheme_Object *argv[]);
static Scheme_Object *vector (int argc, Scheme_Object *argv[]);
static Scheme_Object *vector_length (int argc, Scheme_Object *argv[]);
static Scheme_Object *vector_ref (int argc, Scheme_Object *argv[]);
static Scheme_Object *vector_set (int argc, Scheme_Object *argv[]);
static Scheme_Object *vector_to_list (int argc, Scheme_Object *argv[]);
static Scheme_Object *list_to_vector (int argc, Scheme_Object *argv[]);
static Scheme_Object *vector_fill (int argc, Scheme_Object *argv[]);

static Scheme_Object *vector_append (int argc, Scheme_Object *argv[]);

void
scheme_init_vector (Scheme_Env *env)
{
  scheme_vector_type = scheme_make_type ("<vector>");
  scheme_add_global ("<vector>", scheme_vector_type, env);
  scheme_add_global ("vector?", scheme_make_prim (vector_p), env);
  scheme_add_global ("make-vector", scheme_make_prim (make_vector), env);
  scheme_add_global ("vector", scheme_make_prim (vector), env);
  scheme_add_global ("vector-length", scheme_make_prim (vector_length), env);
  scheme_add_global ("vector-ref", scheme_make_prim (vector_ref), env);
  scheme_add_global ("vector-set!", scheme_make_prim (vector_set), env);
  scheme_add_global ("vector->list", scheme_make_prim (vector_to_list), env);
  scheme_add_global ("list->vector", scheme_make_prim (list_to_vector), env);
  scheme_add_global ("vector-fill!", scheme_make_prim (vector_fill), env);

  scheme_add_global ("vector-append", scheme_make_prim (vector_append), env);
}

Scheme_Object *
scheme_make_vector (int size, Scheme_Object *fill)
{
  Scheme_Object *vec;
  int i;

  vec = scheme_alloc_object ();
  SCHEME_TYPE(vec) = scheme_vector_type;
  SCHEME_VEC_SIZE(vec) = size;
  SCHEME_VEC_ELS(vec) = (Scheme_Object**)scheme_malloc(sizeof(Scheme_Object*)*size);
  for ( i=0 ; i<size ; ++i )
    {
      SCHEME_VEC_ELS(vec)[i] = fill;
    }
  return (vec);
}

/* locals */

static Scheme_Object *
vector_p (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "vector?: wrong number of args");
  return (SCHEME_VECTORP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Object *
make_vector (int argc, Scheme_Object *argv[])
{
  Scheme_Object *vec, *fill;
  int len;

  SCHEME_ASSERT ((argc==1 || argc==2), "make-vector: wrong number of args");
  SCHEME_ASSERT (SCHEME_INTP (argv[0]), "make-vector: first arg must be an integer");
  len = SCHEME_INT_VAL (argv[0]);
  if (argc == 2)
    {
      fill = argv[1];
    }
  else
    {
      fill = scheme_false;
    }
  vec = scheme_make_vector (len, fill);
  return (vec);
}

static Scheme_Object *
vector (int argc, Scheme_Object *argv[])
{
  Scheme_Object *vec;
  int i;

  vec = scheme_make_vector (argc, 0);
  for ( i=0 ; i<argc ; ++i )
    {
      SCHEME_VEC_ELS(vec)[i] = argv[i];
    }
  return (vec);
}

static Scheme_Object *
vector_length (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "vector-length: wrong number of args");
  SCHEME_ASSERT (SCHEME_VECTORP (argv[0]), "vector-length: arg must be a vector");
  return (scheme_make_integer (SCHEME_VEC_SIZE (argv[0])));
}

static Scheme_Object *
vector_ref (int argc, Scheme_Object *argv[])
{
  int i;

  SCHEME_ASSERT ((argc == 2), "vector-ref: wrong number of args");
  SCHEME_ASSERT (SCHEME_VECTORP (argv[0]), "vector-ref: first arg must be a vector");
  SCHEME_ASSERT (SCHEME_INTP (argv[1]), "vector-ref: second arg must be an integer");
  i = SCHEME_INT_VAL (argv[1]);
  SCHEME_ASSERT ((i >= 0) && (i < SCHEME_VEC_SIZE (argv[0])),
		 "vector-ref: index out of range");
  return (SCHEME_VEC_ELS(argv[0])[i]);
}

static Scheme_Object *
vector_set (int argc, Scheme_Object *argv[])
{
  int i;

  SCHEME_ASSERT ((argc == 3), "vector-set!: wrong number of args");
  SCHEME_ASSERT (SCHEME_VECTORP (argv[0]), "vector-set!: first arg must be a vector");
  SCHEME_ASSERT (SCHEME_INTP (argv[1]), "vector-set!: second arg must be an integer");
  i = SCHEME_INT_VAL (argv[1]);
  SCHEME_ASSERT ((i >= 0) && (i < SCHEME_VEC_SIZE (argv[0])),
		 "vector-ref: index out of range");
  SCHEME_VEC_ELS(argv[0])[i] = argv[2];
  return (argv[0]);
}

static Scheme_Object *
vector_to_list (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "vector->list: wrong number of args");
  SCHEME_ASSERT (SCHEME_VECTORP(argv[0]), "vector->list: arg must be a vector");
  return (scheme_vector_to_list (argv[0]));
}

Scheme_Object *
scheme_vector_to_list (Scheme_Object *vec)
{
  int len, i;
  Scheme_Object *first, *last, *pair;

  len = SCHEME_VEC_SIZE (vec);
  first = last = scheme_null;
  for ( i=0 ; i<len ; ++i )
    {
      pair = scheme_make_pair (SCHEME_VEC_ELS(vec)[i], scheme_null);
      if (first == scheme_null)
	{
	  first = last = pair;
	}
      else
	{
	  SCHEME_CDR (last) = pair;
	  last = pair;
	}
    }
  return (first);
}

static Scheme_Object *
list_to_vector (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "list->vector: wrong number of args");
  SCHEME_ASSERT (SCHEME_LISTP(argv[0]), "list->vector: arg must be a list");
  return (scheme_list_to_vector (argv[0]));
}

Scheme_Object *
scheme_list_to_vector (Scheme_Object *list)
{
  int len, i;
  Scheme_Object *vec;

  len = scheme_list_length (list);
  vec = scheme_make_vector (len, 0);
  i = 0;
  while (! SCHEME_NULLP (list))
    {
      SCHEME_VEC_ELS(vec)[i] = SCHEME_CAR (list);
      i++;
      list = SCHEME_CDR (list);
    }
  return (vec);
}

static Scheme_Object *
vector_fill (int argc, Scheme_Object *argv[])
{
  int i;
  
  SCHEME_ASSERT ((argc == 2), "vector-fill!: wrong number of args");
  SCHEME_ASSERT (SCHEME_VECTORP (argv[0]), "vector-fill!: first arg must be a vector");
  for ( i=0 ; i<SCHEME_VEC_SIZE (argv[0]) ; ++i )
    {
      SCHEME_VEC_ELS(argv[0])[i] = argv[1];
    }
  return (argv[0]);
}

static Scheme_Object *
vector_append (int argc, Scheme_Object *argv[])
{
  int len1, len2, i;
  Scheme_Object *new;

  SCHEME_ASSERT ((argc == 2), "vector-append: wrong number of args");
  SCHEME_ASSERT (SCHEME_VECTORP (argv[0]) && SCHEME_VECTORP (argv[1]),
		 "vector-append: both args must be vectors");
  len1 = SCHEME_VEC_SIZE (argv[0]);
  len2 = SCHEME_VEC_SIZE (argv[1]);
  new = scheme_make_vector ((len1 + len2), NULL);
  for ( i=0 ; i<len1 ; ++i )
    {
      SCHEME_VEC_ELS(new)[i] = SCHEME_VEC_ELS(argv[0])[i];
    }
  for ( i=len1 ; i<(len1 + len2) ; ++i )
    {
      SCHEME_VEC_ELS(new)[i] = SCHEME_VEC_ELS(argv[1])[i-len1];
    }
  return (new);
}
