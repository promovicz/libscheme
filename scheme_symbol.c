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

#define HASH_TABLE_SIZE 1023
static Scheme_Hash_Table *symbol_table;

/* globals */
Scheme_Object *scheme_symbol_type;
Scheme_Object *scheme_quote_symbol;
Scheme_Object *scheme_quasiquote_symbol;
Scheme_Object *scheme_unquote_symbol;
Scheme_Object *scheme_unquote_splicing_symbol;

/* locals */
static Scheme_Object *symbol_p_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *string_to_symbol_prim (int argc, Scheme_Object *argv[]);
static Scheme_Object *symbol_to_string_prim (int argc, Scheme_Object *argv[]);
static char *downcase (char *str);

void
scheme_init_symbol (Scheme_Env *env)
{
  scheme_symbol_type = scheme_make_type ("<symbol>");
  scheme_add_global ("<symbol>", scheme_symbol_type, env);
  symbol_table = scheme_hash_table (HASH_TABLE_SIZE);
  scheme_quote_symbol = scheme_intern_symbol ("quote");
  scheme_quasiquote_symbol = scheme_intern_symbol ("quasiquote");
  scheme_unquote_symbol = scheme_intern_symbol ("unquote");
  scheme_unquote_splicing_symbol = scheme_intern_symbol ("unquote-splicing");
  scheme_add_global ("symbol?", scheme_make_prim (symbol_p_prim), env);
  scheme_add_global ("string->symbol", scheme_make_prim (string_to_symbol_prim), env);
  scheme_add_global ("symbol->string", scheme_make_prim (symbol_to_string_prim), env);
}

Scheme_Object *
scheme_make_symbol (char *name)
{
  Scheme_Object *sym;

  sym = scheme_alloc_object ();
  SCHEME_TYPE (sym) = scheme_symbol_type;
  SCHEME_STR_VAL (sym) = scheme_strdup (name);
  return (sym);
}

Scheme_Object *
scheme_intern_symbol (char *name)
{
  Scheme_Object *sym;

  name = downcase (name);
  sym = (Scheme_Object *)scheme_lookup_in_table (symbol_table, name);
  if (sym)
    {
      return (sym);
    }
  else
    {
      sym = scheme_make_symbol (name);
      scheme_add_to_table (symbol_table, name, sym);
      return (sym);
    }
}

/* locals */

static Scheme_Object *
symbol_p_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "symbol?: wrong number of args");
  return (SCHEME_SYMBOLP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Object *
string_to_symbol_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "string->symbol: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "string->symbol: arg must be string");
  return (scheme_make_symbol (SCHEME_STR_VAL(argv[0])));
}

static Scheme_Object *
symbol_to_string_prim (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "symbol->string: wrong number of args");
  SCHEME_ASSERT (SCHEME_SYMBOLP(argv[0]), "symbol->string: arg must be symbol");
  return (scheme_make_string (SCHEME_STR_VAL(argv[0])));
}

static char *
downcase (char *str)
{
  char *new;
  int i;

  i = 0;
  new = scheme_strdup (str);
  while (new[i])
    {
      new[i] = tolower (new[i]);
      i++;
    }
  return (new);
}
