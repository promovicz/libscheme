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
Scheme_Value scheme_null;
Scheme_Value scheme_null_type;
Scheme_Value scheme_pair_type;

/* primitive declarations */
static Scheme_Value pair_p_prim (int argc, Scheme_Value argv[]);
static Scheme_Value cons_prim (int argc, Scheme_Value argv[]);
static Scheme_Value car_prim (int argc, Scheme_Value argv[]);
static Scheme_Value cdr_prim (int argc, Scheme_Value argv[]);
static Scheme_Value set_car_prim (int argc, Scheme_Value argv[]);
static Scheme_Value set_cdr_prim (int argc, Scheme_Value argv[]);
static Scheme_Value null_p_prim (int argc, Scheme_Value argv[]);
static Scheme_Value list_p_prim (int argc, Scheme_Value argv[]);
static Scheme_Value list_prim (int argc, Scheme_Value argv[]);
static Scheme_Value length_prim (int argc, Scheme_Value argv[]);
static Scheme_Value append_prim (int argc, Scheme_Value argv[]);
static Scheme_Value reverse_prim (int argc, Scheme_Value argv[]);
static Scheme_Value list_tail_prim (int argc, Scheme_Value argv[]);
static Scheme_Value list_ref_prim (int argc, Scheme_Value argv[]);
static Scheme_Value memv (int argc, Scheme_Value argv[]);
static Scheme_Value memq (int argc, Scheme_Value argv[]);
static Scheme_Value member (int argc, Scheme_Value argv[]);
static Scheme_Value assv (int argc, Scheme_Value argv[]);
static Scheme_Value assq (int argc, Scheme_Value argv[]);
static Scheme_Value assoc (int argc, Scheme_Value argv[]);
static Scheme_Value caar_prim (int argc, Scheme_Value argv[]);
static Scheme_Value cadr_prim (int argc, Scheme_Value argv[]);
static Scheme_Value cdar_prim (int argc, Scheme_Value argv[]);
static Scheme_Value cddr_prim (int argc, Scheme_Value argv[]);
static Scheme_Value caaar_prim (int argc, Scheme_Value argv[]);
static Scheme_Value caadr_prim (int argc, Scheme_Value argv[]);
static Scheme_Value cadar_prim (int argc, Scheme_Value argv[]);
static Scheme_Value cdaar_prim (int argc, Scheme_Value argv[]);
static Scheme_Value cdadr_prim (int argc, Scheme_Value argv[]);
static Scheme_Value cddar_prim (int argc, Scheme_Value argv[]);
static Scheme_Value caddr_prim (int argc, Scheme_Value argv[]);
static Scheme_Value cdddr_prim (int argc, Scheme_Value argv[]);

/* internal declarations */
static Scheme_Value append (Scheme_Value lst1, Scheme_Value lst2);

/* exported functions */

void
scheme_init_list (Scheme_Env *env)
{
  scheme_null_type = scheme_make_type ("<empty-list>");
  scheme_add_global ("<empty-list>", scheme_null_type, env);
  scheme_null = scheme_alloc_object (scheme_null_type, 0);
  scheme_pair_type = scheme_make_type ("<pair>");
  scheme_add_global ("<pair>", scheme_pair_type, env);
  scheme_add_prim ("pair?", pair_p_prim, env);
  scheme_add_prim ("cons", cons_prim, env);
  scheme_add_prim ("car", car_prim, env);
  scheme_add_prim ("cdr", cdr_prim, env);
  scheme_add_prim ("set-car!", set_car_prim, env);
  scheme_add_prim ("set-cdr!", set_cdr_prim, env);
  scheme_add_prim ("null?", null_p_prim, env);
  scheme_add_prim ("list?", list_p_prim, env);
  scheme_add_prim ("list", list_prim, env);
  scheme_add_prim ("length", length_prim, env);
  scheme_add_prim ("append", append_prim, env);
  scheme_add_prim ("reverse", reverse_prim, env);
  scheme_add_prim ("list-tail", list_tail_prim, env);
  scheme_add_prim ("list-ref", list_ref_prim, env);
  scheme_add_prim ("memq", memq, env);
  scheme_add_prim ("memv", memv, env);
  scheme_add_prim ("member", member, env);
  scheme_add_prim ("assq", assq, env);
  scheme_add_prim ("assv", assv, env);
  scheme_add_prim ("assoc", assoc, env);
  scheme_add_prim ("caar", caar_prim, env);
  scheme_add_prim ("cadr", cadr_prim, env);
  scheme_add_prim ("cdar", cdar_prim, env);
  scheme_add_prim ("cddr", cddr_prim, env);
  scheme_add_prim ("caaar", caaar_prim, env);
  scheme_add_prim ("caadr", caadr_prim, env);
  scheme_add_prim ("cadar", cadar_prim, env);
  scheme_add_prim ("cdaar", cdaar_prim, env);
  scheme_add_prim ("cdadr", cdadr_prim, env);
  scheme_add_prim ("cddar", cddar_prim, env);
  scheme_add_prim ("caddr", caddr_prim, env);
  scheme_add_prim ("cdddr", cdddr_prim, env);
}

