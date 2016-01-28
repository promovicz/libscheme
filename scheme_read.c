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
#include <stdlib.h>
#include <ctype.h>

#define MAX_STRING_SIZE 1024
#define MAX_NUMBER_SIZE 64
#define MAX_SYMBOL_SIZE 255

/* local function prototypes */

static Scheme_Object *read_char (Scheme_Object *port);
static Scheme_Object *read_list (Scheme_Object *port);
static Scheme_Object *read_string (Scheme_Object *port);
static Scheme_Object *read_quote (Scheme_Object *port);
static Scheme_Object *read_vector (Scheme_Object *port);
static Scheme_Object *read_number (Scheme_Object *port);
static Scheme_Object *read_hex_number (Scheme_Object *port);
static Scheme_Object *read_binary_number (Scheme_Object *port);
static Scheme_Object *read_octal_number (Scheme_Object *port);
static Scheme_Object *read_symbol (Scheme_Object *port);
static Scheme_Object *read_character (Scheme_Object *port);
static Scheme_Object *read_quasiquote (Scheme_Object *port);
static Scheme_Object *read_unquote (Scheme_Object *port);
static Scheme_Object *read_unquote_splicing (Scheme_Object *port);
static void skip_whitespace_comments (Scheme_Object *port);
static int peek_char (Scheme_Object *port);
static int double_peek_char (Scheme_Object *port);
static int match_chars (Scheme_Object *port, char *str);

Scheme_Object *
scheme_read (Scheme_Object *port)
{
  int ch;

 start_over:
  ch = scheme_getc (port);
  while (isspace(ch))
    {
      ch = scheme_getc (port);
    }
  switch ( ch )
    {
    case EOF: return (scheme_eof);
    case ')': scheme_signal_error ("read: unexpected ')'");
    case '(': return (read_list (port));
    case '"': return (read_string (port));
    case '\'': return (read_quote (port));
    case '`': return (read_quasiquote (port));
    case ',':
      if (peek_char (port) == '@')
	{
	  ch = scheme_getc (port);
	  return (read_unquote_splicing (port));
	}
      else
	{
	  return (read_unquote (port));
	}
    case ';':
      while ((ch = scheme_getc (port)) != '\n')
	{
	  if (ch == EOF)
	    {
	      return (scheme_eof);
	    }
	}
      goto start_over;
    case '+':
    case '-':
      if (isdigit (peek_char (port)))
	{
	  scheme_ungetc (ch, port);
	  return (read_number (port));
	}
      else
	{
	  scheme_ungetc (ch, port);
	  return (read_symbol (port));
	}
    case '#':
      ch = scheme_getc (port);
      switch ( ch )
	{
	case '(': return (read_vector (port));
	case '\\': return (read_character (port));
	case 't': return (scheme_true);
	case 'f': return (scheme_false);
	case 'x': return (read_hex_number (port));
	case 'b': return (read_binary_number (port));
	case 'o': return (read_octal_number (port));
	case '|':
	  do
	    {
	      ch = scheme_getc (port);
	      if (ch == EOF)
		{
		  scheme_signal_error ("read: end of file in #| comment");
		}
	      if ((ch == '|') && (peek_char(port) == '#'))
		{
		  ch = scheme_getc (port);
		  goto start_over;
		}
	    }
	  while ( 1 );
	  break;
	default:
	  scheme_signal_error ("read: unexpected `#'");
	}
    default:
      if (isdigit (ch))
	{
	  scheme_ungetc (ch, port);
	  return (read_number (port));
	}
      else
	{
	  scheme_ungetc (ch, port);
	  return (read_symbol (port));
	}
    }
}

static Scheme_Object *
read_char (Scheme_Object *port)
{
  int ch;

  ch = scheme_getc (port);
  if (ch == EOF)
    {
      return (scheme_eof);
    }
  else
    {
      return (scheme_make_char (ch));
    }
}

/* "(" has already been read */
static Scheme_Object *
read_list (Scheme_Object *port)
{
  Scheme_Object *obj, *car, *cdr;
  int ch;

  skip_whitespace_comments (port);
  if (peek_char(port) == ')')
    {
      ch = scheme_getc (port);
      return (scheme_null);
    }
  car = scheme_read (port);
  skip_whitespace_comments (port);
  if (peek_char(port) == ')')
    {
      ch = scheme_getc (port);
      cdr = scheme_null;
    }
  else if ((peek_char(port) == '.') && isspace (double_peek_char(port)))
    {
      ch = scheme_getc (port);
      cdr = scheme_read (port);
      skip_whitespace_comments (port);
      if (peek_char(port) != ')')
	{
	  scheme_signal_error ("read: malformed list");
	}
      ch = scheme_getc (port);
    }
  else
    {
      cdr = read_list (port);
    }
  return (scheme_make_pair (car, cdr));
}

