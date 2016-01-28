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

#define GEN_BIN_COMP_PROT(name) \
static int name (Scheme_Object *n1, Scheme_Object *n2)

#define GEN_BIN_COMP(name, scheme_name, op) \
static int \
name (Scheme_Object *n1, Scheme_Object *n2) \
{ \
  SCHEME_ASSERT (SCHEME_NUMBERP(n1) && SCHEME_NUMBERP(n2), \
                 #scheme_name ": args must be numbers"); \
  if (SCHEME_INTP(n1)) \
    { \
      if (SCHEME_INTP(n2)) \
        return (SCHEME_INT_VAL(n1) op SCHEME_INT_VAL(n2)); \
      else \
        return (SCHEME_INT_VAL(n1) op SCHEME_DBL_VAL(n2)); \
    } \
  else \
    { \
      if (SCHEME_INTP(n2)) \
        return (SCHEME_DBL_VAL(n1) op SCHEME_INT_VAL(n2)); \
      else \
        return (SCHEME_DBL_VAL(n1) op SCHEME_DBL_VAL(n2)); \
    } \
}

#define GEN_NARY_COMP(name, scheme_name, bin_name) \
static Scheme_Object * \
name (int argc, Scheme_Object *argv[]) \
{ \
  int i; \
  SCHEME_ASSERT ((argc > 1), #scheme_name ": wrong number of args"); \
  for ( i=0 ; i<(argc-1) ; ++i ) \
    { \
      if (! bin_name(argv[i], argv[i+1])) \
        { \
          return (scheme_false); \
        } \
    } \
  return (scheme_true); \
}

#define GEN_BIN_PROT(name) \
static Scheme_Object *name (Scheme_Object *n1, Scheme_Object *n2)

#define GEN_BIN_OP(name, scheme_name, op) \
static Scheme_Object * \
name (Scheme_Object *n1, Scheme_Object *n2) \
{ \
  SCHEME_ASSERT (SCHEME_NUMBERP(n1) && SCHEME_NUMBERP(n2), \
                 #scheme_name ": args must be numbers"); \
  if (SCHEME_INTP(n1)) \
    { \
      if (SCHEME_INTP(n2)) \
	return (scheme_make_integer (SCHEME_INT_VAL(n1) op SCHEME_INT_VAL(n2))); \
      else \
        return (scheme_make_double (SCHEME_INT_VAL(n1) op SCHEME_DBL_VAL(n2))); \
    } \
  else \
    { \
      if (SCHEME_INTP(n2)) \
        return (scheme_make_double (SCHEME_DBL_VAL(n1) op SCHEME_INT_VAL(n2))); \
      else \
        return (scheme_make_double (SCHEME_DBL_VAL(n1) op SCHEME_DBL_VAL(n2))); \
    } \
}

#define GEN_NARY_OP(name, scheme_name, bin_name, ident) \
static Scheme_Object * \
name (int argc, Scheme_Object *argv[]) \
{ \
  Scheme_Object *ret; \
  int i; \
  ret = scheme_make_integer (ident); \
  for ( i=0 ; i<argc ; ++i ) \
    { \
      ret = bin_name (ret, argv[i]); \
    } \
  return (ret); \
}

#define GEN_TWOARY_OP(name, scheme_name, bin_name) \
static Scheme_Object * \
name (int argc, Scheme_Object *argv[]) \
{ \
  Scheme_Object *ret; \
  int i; \
  SCHEME_ASSERT ((argc > 1), #scheme_name ": wrong number of args"); \
  ret = argv[0]; \
  for ( i=1 ; i<argc ; ++i ) \
    { \
      ret = bin_name (ret, argv[i]); \
    } \
  return (ret); \
}

#define GEN_UNARY_OP(name, scheme_name, c_name) \
static Scheme_Object * \
name (int argc, Scheme_Object *argv[]) \
{ \
  SCHEME_ASSERT ((argc == 1), #scheme_name ": wrong number of args"); \
  if (SCHEME_INTP (argv[0])) \
    { \
      return (scheme_make_double (c_name (SCHEME_INT_VAL (argv[0])))); \
    } \
  else \
    { \
      return (scheme_make_double (c_name (SCHEME_DBL_VAL (argv[0])))); \
    } \
}

#define GEN_BIN_FUN(name, scheme_name, c_name) \
static Scheme_Object * \
name (int argc, Scheme_Object *argv[]) \
{ \
  Scheme_Object *n1, *n2; \
  SCHEME_ASSERT ((argc == 2), #scheme_name ": wrong number of args"); \
  SCHEME_ASSERT (SCHEME_NUMBERP(argv[0]) && SCHEME_NUMBERP(argv[1]), \
                 #scheme_name ": both args must be numbers"); \
  n1 = argv[0]; n2 = argv[1]; \
  if (SCHEME_INTP(n1)) \
    { \
      if (SCHEME_INTP(n2)) \
	return (scheme_make_integer (c_name (SCHEME_INT_VAL(n1), SCHEME_INT_VAL(n2)))); \
      else \
        return (scheme_make_double (c_name (SCHEME_INT_VAL(n1), SCHEME_DBL_VAL(n2)))); \
    } \
  else \
    { \
      if (SCHEME_INTP(n2)) \
        return (scheme_make_double (c_name (SCHEME_DBL_VAL(n1), SCHEME_INT_VAL(n2)))); \
      else \
        return (scheme_make_double (c_name (SCHEME_DBL_VAL(n1), SCHEME_DBL_VAL(n2)))); \
    } \
}


