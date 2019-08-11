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
Scheme_Object *scheme_pointer_type;

/* locals */
static Scheme_Object *pointer_p (int argc, Scheme_Object *argv[]);
static Scheme_Object *pointer_eq (int argc, Scheme_Object *argv[]);

void
scheme_init_pointer (Scheme_Env *env)
{
  scheme_pointer_type = scheme_make_type ("<pointer>");
  scheme_add_global ("<pointer>", scheme_pointer_type, env);
  scheme_add_global ("pointer?", scheme_make_prim (pointer_p), env);
  scheme_add_global ("pointer=?", scheme_make_prim (pointer_eq), env);
}

Scheme_Object *
scheme_make_pointer (void *pointer)
{
  Scheme_Object *sc;

  sc = scheme_alloc_object (scheme_pointer_type, 0);
  SCHEME_PTR_VAL (sc) = pointer;
  return (sc);
}

/* locals */

static Scheme_Object *
pointer_p (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT (argc == 1, "pointer?: wrong number of args");
  return (SCHEME_POINTERP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Object *
pointer_eq (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT (argc == 2, "pointer=?: wrong number of args");
  SCHEME_ASSERT (SCHEME_POINTERP(argv[0]) && SCHEME_POINTERP(argv[1]),
		 "pointer=?: both args must be pointers");
  return ((SCHEME_PTR_VAL(argv[0]) == SCHEME_PTR_VAL(argv[1]))
	  ? scheme_true : scheme_false);
}

