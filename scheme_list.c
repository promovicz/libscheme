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
Scheme_Object *scheme_null;
Scheme_Object *scheme_null_type, *scheme_pair_type;

/* locals */
static Scheme_Object *scheme_make_null (void);
static Scheme_Object *pair_p_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *cons_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *car_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *cdr_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *set_car_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *set_cdr_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *null_p_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *list_p_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *list_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *length_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *append_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *reverse_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *list_tail_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *list_ref_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *memv (int argc, Scheme_Object *argv[]);
static Scheme_Object *memq (int argc, Scheme_Object *argv[]);
static Scheme_Object *member (int argc, Scheme_Object *argv[]);
static Scheme_Object *assv (int argc, Scheme_Object *argv[]);
static Scheme_Object *assq (int argc, Scheme_Object *argv[]);
static Scheme_Object *assoc (int argc, Scheme_Object *argv[]);
static Scheme_Object *caar_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *cadr_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *cdar_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *cddr_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *caaar_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *caadr_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *cadar_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *cdaar_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *cdadr_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *cddar_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *caddr_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *cdddr_prim (int argc, Scheme_Object *argv[]);

void
scheme_init_list (Scheme_Env *env)
{
  scheme_null_type = scheme_make_type ("<empty-list>");
  scheme_add_global ("<empty-list>", scheme_null_type, env);
  scheme_null = scheme_make_null ();
  scheme_pair_type = scheme_make_type ("<pair>");
  scheme_add_global ("<pair>", scheme_pair_type, env);
  scheme_add_global ("pair?", scheme_make_prim (pair_p_prim), env);
  scheme_add_global ("cons", scheme_make_prim (cons_prim), env);
  scheme_add_global ("car", scheme_make_prim (car_prim), env);
  scheme_add_global ("cdr", scheme_make_prim (cdr_prim), env);
  scheme_add_global ("set-car!", scheme_make_prim (set_car_prim), env);
  scheme_add_global ("set-cdr!", scheme_make_prim (set_cdr_prim), env);
  scheme_add_global ("null?", scheme_make_prim (null_p_prim), env);
  scheme_add_global ("list?", scheme_make_prim (list_p_prim), env);
  scheme_add_global ("list", scheme_make_prim (list_prim), env);
  scheme_add_global ("length", scheme_make_prim (length_prim), env);
  scheme_add_global ("append", scheme_make_prim (append_prim), env);
  scheme_add_global ("reverse", scheme_make_prim (reverse_prim), env);
  scheme_add_global ("list-tail", scheme_make_prim (list_tail_prim), env);
  scheme_add_global ("list-ref", scheme_make_prim (list_ref_prim), env);
  scheme_add_global ("memq", scheme_make_prim (memq), env);
  scheme_add_global ("memv", scheme_make_prim (memv), env);
  scheme_add_global ("member", scheme_make_prim (member), env);
  scheme_add_global ("assq", scheme_make_prim (assq), env);
  scheme_add_global ("assv", scheme_make_prim (assv), env);
  scheme_add_global ("assoc", scheme_make_prim (assoc), env);
  scheme_add_global ("caar", scheme_make_prim (caar_prim), env);
  scheme_add_global ("cadr", scheme_make_prim (cadr_prim), env);
  scheme_add_global ("cdar", scheme_make_prim (cdar_prim), env);
  scheme_add_global ("cddr", scheme_make_prim (cddr_prim), env);
  scheme_add_global ("caaar", scheme_make_prim (caaar_prim), env);
  scheme_add_global ("caadr", scheme_make_prim (caadr_prim), env);
  scheme_add_global ("cadar", scheme_make_prim (cadar_prim), env);
  scheme_add_global ("cdaar", scheme_make_prim (cdaar_prim), env);
  scheme_add_global ("cdadr", scheme_make_prim (cdadr_prim), env);
  scheme_add_global ("cddar", scheme_make_prim (cddar_prim), env);
  scheme_add_global ("caddr", scheme_make_prim (caddr_prim), env);
  scheme_add_global ("cdddr", scheme_make_prim (cdddr_prim), env);
}

static Scheme_Object *
scheme_make_null (void)
{
  Scheme_Object *null;

  null = scheme_alloc_object ();
  SCHEME_TYPE (null) = scheme_null_type;
  return (null);
}

Scheme_Object *
scheme_make_pair (Scheme_Object *car, Scheme_Object *cdr)
{
  Scheme_Object *cons;

  cons = scheme_alloc_object ();
  SCHEME_TYPE(cons) = scheme_pair_type;
  SCHEME_CAR(cons) = car;
  SCHEME_CDR(cons) = cdr;
  return (cons);
}

Scheme_Object *
scheme_alloc_list (int size)
{
  Scheme_Object *first, *last, *pair;
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

int
scheme_list_length (Scheme_Object *list)
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

Scheme_Object *
scheme_map_1 (Scheme_Object *(*fun)(Scheme_Object*), Scheme_Object *lst)
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

Scheme_Object *
scheme_car (Scheme_Object *pair)
{
  return (SCHEME_CAR (pair));
}

Scheme_Object *
scheme_cdr (Scheme_Object *pair)
{
  return (SCHEME_CDR (pair));
}

Scheme_Object *
scheme_cadr (Scheme_Object *pair)
{
  return (SCHEME_CAR (SCHEME_CDR (pair)));
}

Scheme_Object *
scheme_caddr (Scheme_Object *pair)
{
  return (SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (pair))));
}

/* local functions */

static Scheme_Object *
pair_p_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "pair?: wrong number of args");
  return ((SCHEME_TYPE (argv[0]) == scheme_pair_type) ? scheme_true : scheme_false);
}