/* '"' has already been read */
static Scheme_Object *
read_string (Scheme_Object *port)
{
  char ch, buf[MAX_STRING_SIZE];
  int i;

  i = 0;
  while ((ch = scheme_getc (port)) != '"')
    {
      if (ch == '\\')
	{
	  ch = scheme_getc (port);
	}
      if (i > MAX_STRING_SIZE)
	{
	  scheme_signal_error ("read: string too long for reader");
	}
      buf[i++] = ch;
    }
  buf[i] = '\0';
  return (scheme_make_string (buf));
}

/* "'" has been read */
static Scheme_Object *
read_quote (Scheme_Object *port)
{
  Scheme_Object *obj;

  obj = scheme_read (port);
  return (scheme_make_pair (scheme_quote_symbol, 
			    scheme_make_pair (obj, scheme_null)));
}

/* "#(" has been read */
static Scheme_Object *
read_vector (Scheme_Object *port)
{
  Scheme_Object *obj, *vec;
  int len, i;
  
  obj = read_list (port);
  len = scheme_list_length (obj);
  vec = scheme_make_vector (len, NULL);
  for ( i=0 ; i<len ; ++i )
    {
      SCHEME_VEC_ELS(vec)[i] = SCHEME_CAR(obj);
      obj = SCHEME_CDR(obj);
    }
  return (vec);
}

/* nothing has been read */
static Scheme_Object  *
read_number (Scheme_Object *port)
{
  char buf[MAX_NUMBER_SIZE];
  int i, is_float, is_negative, ch;

  i = 0;
  is_negative = is_float = 0;
  ch = scheme_getc (port);
  if (ch == '+')
    {
      ch = scheme_getc (port);
    }
  else if (ch == '-')
    {
      is_negative = 1;
      ch = scheme_getc (port);
    }
  do
    {
      if (i > MAX_NUMBER_SIZE)
	{
	  scheme_signal_error ("read: number too long for reader");
	}
      if ((ch == '.') || (ch == 'e') || (ch == 'E'))
	{
	  is_float = 1;
	}
      buf[i++] = ch;
    }
  while (isdigit (ch = scheme_getc (port)) || (ch == '.') || (ch == 'e') || (ch == 'E'));
  scheme_ungetc (ch, port);
  buf[i] = '\0';
  if ( is_float )
    {
      double d;
      d = strtod (buf, NULL);
      if (is_negative)
	{
	  d = -d;
	}
      return (scheme_make_double (d));
    }
  else
    {
      int i;

      i = atoi (buf);
      if (is_negative)
	{
	  i = -i;
	} 
      return (scheme_make_integer (i));
    }
}

static Scheme_Object *
read_hex_number (Scheme_Object *port)
{
  int ch, i;

  i = 0;
  while ( 1 )
    {
      ch = scheme_getc (port);
      if (ch >= '0' && ch <= '9')
	{
	  i *= 16;
	  i += ch - '0';
	}
      else if ((ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))
	{
	  i *= 16;
	  if (ch >= 'a' && ch <= 'f')
	    i += ch - 'a' + 10;
	  else
	    i += ch - 'A' + 10;
	}
      else
	{
	  scheme_ungetc (ch, port);
	  return (scheme_make_integer (i));
	}
    }
}

static Scheme_Object *
read_binary_number (Scheme_Object *port)
{
  int ch, i;

  i = 0;
  while ( 1 )
    {
      ch = scheme_getc (port);
      if (ch == '0' || ch == '1')
	{
	  i *= 2;
	  i += ch - '0';
	}
      else
	{
	  scheme_ungetc (ch, port);
	  return (scheme_make_integer (i));
	}
    }
}

static Scheme_Object *
read_octal_number (Scheme_Object *port)
{
  int ch, i;

  i = 0;
  while ( 1 )
    {
      ch = scheme_getc (port);
      if (ch >= '0' && ch <= '7')
	{
	  i *= 8;
	  i += ch - '0';
	}
      else
	{
	  scheme_ungetc (ch, port);
	  return (scheme_make_integer (i));
	}
    }
}

/* nothing has been read */
static Scheme_Object *
read_symbol (Scheme_Object *port)
{
  char buf[MAX_SYMBOL_SIZE];
  int i, ch;

  i = 0;
  while ((!isspace (ch = scheme_getc (port)))
	 && (ch != '(')
	 && (ch != ')')
	 && (ch != '"')
	 && (ch != ';')
	 && (ch != EOF))
    {
      buf[i++] = ch;
    }
  if (ch != EOF)
    {
      scheme_ungetc (ch, port);
    }
  buf[i] = '\0';
  return (scheme_intern_symbol ((char *)&buf));
}

