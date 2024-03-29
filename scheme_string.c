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
#include <ctype.h>

/* globals */
Scheme_Value scheme_string_type;

/* locals */
static Scheme_Value string_p (int argc, Scheme_Value argv[]);
static Scheme_Value make_string (int argc, Scheme_Value argv[]);
static Scheme_Value string (int argc, Scheme_Value argv[]);
static Scheme_Value string_length (int argc, Scheme_Value argv[]);
static Scheme_Value string_ref (int argc, Scheme_Value argv[]);
static Scheme_Value string_set (int argc, Scheme_Value argv[]);
static Scheme_Value string_eq (int argc, Scheme_Value argv[]);
static Scheme_Value string_ci_eq (int argc, Scheme_Value argv[]);
static Scheme_Value string_lt (int argc, Scheme_Value argv[]);
static Scheme_Value string_gt (int argc, Scheme_Value argv[]);
static Scheme_Value string_lt_eq (int argc, Scheme_Value argv[]);
static Scheme_Value string_gt_eq (int argc, Scheme_Value argv[]);
static Scheme_Value string_ci_lt (int argc, Scheme_Value argv[]);
static Scheme_Value string_ci_gt (int argc, Scheme_Value argv[]);
static Scheme_Value string_ci_lt_eq (int argc, Scheme_Value argv[]);
static Scheme_Value string_ci_gt_eq (int argc, Scheme_Value argv[]);
static Scheme_Value substring (int argc, Scheme_Value argv[]);
static Scheme_Value string_append (int argc, Scheme_Value argv[]);
static Scheme_Value string_to_list (int argc, Scheme_Value argv[]);
static Scheme_Value list_to_string (int argc, Scheme_Value argv[]);
static Scheme_Value string_copy (int argc, Scheme_Value argv[]);
static Scheme_Value string_fill (int argc, Scheme_Value argv[]);

static int strcmp_ci (char *str1, char *str2);

void
scheme_init_string (Scheme_Env *env)
{
  scheme_string_type = scheme_make_type ("<string>");
  scheme_add_global ("<string>", scheme_string_type, env);
  scheme_add_prim ("string?", string_p, env);
  scheme_add_prim ("make-string", make_string, env);
  scheme_add_prim ("string", string, env);
  scheme_add_prim ("string-length", string_length, env);
  scheme_add_prim ("string-ref", string_ref, env);
  scheme_add_prim ("string-set!", string_set, env);
  scheme_add_prim ("string=?", string_eq, env);
  scheme_add_prim ("string-ci=?", string_ci_eq, env);
  scheme_add_prim ("string<?", string_lt, env);
  scheme_add_prim ("string>?", string_gt, env);
  scheme_add_prim ("string<=?", string_lt_eq, env);
  scheme_add_prim ("string>=?", string_gt_eq, env);
  scheme_add_prim ("string-ci<?", string_ci_lt, env);
  scheme_add_prim ("string-ci>?", string_ci_gt, env);
  scheme_add_prim ("string-ci<=?", string_ci_lt_eq, env);
  scheme_add_prim ("string-ci>=?", string_ci_gt_eq, env);
  scheme_add_prim ("substring", substring, env);
  scheme_add_prim ("string-append", string_append, env);
  scheme_add_prim ("string->list", string_to_list, env);
  scheme_add_prim ("list->string", list_to_string, env);
  scheme_add_prim ("string-copy", string_copy, env);
  scheme_add_prim ("string-fill!", string_fill, env);
}

Scheme_Value
scheme_make_string (const char *chars)
{
  Scheme_Value str;
  size_t len = strlen(chars);
  char *new;

  str = scheme_alloc_object (scheme_string_type, len + 1);
  new = SCHEME_PTR_VAL(str);
  if(len > 0) {
    memcpy(new, chars, len);
  }
  new[len] = 0;
  SCHEME_STR_VAL(str) = new;
  return (str);
}

Scheme_Value
scheme_alloc_string (int size, char fill)
{
  int i;
  size_t nbytes = sizeof(Scheme_Object) + (size + 1) * sizeof(char);
  Scheme_Value str;
  char *val;

  str = scheme_malloc (nbytes);
  val = (char*)(&str[1]);

  SCHEME_TYPE (str) = scheme_string_type;
  SCHEME_STR_VAL (str) = val;
  for ( i=0 ; i<size ; ++i )
    {
      SCHEME_STR_VAL(str)[i] = fill;
    }
  SCHEME_STR_VAL(str)[i] = '\0';
  return (str);
}

/* locals */

