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
#include <ctype.h>

#define CHAR_CACHE_SIZE 256

/* globals */
Scheme_Value scheme_char_type;

/* internal variables */
static Scheme_Value char_cache[CHAR_CACHE_SIZE];

/* primitive declarations */
static Scheme_Value char_p (int argc, Scheme_Value argv[]);
static Scheme_Value char_eq (int argc, Scheme_Value argv[]);
static Scheme_Value char_lt (int argc, Scheme_Value argv[]);
static Scheme_Value char_gt (int argc, Scheme_Value argv[]);
static Scheme_Value char_lt_eq (int argc, Scheme_Value argv[]);
static Scheme_Value char_gt_eq (int argc, Scheme_Value argv[]);
static Scheme_Value char_eq_ci (int argc, Scheme_Value argv[]);
static Scheme_Value char_lt_ci (int argc, Scheme_Value argv[]);
static Scheme_Value char_gt_ci (int argc, Scheme_Value argv[]);
static Scheme_Value char_lt_eq_ci (int argc, Scheme_Value argv[]);
static Scheme_Value char_gt_eq_ci (int argc, Scheme_Value argv[]);
static Scheme_Value char_alphabetic (int argc, Scheme_Value argv[]);
static Scheme_Value char_numeric (int argc, Scheme_Value argv[]);
static Scheme_Value char_whitespace (int argc, Scheme_Value argv[]);
static Scheme_Value char_upper_case (int argc, Scheme_Value argv[]);
static Scheme_Value char_lower_case (int argc, Scheme_Value argv[]);
static Scheme_Value char_to_integer (int argc, Scheme_Value argv[]);
static Scheme_Value integer_to_char (int argc, Scheme_Value argv[]);
static Scheme_Value char_upcase (int argc, Scheme_Value argv[]);
static Scheme_Value char_downcase (int argc, Scheme_Value argv[]);

/* exported functions */

void
scheme_init_char (Scheme_Env *env)
{
  scheme_char_type = scheme_make_type ("<char>");
  scheme_add_global ("<char>", scheme_char_type, env);
  scheme_add_global ("char?", scheme_make_prim (char_p), env);
  scheme_add_global ("char=?", scheme_make_prim (char_eq), env);
  scheme_add_global ("char<?", scheme_make_prim (char_lt), env);
  scheme_add_global ("char>?", scheme_make_prim (char_gt), env);
  scheme_add_global ("char<=?", scheme_make_prim (char_lt_eq), env);
  scheme_add_global ("char>=?", scheme_make_prim (char_gt_eq), env);
  scheme_add_global ("char-ci=?", scheme_make_prim (char_eq_ci), env);
  scheme_add_global ("char-ci<?", scheme_make_prim (char_lt_ci), env);
  scheme_add_global ("char-ci>?", scheme_make_prim (char_gt_ci), env);
  scheme_add_global ("char-ci<=?", scheme_make_prim (char_lt_eq_ci), env);
  scheme_add_global ("char-ci>=?", scheme_make_prim (char_gt_eq_ci), env);
  scheme_add_global ("char-alphabetic?", scheme_make_prim (char_alphabetic), env);
  scheme_add_global ("char-numeric?", scheme_make_prim (char_numeric), env);
  scheme_add_global ("char-whitespace?", scheme_make_prim (char_whitespace), env);
  scheme_add_global ("char-upper-case?", scheme_make_prim (char_upper_case), env);
  scheme_add_global ("char-lower-case?", scheme_make_prim (char_lower_case), env);
  scheme_add_global ("char->integer", scheme_make_prim (char_to_integer), env);
  scheme_add_global ("integer->char", scheme_make_prim (integer_to_char), env);
  scheme_add_global ("char-upcase", scheme_make_prim (char_upcase), env);
  scheme_add_global ("char-downcase", scheme_make_prim (char_downcase), env);
}

Scheme_Value
scheme_make_char (char ch)
{
  unsigned idx = (unsigned)ch;
  Scheme_Value sc;

  /* check if we have an object in the cache */
  if(idx < CHAR_CACHE_SIZE) {
    sc = char_cache[idx];
    if(sc != NULL) {
      return sc;
    }
  }

  /* allocate and initialize the object */
  sc = scheme_alloc_object (scheme_char_type, 0);
  SCHEME_CHAR_VAL (sc) = ch;

  /* put the object in the cache */
  if(idx < CHAR_CACHE_SIZE) {
    char_cache[idx] = sc;
  }

  /* done */
  return (sc);
}

/* primitive functions */

static Scheme_Value
char_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 1, "char?: wrong number of args");
  return (SCHEME_CHARP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Value
char_eq (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 2, "char=?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP(argv[0]) && SCHEME_CHARP(argv[1]),
		 "char=?: both args must be characters");
  return ((SCHEME_CHAR_VAL(argv[0]) == SCHEME_CHAR_VAL(argv[1]))
	  ? scheme_true : scheme_false);
}

static Scheme_Value
char_lt (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 2, "char<?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP(argv[0]) && SCHEME_CHARP(argv[1]),
		 "char<?: both args must be characters");
  return ((SCHEME_CHAR_VAL(argv[0]) < SCHEME_CHAR_VAL(argv[1]))
	  ? scheme_true : scheme_false);
}