Scheme_Value
scheme_make_pair (Scheme_Value car, Scheme_Value cdr)
{
  Scheme_Value cons;

  cons = scheme_alloc_object (scheme_pair_type, 0);
  SCHEME_CAR(cons) = car;
  SCHEME_CDR(cons) = cdr;
  return (cons);
}

Scheme_Value
scheme_alloc_list (int size)
{
  Scheme_Value first, last, pair;
  int i;

  first = last = scheme_null;
  for ( i=0 ; i<size ; ++i )
    {
      pair = scheme_make_pair (scheme_false, scheme_null);
      if (SCHEME_NULLP (first))
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

SCHEME_FUN_PURE
int
scheme_list_length (Scheme_Value list)
{
  int len;

  len = 0;
  while (list != scheme_null)
    {
      len++;
      if (SCHEME_PAIRP(list))
	{
	  list = SCHEME_CDR(list);
	}
      else
	{
	  list = scheme_null;
	}
    }
  return (len);
}

Scheme_Value
scheme_map_1 (Scheme_Value (*fun)(Scheme_Object*), Scheme_Value lst)
{
  if (lst == scheme_null)
    {
      return (scheme_null);
    }
  else
    {
      return (scheme_make_pair (fun (SCHEME_CAR (lst)),
				scheme_map_1 (fun, SCHEME_CDR (lst))));
    }
}

SCHEME_FUN_PURE
Scheme_Value
scheme_car (Scheme_Value pair)
{
  return (SCHEME_CAR (pair));
}

SCHEME_FUN_PURE
Scheme_Value
scheme_cdr (Scheme_Value pair)
{
  return (SCHEME_CDR (pair));
}

SCHEME_FUN_PURE
Scheme_Value
scheme_cadr (Scheme_Value pair)
{
  return (SCHEME_CAR (SCHEME_CDR (pair)));
}

SCHEME_FUN_PURE
Scheme_Value
scheme_caddr (Scheme_Value pair)
{
  return (SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (pair))));
}

/* primitive functions */

static Scheme_Value
pair_p_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "pair?: wrong number of args");
  return ((SCHEME_TYPE (argv[0]) == scheme_pair_type) ? scheme_true : scheme_false);
}

static Scheme_Value
cons_prim (int argc, Scheme_Value argv[])
{
  Scheme_Value cons;

  SCHEME_ASSERT ((argc == 2), "cons: wrong number of args");
  cons = scheme_make_pair (argv[0], argv[1]);
  return (cons);
}

static Scheme_Value
car_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "car: wrong number of args");
  SCHEME_ASSERT (SCHEME_TYPE(argv[0])==scheme_pair_type, "car: arg must be pair");
  return (SCHEME_CAR (argv[0]));
}

static Scheme_Value
cdr_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "cdr: wrong number of args");
  SCHEME_ASSERT (SCHEME_TYPE(argv[0])==scheme_pair_type, "cdr: arg must be pair");
  return (SCHEME_CDR (argv[0]));
}

static Scheme_Value
set_car_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 2), "set-car!: wrong number of args");
  SCHEME_ASSERT (SCHEME_TYPE(argv[0])==scheme_pair_type, "set-car!: first arg must be pair");
  SCHEME_CAR (argv[0]) = argv[1];
  return (argv[1]);
}

