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
#define MALLOC malloc
#else
#define MALLOC GC_malloc
#endif

Scheme_Object *
scheme_alloc_object (void)
{
  Scheme_Object *object;

  object = (Scheme_Object *) scheme_malloc (sizeof (Scheme_Object));
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
  
  space = MALLOC (num*size);
#ifdef NO_GC
  memset (space, 0, (num*size));
#endif
  SCHEME_ASSERT ((space != 0), "memory allocation failure");
  return (space);
}

char *
scheme_strdup (char *str)
{
  char *new;

  new = scheme_malloc ((strlen (str) + 1) * sizeof (char));
  strcpy (new, str);
  return (new);
}