static Scheme_Value
string_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "string?: wrong number of args");
  return (SCHEME_STRINGP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Value
make_string (int argc, Scheme_Value argv[])
{
  int len;
  char fill;
  Scheme_Value str;

  SCHEME_ASSERT ((argc == 1) || (argc == 2), "make-string: wrong number of args");
  SCHEME_ASSERT (SCHEME_INTP (argv[0]), "make-string: first arg must be integer");
  len = SCHEME_INT_VAL (argv[0]);
  if (argc == 2)
    {
      SCHEME_ASSERT (SCHEME_CHARP(argv[1]), "make-string: second arg must be character");
      fill = SCHEME_CHAR_VAL (argv[1]);
    }
  else
    {
      fill = ' ';
    }
  str = scheme_alloc_string (len, fill);
  return (str);
}

static Scheme_Value
string (int argc, Scheme_Value argv[])
{
  Scheme_Value str;
  int i;

  str = scheme_alloc_string (argc, 0);
  for ( i=0 ; i<argc ; ++i )
    {
      SCHEME_ASSERT (SCHEME_CHARP (argv[0]), "string: args must all be characters");
      SCHEME_STR_VAL(str)[i] = SCHEME_CHAR_VAL(argv[i]);
    }
  return (str);
}

static Scheme_Value
string_length (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "string-length: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP (argv[0]), "string-length: arg must be a string");
  return (scheme_make_integer (strlen (SCHEME_STR_VAL (argv[0]))));
}

static Scheme_Value
string_ref (int argc, Scheme_Value argv[])
{
  int i, len;
  char *str;

  SCHEME_ASSERT ((argc == 2), "string-ref: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "string-ref: first arg must be a string");
  SCHEME_ASSERT (SCHEME_INTP(argv[1]), "string-ref: second arg must be an integer");
  str = SCHEME_STR_VAL(argv[0]);
  len = strlen (str);
  i = SCHEME_INT_VAL(argv[1]);
  if ((i < 0) || (i >= len))
    {
      scheme_signal_error ("string-ref: index out of range: %d", i);
    }
  return (scheme_make_char (str[i]));
}

static Scheme_Value
string_set (int argc, Scheme_Value argv[])
{
  int i, len;
  char *str;

  SCHEME_ASSERT ((argc == 3), "string-set!: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "string-set!: first arg must be a string");
  SCHEME_ASSERT (SCHEME_INTP(argv[1]), "string-set!: second arg must be an integer");
  SCHEME_ASSERT (SCHEME_CHARP(argv[2]), "string-set!: third arg must be a character");
  str = SCHEME_STR_VAL(argv[0]);
  len = strlen (str);
  i = SCHEME_INT_VAL(argv[1]);
  if ((i < 0) || (i >= len))
    {
      scheme_signal_error ("string-ref: index out of range: %d", i);
    }
  str[i] = SCHEME_CHAR_VAL (argv[2]);
  return (argv[0]);
}

/* comparisons */

#define GEN_STRING_COMP(name, scheme_name, comp, op) \
static Scheme_Value  \
name (int argc, Scheme_Value argv[]) \
{ \
  SCHEME_ASSERT ((argc == 2), #scheme_name ": wrong number of args"); \
  SCHEME_ASSERT ((SCHEME_STRINGP(argv[0]) && SCHEME_STRINGP(argv[1])), \
                 #scheme_name ": both args must be strings"); \
  return ((comp (SCHEME_STR_VAL(argv[0]), SCHEME_STR_VAL(argv[1])) op 0) \
	  ? scheme_true : scheme_false); \
}

GEN_STRING_COMP(string_eq, "string=?", strcmp, ==)
GEN_STRING_COMP(string_ci_eq, "string-ci=?", strcmp_ci, ==)
GEN_STRING_COMP(string_lt, "string<?", strcmp, <)
GEN_STRING_COMP(string_gt, "string>?", strcmp, >)
GEN_STRING_COMP(string_lt_eq, "string<=?", strcmp, <=)
GEN_STRING_COMP(string_gt_eq, "string>=?", strcmp, >=)
GEN_STRING_COMP(string_ci_lt, "string-ci<?", strcmp_ci, <)
GEN_STRING_COMP(string_ci_gt, "string-ci>?", strcmp_ci, >)
GEN_STRING_COMP(string_ci_lt_eq, "string-ci<=?", strcmp_ci, <=)
GEN_STRING_COMP(string_ci_gt_eq, "string-ci>=?", strcmp_ci, >=)

static Scheme_Value
substring (int argc, Scheme_Value argv[])
{
  int len, start, finish, i;
  char *chars;
  Scheme_Value str;

  SCHEME_ASSERT ((argc == 3), "substring: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "substring: first arg must be a string");
  SCHEME_ASSERT (SCHEME_INTP(argv[1]) && SCHEME_INTP(argv[2]),
		 "substring: second and third args must be integers");
  chars = SCHEME_STR_VAL (argv[0]);
  len = strlen (chars);
  start = SCHEME_INT_VAL (argv[1]);
  finish = SCHEME_INT_VAL (argv[2]);
  SCHEME_ASSERT ((start >= 0 && start <= len), "substring: first index out of bounds");
  SCHEME_ASSERT ((finish >= start && finish <= len), "substring: second index out of bounds");
  str = scheme_alloc_string (finish-start, 0);
  for ( i=0 ; i<finish-start ; ++i )
    {
      SCHEME_STR_VAL(str)[i] = chars[i+start];
    }
  return (str);
}

static Scheme_Value append_2 (Scheme_Value str1, Scheme_Value str2);

static Scheme_Value
string_append (int argc, Scheme_Value argv[])
{
  Scheme_Value new;
  int i;

  new = scheme_alloc_string (0, 0);
  for ( i=0 ; i<argc ; ++i )
    {
      new = append_2 (new, argv[i]);
    }
  return (new);
}

static Scheme_Value
append_2 (Scheme_Value str1, Scheme_Value str2)
{
  int len1, len2, i;
  char *chars1, *chars2;
  Scheme_Value new;

  SCHEME_ASSERT (SCHEME_STRINGP(str1) && SCHEME_STRINGP(str2),
		 "string-append: arguments must be strings");
  chars1 = SCHEME_STR_VAL (str1);
  chars2 = SCHEME_STR_VAL (str2);
  len1 = strlen (chars1);
  len2 = strlen (chars2);
  new = scheme_alloc_string (len1+len2, 0);
  for ( i=0 ; i<len1 ; ++i )
    {
      SCHEME_STR_VAL(new)[i] = chars1[i];
    }
  for ( i=0 ; i<len2 ; ++i )
    {
      SCHEME_STR_VAL(new)[i+len1] = chars2[i];
    }
  SCHEME_STR_VAL(new)[len1+len2] = '\0';
  return (new);
}


static Scheme_Value
string_to_list (int argc, Scheme_Value argv[])
{
  int len, i;
  char *chars;
  Scheme_Value first, last, pair;

  SCHEME_ASSERT (argc == 1, "string->list: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "string->list: arg must be a string");
  chars = SCHEME_STR_VAL(argv[0]);
  len = strlen (chars);
  first = last = scheme_null;
  for ( i=0 ; i<len ; ++i )
    {
      pair = scheme_make_pair (scheme_make_char (chars[i]), scheme_null);
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
list_to_string (int argc, Scheme_Value argv[])
{
  int len, i;
  Scheme_Value list, str, ch;

  SCHEME_ASSERT ((argc == 1), "list->string: wrong number of args");
  SCHEME_ASSERT (SCHEME_LISTP(argv[0]), "list->string: arg must be a list");
  list = argv[0];
  len = scheme_list_length (list);
  str = scheme_alloc_string (len, 0);
  i = 0;
  while (! SCHEME_NULLP (list))
    {
      ch = SCHEME_CAR (list);
      SCHEME_ASSERT (SCHEME_CHARP(ch), "list->string: all list elements must be characters");
      SCHEME_STR_VAL(str)[i] = SCHEME_CHAR_VAL(ch);
      i++;
      list = SCHEME_CDR (list);
    }
  return (str);
}

static Scheme_Value
string_copy (int argc, Scheme_Value argv[])
{
  Scheme_Value new;

  SCHEME_ASSERT ((argc == 1), "string-copy: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP (argv[0]), "string-copy: arg must be a string");
  new = scheme_make_string (SCHEME_STR_VAL (argv[0]));
  return (new);
}

static Scheme_Value
string_fill (int argc, Scheme_Value argv[])
{
  int len, i;
  char *chars, ch;

  SCHEME_ASSERT ((argc == 2), "string-fill!: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP (argv[0]), "string-fill!: first arg must be a string");
  SCHEME_ASSERT (SCHEME_CHARP (argv[1]), "string-fill!: second arg must be a character");
  chars = SCHEME_STR_VAL (argv[0]);
  ch = SCHEME_CHAR_VAL (argv[1]);
  len = strlen (chars);
  for ( i=0 ; i<len ; ++i )
    {
      chars[i] = ch;
    }
  return (argv[0]);
}

static int
strcmp_ci (char *str1, char *str2)
{
  while (*str1 && *str2)
    {
      if (toupper(*str1) == toupper(*str2))
	{
	  str1++;
	  str2++;
	  continue;
	}
      else if (toupper(*str1) < toupper(*str2))
	{
	  return -1;
	}
      else
	{
	  return 1;
	}
    }
  if (*str1)
    {
      return 1;
    }
  else if (*str2)
    {
      return -1;
    }
  else
    {
      return 0;
    }
}