static Scheme_Value
char_gt (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 2, "char>?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP(argv[0]) && SCHEME_CHARP(argv[1]),
		 "char>?: both args must be characters");
  return ((SCHEME_CHAR_VAL(argv[0]) > SCHEME_CHAR_VAL(argv[1]))
	  ? scheme_true : scheme_false);
}

static Scheme_Value
char_lt_eq (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 2, "char<=?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP(argv[0]) && SCHEME_CHARP(argv[1]),
		 "char<=?: both args must be characters");
  return ((SCHEME_CHAR_VAL(argv[0]) <= SCHEME_CHAR_VAL(argv[1]))
	  ? scheme_true : scheme_false);
}

static Scheme_Value
char_gt_eq (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 2, "char>=?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP(argv[0]) && SCHEME_CHARP(argv[1]),
		 "char>=?: both args must be characters");
  return ((SCHEME_CHAR_VAL(argv[0]) >= SCHEME_CHAR_VAL(argv[1]))
	  ? scheme_true : scheme_false);
}

static Scheme_Value
char_eq_ci (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 2, "char-ci=?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP(argv[0]) && SCHEME_CHARP(argv[1]),
		 "char-ci=?: both args must be characters");
  return (toupper(SCHEME_CHAR_VAL(argv[0])) == toupper(SCHEME_CHAR_VAL(argv[1]))
	  ? scheme_true : scheme_false);
}

static Scheme_Value
char_lt_ci (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 2, "char-ci<?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP(argv[0]) && SCHEME_CHARP(argv[1]),
		 "char-ci<?: both args must be characters");
  return (toupper(SCHEME_CHAR_VAL(argv[0])) < toupper(SCHEME_CHAR_VAL(argv[1]))
	  ? scheme_true : scheme_false);
}

static Scheme_Value
char_gt_ci (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 2, "char-ci>?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP(argv[0]) && SCHEME_CHARP(argv[1]),
		 "char-ci>?: both args must be characters");
  return (toupper(SCHEME_CHAR_VAL(argv[0])) > toupper(SCHEME_CHAR_VAL(argv[1]))
	  ? scheme_true : scheme_false);
}

static Scheme_Value
char_lt_eq_ci (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 2, "char-ci<=?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP(argv[0]) && SCHEME_CHARP(argv[1]),
		 "char-ci<=?: both args must be characters");
  return (toupper(SCHEME_CHAR_VAL(argv[0])) <= toupper(SCHEME_CHAR_VAL(argv[1]))
	  ? scheme_true : scheme_false);
}

static Scheme_Value
char_gt_eq_ci (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 2, "char-ci>=?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP(argv[0]) && SCHEME_CHARP(argv[1]),
		 "char-ci>=?: both args must be characters");
  return (toupper(SCHEME_CHAR_VAL(argv[0])) >= toupper(SCHEME_CHAR_VAL(argv[1]))
	  ? scheme_true : scheme_false);
}

static Scheme_Value
char_alphabetic (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 1, "char-alphabetic?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP (argv[0]), "char-alphabetic?: arg must be a character");
  return (isalpha (SCHEME_CHAR_VAL (argv[0])) ? scheme_true : scheme_false);
}

static Scheme_Value
char_numeric (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 1, "char-numeric?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP (argv[0]), "char-numeric?: arg must be a character");
  return (isdigit (SCHEME_CHAR_VAL (argv[0])) ? scheme_true : scheme_false);
}

static Scheme_Value
char_whitespace (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 1, "char-whitespace?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP (argv[0]), "char-whitespace?: arg must be a character");
  return (isspace (SCHEME_CHAR_VAL (argv[0])) ? scheme_true : scheme_false);
}

static Scheme_Value
char_upper_case (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 1, "char-upper-case?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP (argv[0]), "char-upper-case?: arg must be a character");
  return (isupper (SCHEME_CHAR_VAL (argv[0])) ? scheme_true : scheme_false);
}

static Scheme_Value
char_lower_case (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 1, "char-lower-case?: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP (argv[0]), "char-lower-case?: arg must be a character");
  return (islower (SCHEME_CHAR_VAL (argv[0])) ? scheme_true : scheme_false);
}

static Scheme_Value
char_to_integer (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 1, "char->integer: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP (argv[0]), "char->integer: arg must be a character");
  return (scheme_make_integer (SCHEME_CHAR_VAL (argv[0])));
}

static Scheme_Value
integer_to_char (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 1, "integer->char: wrong number of args");
  SCHEME_ASSERT (SCHEME_INTP (argv[0]), "integer->char: arg must be an integer");
  return (scheme_make_char (SCHEME_INT_VAL (argv[0])));
}

static Scheme_Value
char_upcase (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 1, "char-upcase: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP (argv[0]), "char-upcase: arg must be a character");
  return (scheme_make_char (toupper (SCHEME_CHAR_VAL (argv[0]))));
}

static Scheme_Value
char_downcase (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT (argc == 1, "char-downcase: wrong number of args");
  SCHEME_ASSERT (SCHEME_CHARP (argv[0]), "char-downcase: arg must be a character");
  return (scheme_make_char (tolower (SCHEME_CHAR_VAL (argv[0]))));
}
