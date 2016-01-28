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

#define SCHEME_MAX_PRINT_SIZE 256000

/* locals */
static char print_buffer[SCHEME_MAX_PRINT_SIZE];

static void print_to_port (Scheme_Object *obj, Scheme_Object *port, int escaped);
static int print (char *str, int index, Scheme_Object *obj, int escaped);
static int print_string (char *str, int index, Scheme_Object *string, int escaped);
static int print_pair (char *str, int index, Scheme_Object *pair, int escaped);
static int print_vector (char *str, int index, Scheme_Object *vec, int escaped);
static int print_char (char *str, int index, Scheme_Object *chobj, int escaped);

void
scheme_debug_print (Scheme_Object *obj)
{
  scheme_write (obj, scheme_stdout_port);
  fflush (stdout);
}

void
scheme_write (Scheme_Object *obj, Scheme_Object *port)
{
  print_to_port (obj, port, 1);
}

void
scheme_display (Scheme_Object *obj, Scheme_Object *port)
{
  print_to_port (obj, port, 0);
}

char *
scheme_write_to_string (Scheme_Object *obj)
{
  int index = 0;

  index = print (print_buffer, index, obj, 1);
  print_buffer[index] = '\0';
  return (scheme_strdup (print_buffer));
}

char *
scheme_display_to_string (Scheme_Object *obj)
{
  int index = 0;

  index = print (print_buffer, index, obj, 0);
  print_buffer[index] = '\0';
  return (scheme_strdup (print_buffer));
}

void
scheme_write_string (char *str, Scheme_Object *port)
{
  Scheme_Output_Port *op;
  op = (Scheme_Output_Port *) SCHEME_PTR_VAL (port);
  (op->write_string_fun) (str, op);
}

static void 
print_to_port (Scheme_Object *obj, Scheme_Object *port, int escaped)
{
  Scheme_Output_Port *op;
  int index = 0;
  
  op = (Scheme_Output_Port *) SCHEME_PTR_VAL (port);
  index = print (print_buffer, index, obj, escaped);
  print_buffer[index] = '\0';

  op = (Scheme_Output_Port *) SCHEME_PTR_VAL (port);
  (op->write_string_fun) (print_buffer, op);
}

static int
print (char *str, int index, Scheme_Object *obj, int escaped)
{
  Scheme_Object *type;

  type = SCHEME_TYPE (obj);
  if (type==scheme_type_type || type==scheme_symbol_type)
    {
      sprintf ((str + index), "%s", SCHEME_STR_VAL (obj));
      index += strlen (SCHEME_STR_VAL (obj));
    }
  else if (type==scheme_string_type)
    {
      index = print_string (str, index, obj, escaped);
    }
  else if (type==scheme_char_type)
    {
      index = print_char (str, index, obj, escaped);
    }
  else if (type==scheme_integer_type)
    {
      sprintf ((str + index), "%d", SCHEME_INT_VAL (obj));
      index += strlen (str + index);
    }
  else if (type==scheme_double_type)
    {
      sprintf ((str + index), "%f", SCHEME_DBL_VAL (obj));
      index += strlen (str + index);
    }
  else if (type==scheme_null_type)
    {
      sprintf ((str + index), "()");
      index += 2;
    }
  else if (type==scheme_pair_type)
    {
      index = print_pair (str, index, obj, escaped);
    }
  else if (type==scheme_vector_type)
    {
      index = print_vector (str, index, obj, escaped);
    }
  else if (type==scheme_true_type)
    {
      sprintf ((str + index), "#t");
      index += 2;
    }
  else if (type==scheme_false_type)
    {
      sprintf ((str + index), "#f");
      index += 2;
    }
  else
    {
      sprintf ((str + index), "#%s", SCHEME_STR_VAL(SCHEME_TYPE(obj)));
      index += strlen (str + index);
    }
  return (index);
}

static int
print_string (char *buf, int index, Scheme_Object *string, int escaped)
{
  char *str;

  str = SCHEME_STR_VAL (string);
  if ( escaped )
    {
      buf[index++] = '"';
    }
  while ( *str )
    {
      if (escaped && ((*str == '"') || (*str == '\\')))
	{
	  buf[index++] = '\\';
	}
      buf[index++] = *str;
      *str++;
    }
  if ( escaped )
    {
      buf[index++] = '"';
    }
  return (index);
}

static int
print_pair (char *str, int index, Scheme_Object *pair, int escaped)
{
  Scheme_Object *cdr;
  int num_chars;

  str[index++] = '(';
  index = print (str, index, SCHEME_CAR (pair), escaped);
  cdr = SCHEME_CDR (pair);
  while ((cdr != scheme_null) && (SCHEME_TYPE(cdr) == scheme_pair_type))
    {
      str[index++] = ' ';
      index = print (str, index, SCHEME_CAR (cdr), escaped);
      cdr = SCHEME_CDR (cdr);
    }
  if (cdr != scheme_null)
    {
      str[index++] = ' ';
      str[index++] = '.';
      str[index++] = ' ';
      index = print (str, index, cdr, escaped);
    }
  str[index++] = ')';
  return (index);
}

static int
print_vector (char *str, int index, Scheme_Object *vec, int escaped)
{
  int i;

  str[index++] = '#';
  str[index++] = '(';
  for ( i=0 ; i<SCHEME_VEC_SIZE(vec) ; ++i )
    {
      index = print (str, index, SCHEME_VEC_ELS(vec)[i], escaped);
      if (i<SCHEME_VEC_SIZE(vec)-1)
	{
	  str[index++] = ' ';
	}
    }
  str[index++] = ')';
  return (index);
}

static int
print_char (char *str, int index, Scheme_Object *charobj, int escaped)
{
  char ch;
  int num_chars;

  ch = SCHEME_CHAR_VAL (charobj);
  if (escaped)
    {
      switch ( ch )
	{
	case '\n':
	  sprintf ((str + index), "#\\newline");
	  index += 9;
	  break;
	case '\t':
	  sprintf ((str + index), "#\\tab");
	  index += 5;
	  break;
	case ' ':
	  sprintf ((str + index), "#\\space");
	  index += 7;
	  break;
	case '\r':
	  sprintf ((str + index), "#\\return");
	  index += 8;
	  break;
	case '\f':
	  sprintf ((str + index), "#\\page");
	  index += 6;
	  break;
	case '\b':
	  sprintf ((str + index), "#\\backspace");
	  index += 11;
	  break;
	default:
	  sprintf ((str + index), "#\\%c", ch);
	  index += 3;
	  break;
	}
    }
  else
    {
      str[index++] = ch;
    }
  return (index);
}