static Scheme_Value
set_cdr_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 2), "set-cdr!: wrong number of args");
  SCHEME_ASSERT (SCHEME_TYPE(argv[0])==scheme_pair_type, "set-cdr!: first arg must be pair");
  SCHEME_CDR (argv[0]) = argv[1];
  return (argv[1]);
}

static Scheme_Value
null_p_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "null?: wrong number of args");
  return ((argv[0] == scheme_null) ? scheme_true : scheme_false);
}

static Scheme_Value
list_p_prim (int argc, Scheme_Value argv[])
{
  Scheme_Value obj1, obj2;

  SCHEME_ASSERT ((argc == 1), "list?: wrong number of args");
  obj1 = obj2 = argv[0];
  do
    {
      if (SCHEME_NULLP (obj1))
	{
	  return (scheme_true);
	}
      if (! SCHEME_PAIRP (obj1))
	{
	  return (scheme_false);
	}
      obj1 = SCHEME_CDR (obj1);
      if (SCHEME_NULLP (obj1))
	{
	  return (scheme_true);
	}
      if (! SCHEME_PAIRP (obj1))
	{
	  return (scheme_false);
	}
      obj1 = SCHEME_CDR (obj1);
      obj2 = SCHEME_CDR (obj2);
    }
  while (obj1 != obj2);
  return (scheme_false);
}

static Scheme_Value
list_prim (int argc, Scheme_Value argv[])
{
  int i;
  Scheme_Value first, last, pair;

  first = last = scheme_null;
  for ( i=0 ; i<argc ; ++i )
    {
      pair = scheme_make_pair (argv[i], scheme_null);
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

static Scheme_Value
length_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "length: wrong number of args");
  SCHEME_ASSERT (SCHEME_LISTP(argv[0]), "length: arg must be a list");
  return (scheme_make_integer (scheme_list_length (argv[0])));
}

static Scheme_Value
append_prim (int argc, Scheme_Value argv[])
{
  Scheme_Value res;
  int i;

  res = scheme_null;
  for ( i=0 ; i<argc ; ++i )
    {
      res = append (res, argv[i]);
    }
  return (res);
}

static Scheme_Value
reverse_prim (int argc, Scheme_Value argv[])
{
  Scheme_Value lst, last;

  SCHEME_ASSERT ((argc == 1), "reverse: wrong number of args");
  last = scheme_null;
  lst = argv[0];
  while ( ! SCHEME_NULLP (lst))
    {
      last = scheme_make_pair (SCHEME_CAR (lst), last);
      lst = SCHEME_CDR (lst);
    }
  return (last);
}

static Scheme_Value
list_tail_prim (int argc, Scheme_Value argv[])
{
  int i, k;
  Scheme_Value lst;

  SCHEME_ASSERT ((argc == 2), "list-tail: wrong number of args");
  SCHEME_ASSERT (SCHEME_LISTP(argv[0]), "list-tail: first arg must be a list");
  SCHEME_ASSERT (SCHEME_INTP(argv[1]), "list-tail: second arg must be an integer");
  lst = argv[0];
  k = SCHEME_INT_VAL(argv[1]);
  for ( i=0 ; i<k ; ++i )
    {
      SCHEME_ASSERT (SCHEME_PAIRP (lst), "list-tail: index too large for list");
      lst = SCHEME_CDR (lst);
    }
  return (lst);
}

static Scheme_Value
list_ref_prim (int argc, Scheme_Value argv[])
{
  int i, k;
  Scheme_Value lst;

  SCHEME_ASSERT ((argc == 2), "list-ref: wrong number of args");
  SCHEME_ASSERT (SCHEME_LISTP(argv[0]), "list-ref: first arg must be a list");
  SCHEME_ASSERT (SCHEME_INTP(argv[1]), "list-ref: second arg must be an integer");
  lst = argv[0];
  k = SCHEME_INT_VAL(argv[1]);
  for ( i=0 ; i<k ; ++i )
    {
      lst = SCHEME_CDR (lst);
      SCHEME_ASSERT (SCHEME_PAIRP (lst), "list-ref: index too large for list");
    }
  return (SCHEME_CAR (lst));
}