static Scheme_Object *
cons_prim (int argc, Scheme_Object *argv[])
{
  Scheme_Object *cons;

  SCHEME_ASSERT ((argc == 2), "cons: wrong number of args");
  cons = scheme_make_pair (argv[0], argv[1]);
  return (cons);
}

static Scheme_Object *
car_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "car: wrong number of args");
  SCHEME_ASSERT (SCHEME_TYPE(argv[0])==scheme_pair_type, "car: arg must be pair");
  return (SCHEME_CAR (argv[0]));
}

static Scheme_Object *
cdr_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "cdr: wrong number of args");
  SCHEME_ASSERT (SCHEME_TYPE(argv[0])==scheme_pair_type, "cdr: arg must be pair");
  return (SCHEME_CDR (argv[0]));
}

static Scheme_Object *
set_car_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 2), "set-car!: wrong number of args");
  SCHEME_ASSERT (SCHEME_TYPE(argv[0])==scheme_pair_type, "set-car!: first arg must be pair");
  SCHEME_CAR (argv[0]) = argv[1];
  return (argv[1]);
}

static Scheme_Object *
set_cdr_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 2), "set-cdr!: wrong number of args");
  SCHEME_ASSERT (SCHEME_TYPE(argv[0])==scheme_pair_type, "set-cdr!: first arg must be pair");
  SCHEME_CDR (argv[0]) = argv[1];
  return (argv[1]);
}

static Scheme_Object *
null_p_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "null?: wrong number of args");
  return ((argv[0] == scheme_null) ? scheme_true : scheme_false);
}

static Scheme_Object *
list_p_prim (int argc, Scheme_Object *argv[])
{
  Scheme_Object *obj1, *obj2;

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

static Scheme_Object *
list_prim (int argc, Scheme_Object *argv[])
{
  int i;
  Scheme_Object *first, *last, *pair;

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

static Scheme_Object *
length_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "length: wrong number of args");
  SCHEME_ASSERT (SCHEME_LISTP(argv[0]), "length: arg must be a list");
  return (scheme_make_integer (scheme_list_length (argv[0])));
}

static Scheme_Object *append (Scheme_Object *lst1, Scheme_Object *lst2);

static Scheme_Object *
append_prim (int argc, Scheme_Object *argv[])
{
  Scheme_Object *res;
  int i;

  res = scheme_null;
  for ( i=0 ; i<argc ; ++i )
    {
      res = append (res, argv[i]);
    }
  return (res);
}

static Scheme_Object *
append (Scheme_Object *lst1, Scheme_Object *lst2)
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

static Scheme_Object *
reverse_prim (int argc, Scheme_Object *argv[])
{
  Scheme_Object *lst, *cur, *last;

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

static Scheme_Object *
list_tail_prim (int argc, Scheme_Object *argv[])
{
  int i, k;
  Scheme_Object *lst;

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

static Scheme_Object *
list_ref_prim (int argc, Scheme_Object *argv[])
{
  int i, k;
  Scheme_Object *lst;

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
static Scheme_Object * \
name (int argc, Scheme_Object *argv[]) \
{ \
  Scheme_Object *list; \
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
static Scheme_Object * \
name (int argc, Scheme_Object *argv[]) \
{ \
  Scheme_Object *pair, *list; \
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

static Scheme_Object *
caar_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT((argc == 1), "caar: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "caar: arg must be a pair");
  return (SCHEME_CAR (SCHEME_CAR (argv[0])));
}

static Scheme_Object *
cadr_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT((argc == 1), "cadr: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cadr: arg must be a pair");
  return (SCHEME_CAR (SCHEME_CDR (argv[0])));
}

static Scheme_Object *
cdar_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT((argc == 1), "cdar: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cdar: arg must be a pair");
  return (SCHEME_CDR (SCHEME_CAR (argv[0])));
}

static Scheme_Object *
cddr_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT((argc == 1), "cddr: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cddr: arg must be a pair");
  return (SCHEME_CDR (SCHEME_CDR (argv[0])));
}

static Scheme_Object *
caaar_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT((argc == 1), "caaar: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "caaar: arg must be a pair");
  return (SCHEME_CAR (SCHEME_CAR (SCHEME_CAR (argv[0]))));
}

static Scheme_Object *
caadr_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT((argc == 1), "caadr: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "caadr: arg must be a pair");
  return (SCHEME_CAR (SCHEME_CAR (SCHEME_CDR (argv[0]))));
}

static Scheme_Object *
cadar_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT((argc == 1), "cadar: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cadar: arg must be a pair");
  return (SCHEME_CAR (SCHEME_CDR (SCHEME_CAR (argv[0]))));
}

static Scheme_Object *
cdaar_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT((argc == 1), "cdaar: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cdaar: arg must be a pair");
  return (SCHEME_CDR (SCHEME_CAR (SCHEME_CAR (argv[0]))));
}

static Scheme_Object *
cdadr_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT((argc == 1), "cdadr: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cdadr: arg must be a pair");
  return (SCHEME_CDR (SCHEME_CAR (SCHEME_CDR (argv[0]))));
}

static Scheme_Object *
cddar_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT((argc == 1), "cddar: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cddar: arg must be a pair");
  return (SCHEME_CDR (SCHEME_CDR (SCHEME_CDR (argv[0]))));
}

static Scheme_Object *
caddr_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT((argc == 1), "caddr: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "caddr: arg must be a pair");
  return (SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (argv[0]))));
}

static Scheme_Object *
cdddr_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT((argc == 1), "cdddr: wrong number of args");
  SCHEME_ASSERT(SCHEME_PAIRP(argv[0]), "cdddr: arg must be a pair");
  return (SCHEME_CDR (SCHEME_CDR (SCHEME_CDR (argv[0]))));
}

