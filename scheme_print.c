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

/* static function declarations */
static void print_to_port (Scheme_Value obj, Scheme_Value port, int escaped);
static int print (FILE *os, Scheme_Value obj, int escaped);
static int print_string (FILE *os, Scheme_Value string, int escaped);
static int print_pair (FILE *os, Scheme_Value pair, int escaped);
static int print_vector (FILE *os, Scheme_Value vec, int escaped);
static int print_char (FILE *os, Scheme_Value chobj, int escaped);

/* exported functions */

void
scheme_write (Scheme_Value obj, Scheme_Value port)
{
  print_to_port (obj, port, 1);
}

void
scheme_display (Scheme_Value obj, Scheme_Value port)
{
  print_to_port (obj, port, 0);
}

/* static functions */

static void
print_to_port (Scheme_Value obj, Scheme_Value port, int escaped)
{
  Scheme_Port *op;

  op = (Scheme_Port *) SCHEME_PTR_VAL (port);
  print (op->stream, obj, escaped);
}

static int
print (FILE *os, Scheme_Value obj, int escaped)
{
  Scheme_Value type;

  type = SCHEME_TYPE (obj);
  if (type==scheme_type_type || type==scheme_symbol_type)
    {
      fprintf (os, "%s", SCHEME_STR_VAL (obj));
    }
  else if (type==scheme_string_type)
    {
      print_string (os, obj, escaped);
    }
  else if (type==scheme_char_type)
    {
      print_char (os, obj, escaped);
    }
  else if (type==scheme_integer_type)
    {
      fprintf (os, "%d", SCHEME_INT_VAL (obj));
    }
  else if (type==scheme_double_type)
    {
      fprintf (os, "%f", SCHEME_DBL_VAL (obj));
    }
  else if (type==scheme_null_type)
    {
      fprintf (os, "()");
    }
  else if (type==scheme_pair_type)
    {
      print_pair (os, obj, escaped);
    }
  else if (type==scheme_vector_type)
    {
      print_vector (os, obj, escaped);
    }
  else if (type==scheme_true_type)
    {
      fprintf (os, "#t");
    }
  else if (type==scheme_false_type)
    {
      fprintf (os, "#f");
    }
  else
    {
      fprintf (os, "#%s", SCHEME_STR_VAL(SCHEME_TYPE(obj)));
    }
  return (0);
}

static int
print_string (FILE *os, Scheme_Value string, int escaped)
{
  char *str;

  str = SCHEME_STR_VAL (string);
  if ( escaped )
    {
      fputc('"', os);
    }
  while ( *str )
    {
      if (escaped && ((*str == '"') || (*str == '\\')))
	{
	  fputc('\\', os);
	}
      fputc(*str, os);
      str++;
    }
  if ( escaped )
    {
      fputc('"', os);
    }
  return (0);
}

static int
print_pair (FILE *os, Scheme_Value pair, int escaped)
{
  Scheme_Value cdr;

  fputc('(', os);
  print (os, SCHEME_CAR (pair), escaped);
  cdr = SCHEME_CDR (pair);
  while ((cdr != scheme_null) && (SCHEME_TYPE(cdr) == scheme_pair_type))
    {
      fputc(' ', os);
      print (os, SCHEME_CAR (cdr), escaped);
      cdr = SCHEME_CDR (cdr);
    }
  if (cdr != scheme_null)
    {
      fputs(" . ", os);
      print (os, cdr, escaped);
    }
  fputc(')', os);
  return (0);
}

static int
print_vector (FILE *os, Scheme_Value vec, int escaped)
{
  int i;

  fputs("#(", os);
  for ( i=0 ; i<SCHEME_VEC_SIZE(vec) ; ++i )
    {
      print (os, SCHEME_VEC_ELS(vec)[i], escaped);
      if (i<SCHEME_VEC_SIZE(vec)-1)
	{
	  fputc(' ', os);
	}
    }
  fputc(')', os);
  return (0);
}

static int
print_char (FILE *os, Scheme_Value charobj, int escaped)
{
  char ch;

  ch = SCHEME_CHAR_VAL (charobj);
  if (escaped)
    {
      switch ( ch )
	{
	case '\n':
	  fprintf (os, "#\\newline");
	  break;
	case '\t':
	  fprintf (os, "#\\tab");
	  break;
	case ' ':
	  fprintf (os, "#\\space");
	  break;
	case '\r':
	  fprintf (os, "#\\return");
	  break;
	case '\f':
	  fprintf (os, "#\\page");
	  break;
	case '\b':
	  fprintf (os, "#\\backspace");
	  break;
	default:
	  fprintf (os, "#\\%c", ch);
	  break;
	}
    }
  else
    {
      fputc(ch, os);
    }
  return (0);
}