#define GEN_MEM(name, scheme_name, comp) \
static Scheme_Value  \
name (int argc, Scheme_Value argv[]) \
{ \
  Scheme_Value list; \
  SCHEME_ASSERT ((argc == 2), #scheme_name ": wrong number of args"); \
  SCHEME_ASSERT (SCHEME_LISTP(argv[1]), #scheme_name ": second arg must be list"); \
  list = argv[1]; \
  while (! SCHEME_NULLP (list)) \
    { \
      if (comp (argv[0], SCHEME_CAR (list))) \
	{ \
          return (list); \
	} \
      list = SCHEME_CDR (list); \
    } \
  return (scheme_false); \
}

GEN_MEM(memv, memv, scheme_eqv)
GEN_MEM(memq, memq, scheme_eq)
GEN_MEM(member, member, scheme_equal)

#define GEN_ASS(name, scheme_name, comp) \
static Scheme_Value  \
name (int argc, Scheme_Value argv[]) \
{ \
  Scheme_Value pair, list; \
  SCHEME_ASSERT ((argc == 2), #scheme_name ": wrong number of args"); \
  SCHEME_ASSERT (SCHEME_LISTP(argv[1]), #scheme_name ": second arg must be list"); \
  list = argv[1]; \
  while (! SCHEME_NULLP (list)) \
    { \
      pair = SCHEME_CAR (list); \
      SCHEME_ASSERT (SCHEME_PAIRP (pair), #scheme_name ": second arg must be list of pairs"); \
      if (comp (argv[0], SCHEME_CAR (pair))) \
	{ \
          return (pair); \
	} \
      list = SCHEME_CDR (list); \
    } \
  return (scheme_false); \
}

GEN_ASS(assv, assv, scheme_eqv)
GEN_ASS(assq, assq, scheme_eq)
GEN_ASS(assoc, assoc, scheme_equal)

static Scheme_Value
caar_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT((argc == 1), "caar: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "caar: arg must be a pair");
  return (SCHEME_CAR (SCHEME_CAR (argv[0])));
}

static Scheme_Value
cadr_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT((argc == 1), "cadr: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cadr: arg must be a pair");
  return (SCHEME_CAR (SCHEME_CDR (argv[0])));
}

static Scheme_Value
cdar_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT((argc == 1), "cdar: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cdar: arg must be a pair");
  return (SCHEME_CDR (SCHEME_CAR (argv[0])));
}

static Scheme_Value
cddr_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT((argc == 1), "cddr: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cddr: arg must be a pair");
  return (SCHEME_CDR (SCHEME_CDR (argv[0])));
}

static Scheme_Value
caaar_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT((argc == 1), "caaar: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "caaar: arg must be a pair");
  return (SCHEME_CAR (SCHEME_CAR (SCHEME_CAR (argv[0]))));
}

static Scheme_Value
caadr_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT((argc == 1), "caadr: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "caadr: arg must be a pair");
  return (SCHEME_CAR (SCHEME_CAR (SCHEME_CDR (argv[0]))));
}

static Scheme_Value
cadar_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT((argc == 1), "cadar: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cadar: arg must be a pair");
  return (SCHEME_CAR (SCHEME_CDR (SCHEME_CAR (argv[0]))));
}

static Scheme_Value
cdaar_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT((argc == 1), "cdaar: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cdaar: arg must be a pair");
  return (SCHEME_CDR (SCHEME_CAR (SCHEME_CAR (argv[0]))));
}

static Scheme_Value
cdadr_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT((argc == 1), "cdadr: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cdadr: arg must be a pair");
  return (SCHEME_CDR (SCHEME_CAR (SCHEME_CDR (argv[0]))));
}

static Scheme_Value
cddar_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT((argc == 1), "cddar: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cddar: arg must be a pair");
  return (SCHEME_CDR (SCHEME_CDR (SCHEME_CDR (argv[0]))));
}

static Scheme_Value
caddr_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT((argc == 1), "caddr: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "caddr: arg must be a pair");
  return (SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (argv[0]))));
}

static Scheme_Value
cdddr_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT((argc == 1), "cdddr: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cdddr: arg must be a pair");
  return (SCHEME_CDR (SCHEME_CDR (SCHEME_CDR (argv[0]))));
}

/* internal functions */

static Scheme_Value
append (Scheme_Value lst1, Scheme_Value lst2)
{
  if (SCHEME_NULLP(lst1))
    {
      return (lst2);
    }
  else
    {
      return (scheme_make_pair (SCHEME_CAR (lst1),
				append (SCHEME_CDR (lst1), lst2)));
    }
}