/* "#\" has been read */
static Scheme_Object *
read_character (Scheme_Object *port)
{
  int ch;

  ch = scheme_getc (port);
  switch (ch)
    {
    case 'n': /* maybe `newline' */
    case 'N':
      if ((peek_char(port) == 'e') || (peek_char(port) == 'E'))
	{
	  if (! match_chars (port, "ewline"))
	    {
	      scheme_signal_error ("read: bad character constant");
	    }
	  else
	    {
	      return (scheme_make_char ('\n'));
	    }
	}
      else
	{
	    return (scheme_make_char (ch));
	}
    case 's': /* maybe `space' */
    case 'S':
      if ((peek_char(port) == 'p') || (peek_char(port) == 'P'))
	{
	  if (! match_chars (port, "pace"))
	    {
	      scheme_signal_error ("read: bad character constant");
	    }
	  else
	    {
	      return (scheme_make_char (' '));
	    }
	}
      else
	{
	  return (scheme_make_char (ch));
	}
    case 'r': /* maybe `rubout' */
    case 'R':
      if ((peek_char(port) == 'u') || (peek_char(port) == 'U'))
	{
	  if (! match_chars (port, "ubout"))
	    {
	      scheme_signal_error ("read: bad character constant");
	    }
	  else
	    {
	      return (scheme_make_char (0x7f));
	    }
	}
      else if ((peek_char(port) == 'e') || (peek_char(port) == 'E'))
	{
	  if (! match_chars (port, "eturn"))
	    {
	      scheme_signal_error ("read: bad character constant");
	    }
	  else
	    {
	      return (scheme_make_char ('\r'));
	    }
	}
      else
	{
	  return (scheme_make_char (ch));
	}
    case 'p': /* maybe `page' */
    case 'P':
      if ((peek_char(port) == 'a') || (peek_char(port) == 'A'))
	{
	  if (! match_chars (port, "age"))
	    {
	      scheme_signal_error ("read: bad character constant");
	    }
	  else
	    {
	      return (scheme_make_char ('\f'));
	    }
	}
      else
	{
	  return (scheme_make_char (ch));
	}
    case 't': /* maybe `tab' */
    case 'T':
      if ((peek_char(port) == 'a') || (peek_char(port) == 'A'))
	{
	  if (! match_chars (port, "ab"))
	    {
	      scheme_signal_error ("read: bad character constant");
	    }
	  else
	    {
	      return (scheme_make_char ('\t'));
	    }
	}
      else
	{
	  return (scheme_make_char (ch));
	}
    case 'b': /* maybe `backspace' */
    case 'B':
      if ((peek_char(port) == 'a') || (peek_char(port) == 'A'))
	{
	  if (! match_chars (port, "ackspace"))
	    {
	      scheme_signal_error ("read: bad character constant", NULL);
	    }
	  else
	    {
	      return (scheme_make_char ('\b'));
	    }
	}
      else
	{
	  return (scheme_make_char (ch));
	}
    case 'l': /* maybe `linefeed' */
    case 'L':
      if ((peek_char(port) == 'i') || (peek_char(port) == 'I'))
	{
	  if (! match_chars (port, "inefeed"))
	    {
	      scheme_signal_error ("read: bad character constant");
	    }
	  else
	    {
	      return (scheme_make_char ('\n'));
	    }
	}
      else
	{
	  return (scheme_make_char (ch));
	}
    default:
      return (scheme_make_char (ch));
    }
}

/* "`" has been read */
static Scheme_Object *
read_quasiquote (Scheme_Object *port)
{
  Scheme_Object *quoted_obj, *ret;
  
  quoted_obj = scheme_read (port);
  ret = scheme_make_pair (scheme_quasiquote_symbol, 
			  scheme_make_pair (quoted_obj, scheme_null));
  return (ret);
}

/* "," has been read */
static Scheme_Object *
read_unquote (Scheme_Object *port)
{
  Scheme_Object *obj, *ret;

  obj = scheme_read (port);
  ret = scheme_make_pair (scheme_unquote_symbol, 
			  scheme_make_pair (obj, scheme_null));
  return (ret);
}

/* ",@" has been read */
static Scheme_Object *
read_unquote_splicing (Scheme_Object *port)
{
  Scheme_Object *obj, *ret;

  obj = scheme_read (port);
  ret = scheme_make_pair (scheme_unquote_splicing_symbol, 
			  scheme_make_pair (obj, scheme_null));
  return (ret);
}

/* utilities */

static void
skip_whitespace_comments (Scheme_Object *port)
{
  int ch;

 start_over:
  while (isspace(ch = scheme_getc (port)));
  if ( ch == ';' )
    {
      while ((ch = scheme_getc (port)) != '\n');
      goto start_over;
    }
  scheme_ungetc (ch, port);
}

static int
peek_char (Scheme_Object *port)
{
  int ch;

  ch = scheme_getc (port);
  scheme_ungetc (ch, port);
  return (ch);
}

static int 
double_peek_char (Scheme_Object *port)
{
  int ch1, ch2;

  ch1 = scheme_getc (port);
  ch2 = scheme_getc (port);
  scheme_ungetc (ch2, port);
  scheme_ungetc (ch1, port);
  return (ch2);
}

static int
match_chars (Scheme_Object *port, char *str)
{
  int i;
  int ch;

  i = 0;
  while (str[i])
    {
      ch = scheme_getc (port);
      if (tolower(ch) != tolower(str[i]))
	{
	  return (0);
	}
      i++;
    }
  return (1);
}
