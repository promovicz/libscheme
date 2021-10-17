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

#ifdef NO_GC
#include <stdlib.h>
#define MALLOC malloc
#define CALLOC calloc
#else
#include <gc.h>
#define MALLOC      GC_malloc
#define CALLOC(n,s) GC_malloc(n*s)
#endif

Scheme_Value
scheme_alloc_object (Scheme_Value type, size_t nbytes)
{
  Scheme_Value object;
  size_t total = sizeof(Scheme_Object) + nbytes;

  object = (Scheme_Value) scheme_malloc (total);
  SCHEME_TYPE(object) = type;
  if(nbytes > 0) {
    SCHEME_PTR_VAL(object) = (void*)&object[1];
  }
  return (object);
}

void *
scheme_malloc (size_t size)
{
  void *space;

  space = MALLOC (size);
  SCHEME_ASSERT ((space != 0), "memory allocation failure");
  return (space);
}

void *
scheme_calloc (size_t num, size_t size)
{
  void *space;

  space = CALLOC (num, size);
  SCHEME_ASSERT ((space != 0), "memory allocation failure");
  return (space);
}

char *
scheme_strdup (char *str)
{
  char *new;
  size_t len = strlen(str);
  size_t space = len + 1;

  new = scheme_malloc (space);
  strncpy (new, str, space);
  new[len] = 0;
  return (new);
}
