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
#include "scheme_nummacs.h"
#include <math.h>
#include <string.h>

/* hold up to 4k objects in the cache */
#define INTEGER_CACHE_SIZE  2048
/* split off 1k for negative integers  */
#define INTEGER_CACHE_SPLIT 1024

/* globals */
Scheme_Value scheme_integer_type;
Scheme_Value scheme_double_type;

/* internal variables */
static Scheme_Value integer_cache[INTEGER_CACHE_SIZE];

/* locals */
static Scheme_Value number_p (int argc, Scheme_Value argv[]);
static Scheme_Value complex_p (int argc, Scheme_Value argv[]);
static Scheme_Value real_p (int argc, Scheme_Value argv[]);
static Scheme_Value rational_p (int argc, Scheme_Value argv[]);
static Scheme_Value integer_p (int argc, Scheme_Value argv[]);
static Scheme_Value exact_p (int argc, Scheme_Value argv[]);
static Scheme_Value inexact_p (int argc, Scheme_Value argv[]);
static Scheme_Value eq (int argc, Scheme_Value argv[]);
static Scheme_Value lt (int argc, Scheme_Value argv[]);
static Scheme_Value gt (int argc, Scheme_Value argv[]);
static Scheme_Value lt_eq (int argc, Scheme_Value argv[]);
static Scheme_Value gt_eq (int argc, Scheme_Value argv[]);
static Scheme_Value zero_p (int argc, Scheme_Value argv[]);
static Scheme_Value positive_p (int argc, Scheme_Value argv[]);
static Scheme_Value negative_p (int argc, Scheme_Value argv[]);
static Scheme_Value odd_p (int argc, Scheme_Value argv[]);
static Scheme_Value even_p (int argc, Scheme_Value argv[]);
static Scheme_Value max (int argc, Scheme_Value argv[]);
static Scheme_Value min (int argc, Scheme_Value argv[]);
static Scheme_Value plus (int argc, Scheme_Value argv[]);
static Scheme_Value minus (int argc, Scheme_Value argv[]);
static Scheme_Value mult (int argc, Scheme_Value argv[]);
static Scheme_Value div_prim (int argc, Scheme_Value argv[]);
static Scheme_Value abs_prim (int argc, Scheme_Value argv[]);
static Scheme_Value quotient (int argc, Scheme_Value argv[]);
static Scheme_Value rem_prim (int argc, Scheme_Value argv[]);
static Scheme_Value modulo (int argc, Scheme_Value argv[]);
static Scheme_Value gcd (int argc, Scheme_Value argv[]);
static Scheme_Value lcm (int argc, Scheme_Value argv[]);
static Scheme_Value floor_prim (int argc, Scheme_Value argv[]);
static Scheme_Value ceiling (int argc, Scheme_Value argv[]);
static Scheme_Value truncate (int argc, Scheme_Value argv[]);
static Scheme_Value scheme_round (int argc, Scheme_Value argv[]);
static Scheme_Value exp_prim (int argc, Scheme_Value argv[]);
static Scheme_Value log_prim (int argc, Scheme_Value argv[]);
static Scheme_Value sin_prim (int argc, Scheme_Value argv[]);
static Scheme_Value cos_prim (int argc, Scheme_Value argv[]);
static Scheme_Value asin_prim (int argc, Scheme_Value argv[]);
static Scheme_Value acos_prim (int argc, Scheme_Value argv[]);
static Scheme_Value atan_prim (int argc, Scheme_Value argv[]);
static Scheme_Value sqrt_prim (int argc, Scheme_Value argv[]);
static Scheme_Value expt (int argc, Scheme_Value argv[]);
static Scheme_Value exact_to_inexact (int argc, Scheme_Value argv[]);
static Scheme_Value inexact_to_exact (int argc, Scheme_Value argv[]);
static Scheme_Value number_to_string (int argc, Scheme_Value argv[]);
static Scheme_Value string_to_number (int argc, Scheme_Value argv[]);

/* exported functions */

void
scheme_init_number (Scheme_Env *env)
{
  scheme_integer_type = scheme_make_type ("<integer>");
  scheme_double_type = scheme_make_type ("<double>");
  scheme_add_global ("<integer>", scheme_integer_type, env);
  scheme_add_global ("<double>", scheme_double_type, env);
  scheme_add_prim ("number?", number_p, env);
  scheme_add_prim ("complex?", complex_p, env);
  scheme_add_prim ("real?", real_p, env);
  scheme_add_prim ("rational?", rational_p, env);
  scheme_add_prim ("integer?", integer_p, env);
  scheme_add_prim ("exact?", exact_p, env);
  scheme_add_prim ("inexact?", inexact_p, env);
  scheme_add_prim ("=", eq, env);
  scheme_add_prim ("<", lt, env);
  scheme_add_prim (">", gt, env);
  scheme_add_prim ("<=", lt_eq, env);
  scheme_add_prim (">=", gt_eq, env);
  scheme_add_prim ("zero?", zero_p, env);
  scheme_add_prim ("positive?", positive_p, env);
  scheme_add_prim ("negative?", negative_p, env);
  scheme_add_prim ("odd?", odd_p, env);
  scheme_add_prim ("even?", even_p, env);
  scheme_add_prim ("max", max, env);
  scheme_add_prim ("min", min, env);
  scheme_add_prim ("+", plus, env);
  scheme_add_prim ("-", minus, env);
  scheme_add_prim ("*", mult, env);
  scheme_add_prim ("/", div_prim, env);
  scheme_add_prim ("abs", abs_prim, env);
  scheme_add_prim ("quotient", quotient, env);
  scheme_add_prim ("remainder", rem_prim, env);
  scheme_add_prim ("modulo", modulo, env);
  scheme_add_prim ("gcd", gcd, env);
  scheme_add_prim ("lcm", lcm, env);
  scheme_add_prim ("floor", floor_prim, env);
  scheme_add_prim ("ceiling", ceiling, env);
  scheme_add_prim ("truncate", truncate, env);
  scheme_add_prim ("round", scheme_round, env);
  scheme_add_prim ("exp", exp_prim, env);
  scheme_add_prim ("log", log_prim, env);
  scheme_add_prim ("sin", sin_prim, env);
  scheme_add_prim ("cos", cos_prim, env);
  scheme_add_prim ("asin", asin_prim, env);
  scheme_add_prim ("acos", acos_prim, env);
  scheme_add_prim ("atan", atan_prim, env);
  scheme_add_prim ("sqrt", sqrt_prim, env);
  scheme_add_prim ("expt", expt, env);
  scheme_add_prim ("exact->inexact", exact_to_inexact, env);
  scheme_add_prim ("inexact->exact", inexact_to_exact, env);
  scheme_add_prim ("number->string", number_to_string, env);
  scheme_add_prim ("string->number", string_to_number, env);
}

Scheme_Value
scheme_make_integer (int i)
{
  int idx = i + INTEGER_CACHE_SPLIT;
  Scheme_Value si;

  /* check if we have an object in the cache */
  if(idx >= 0 && idx < INTEGER_CACHE_SIZE) {
    si = integer_cache[idx];
    if(si != NULL) {
      return si;
    }
  }

  /* allocate and initialize the object */
  si = scheme_alloc_object (scheme_integer_type, 0);
  SCHEME_INT_VAL (si) = i;

  /* put the object in the cache */
  if(idx >= 0 && idx < INTEGER_CACHE_SIZE) {
    integer_cache[idx] = si;
  }

  /* done */
  return (si);
}

Scheme_Value
scheme_make_double (double d)
{
  Scheme_Value sd;

  sd = scheme_alloc_object (scheme_double_type, 0);
  SCHEME_DBL_VAL (sd) = d;
  return (sd);
}

/* locals */

static Scheme_Value
number_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "number?: wrong number of args");
  return (SCHEME_NUMBERP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Value
complex_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "complex?: wrong number of args");
  return (SCHEME_NUMBERP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Value
real_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "real?: wrong number of args");
  return (SCHEME_NUMBERP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Value
rational_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "rational?: wrong number of args");
  return (SCHEME_INTP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Value
integer_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "integer?: wrong number of args");
  return (SCHEME_INTP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Value
exact_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "exact?: wrong number of args");
  return (SCHEME_INTP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Value
inexact_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "inexact?: wrong number of args");
  return (SCHEME_DBLP(argv[0]) ? scheme_true : scheme_false);
}

GEN_BIN_COMP_PROT(bin_eq);
GEN_BIN_COMP_PROT(bin_lt);
GEN_BIN_COMP_PROT(bin_gt);
GEN_BIN_COMP_PROT(bin_lt_eq);
GEN_BIN_COMP_PROT(bin_gt_eq);

GEN_NARY_COMP(eq, "=", bin_eq)
GEN_NARY_COMP(lt, "<", bin_lt)
GEN_NARY_COMP(gt, ">", bin_gt)
GEN_NARY_COMP(lt_eq, "<=", bin_lt_eq)
GEN_NARY_COMP(gt_eq, ">=", bin_gt_eq)

GEN_BIN_COMP(bin_eq, "=", ==)
GEN_BIN_COMP(bin_lt, "<", <)
GEN_BIN_COMP(bin_gt, ">", >)
GEN_BIN_COMP(bin_lt_eq, "<=", <=)
GEN_BIN_COMP(bin_gt_eq, ">=", >=)

static Scheme_Value
zero_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "zero?: wrong number of args");
  if (SCHEME_INTP(argv[0]))
    {
      return (SCHEME_INT_VAL(argv[0])==0 ? scheme_true : scheme_false);
    }
  else if (SCHEME_DBLP(argv[0]))
    {
      return (SCHEME_DBL_VAL(argv[0])==0 ? scheme_true : scheme_false);
    }
  else
    {
      scheme_signal_error ("zero?: arg must be a number");
    }
}

static Scheme_Value
positive_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "positive?: wrong number of args");
  if (SCHEME_INTP(argv[0]))
    {
      return (SCHEME_INT_VAL(argv[0])>0 ? scheme_true : scheme_false);
    }
  else if (SCHEME_DBLP(argv[0]))
    {
      return (SCHEME_DBL_VAL(argv[0])>0 ? scheme_true : scheme_false);
    }
  else
    {
      scheme_signal_error ("positive?: arg must be a number");
    }
}

static Scheme_Value
negative_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "negative?: wrong number of args");
  if (SCHEME_INTP(argv[0]))
    {
      return (SCHEME_INT_VAL(argv[0])<0 ? scheme_true : scheme_false);
    }
  else if (SCHEME_DBLP(argv[0]))
    {
      return (SCHEME_DBL_VAL(argv[0])<0 ? scheme_true : scheme_false);
    }
  else
    {
      scheme_signal_error ("negative?: arg must be a number");
    }
}

static Scheme_Value
odd_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "odd?: wrong number of args");
  if (SCHEME_INTP(argv[0]))
    {
      return (((SCHEME_INT_VAL(argv[0])%2) != 0) ? scheme_true : scheme_false);
    }
  else if (SCHEME_DBLP(argv[0]))
    {
      return (scheme_false);
    }
  else
    {
      scheme_signal_error ("odd?: arg must be a number");
    }
}

static Scheme_Value
even_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "even?: wrong number of args");
  if (SCHEME_INTP(argv[0]))
    {
      return ((SCHEME_INT_VAL(argv[0])%2)==0 ? scheme_true : scheme_false);
    }
  else if (SCHEME_DBLP(argv[0]))
    {
      return (scheme_false);
    }
  else
    {
      scheme_signal_error ("even?: arg must be a number");
    }
}

GEN_BIN_PROT(bin_plus);
GEN_BIN_PROT(bin_minus);
GEN_BIN_PROT(bin_mult);
GEN_BIN_PROT(bin_div);
GEN_BIN_PROT(bin_max);
GEN_BIN_PROT(bin_min);

GEN_BIN_OP(bin_plus, "+", +)
GEN_BIN_OP(bin_minus, "-", -)
GEN_BIN_OP(bin_mult, "*", *)

#define MAX(n1,n2) ((n1>n2) ? n1 : n2)
#define MIN(n1,n2) ((n1<n2) ? n1 : n2)

static Scheme_Value
bin_max (Scheme_Value n1, Scheme_Value n2)
{
  if (SCHEME_INTP(n1))
    {
      if (SCHEME_INTP(n2))
	return (scheme_make_integer (MAX (SCHEME_INT_VAL(n1), SCHEME_INT_VAL(n2))));
      else
	return (scheme_make_double (MAX (SCHEME_INT_VAL(n1), SCHEME_DBL_VAL(n2))));
    }
  else
    {
      if (SCHEME_INTP(n2))
	return (scheme_make_double (MAX (SCHEME_DBL_VAL(n1), SCHEME_INT_VAL(n2))));
      else
	return (scheme_make_double (MAX (SCHEME_DBL_VAL(n1), SCHEME_DBL_VAL(n2))));
    }
}

static Scheme_Value
bin_min (Scheme_Value n1, Scheme_Value n2)
{
  if (SCHEME_INTP(n1))
    {
      if (SCHEME_INTP(n2))
	return (scheme_make_integer (MIN (SCHEME_INT_VAL(n1), SCHEME_INT_VAL(n2))));
      else
	return (scheme_make_double (MIN (SCHEME_INT_VAL(n1), SCHEME_DBL_VAL(n2))));
    }
  else
    {
      if (SCHEME_INTP(n2))
	return (scheme_make_double (MIN (SCHEME_DBL_VAL(n1), SCHEME_INT_VAL(n2))));
      else
	return (scheme_make_double (MIN (SCHEME_DBL_VAL(n1), SCHEME_DBL_VAL(n2))));
    }
}

static Scheme_Value
bin_div (Scheme_Value n1, Scheme_Value n2)
{
  if (SCHEME_INTP(n1))
    {
      if (SCHEME_INTP(n2))
	return (scheme_make_double ((double)SCHEME_INT_VAL(n1) / (double)SCHEME_INT_VAL(n2)));
      else
	return (scheme_make_double ((double)SCHEME_INT_VAL(n1) / SCHEME_DBL_VAL(n2)));
    }
  else
    {
      if (SCHEME_INTP(n2))
	return (scheme_make_double (SCHEME_DBL_VAL(n1) / (double)SCHEME_INT_VAL(n2)));
      else
	return (scheme_make_double (SCHEME_DBL_VAL(n1) / SCHEME_DBL_VAL(n2)));
    }
}

GEN_TWOARY_OP(max, "max", bin_max)
GEN_TWOARY_OP(min, "min", bin_min)
GEN_NARY_OP(plus, "+", bin_plus, 0)
GEN_NARY_OP(mult, "*", bin_mult, 1)

static Scheme_Value
minus (int argc, Scheme_Value argv[])
{
  Scheme_Value ret;
  int i;

  SCHEME_ASSERT ((argc > 0), "-: need at least one arg");
  ret = argv[0];
  if (argc == 1)
    {
      ret = bin_minus (scheme_make_integer(0), ret);
    }
  for ( i=1 ; i<argc ; ++i )
    {
      ret = bin_minus (ret, argv[i]);
    }
  return (ret);
}

static Scheme_Value
div_prim (int argc, Scheme_Value argv[])
{
  Scheme_Value ret;
  int i;

  SCHEME_ASSERT ((argc > 0), "/: need at least one arg");
  ret = argv[0];
  if (argc == 1)
    {
      ret = bin_div (scheme_make_integer(1), ret);
    }
  for ( i=1 ; i<argc ; ++i )
    {
      ret = bin_div (ret, argv[i]);
    }
  return (ret);
}

#define ABS(n)  ((n>0) ? n : -n)

static Scheme_Value
abs_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "abs: wrong number of args");
  if (SCHEME_INTP(argv[0]))
    {
      return (scheme_make_integer (ABS (SCHEME_INT_VAL(argv[0]))));
    }
  else if (SCHEME_DBLP(argv[0]))
    {
      return (scheme_make_double (fabs (SCHEME_DBL_VAL(argv[0]))));
    }
  else
    {
	  scheme_signal_error("abs: not a number");
	}
}

static Scheme_Value bin_quotient (Scheme_Value n1, Scheme_Value n2);

static Scheme_Value
bin_quotient (Scheme_Value n1, Scheme_Value n2)
{
  SCHEME_ASSERT ((SCHEME_NUMBERP(n1) && SCHEME_NUMBERP(n2)),
		 "quotient: args must be numbers");
  if (SCHEME_INTP(n1))
    {
      if (SCHEME_INTP(n2))
	return (scheme_make_integer (SCHEME_INT_VAL(n1) / SCHEME_INT_VAL(n2)));
      else
	return (scheme_make_double (SCHEME_INT_VAL(n1) / (int)SCHEME_DBL_VAL(n2)));
    }
  else
    {
      if (SCHEME_INTP(n2))
	return (scheme_make_double ((int)SCHEME_DBL_VAL(n1) / SCHEME_INT_VAL(n2)));
      else
	return (scheme_make_double ((int)SCHEME_DBL_VAL(n1) / (int)SCHEME_DBL_VAL(n2)));
    }
}
static Scheme_Value
quotient (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 2), "quotient: wrong number of args");
  SCHEME_ASSERT ((SCHEME_NUMBERP(argv[0]) && SCHEME_NUMBERP(argv[1])),
		 "quotient: args must be numbers");
  return (bin_quotient (argv[0], argv[1]));
}

static Scheme_Value
rem_prim (int argc, Scheme_Value argv[])
{
  Scheme_Value n1, n2;

  SCHEME_ASSERT ((argc == 2), "remainder: wrong number of args");
  n1 = argv[0];
  n2 = argv[1];
  SCHEME_ASSERT ((SCHEME_NUMBERP(n1) && SCHEME_NUMBERP(n2)),
		 "remainder: args must be numbers");
  if (SCHEME_INTP(n1))
    {
      if (SCHEME_INTP(n2))
	return (scheme_make_integer (SCHEME_INT_VAL(n1) % SCHEME_INT_VAL(n2)));
      else
	return (scheme_make_double (SCHEME_INT_VAL(n1) % (int)SCHEME_DBL_VAL(n2)));
    }
  else
    {
      if (SCHEME_INTP(n2))
	return (scheme_make_double ((int)SCHEME_DBL_VAL(n1) % SCHEME_INT_VAL(n2)));
      else
	return (scheme_make_double ((int)SCHEME_DBL_VAL(n1) % (int)SCHEME_DBL_VAL(n2)));
    }
}

static Scheme_Value
modulo (int argc, Scheme_Value argv[])
{
  Scheme_Value n1, n2;

  SCHEME_ASSERT ((argc == 2), "modulo: wrong number of args");
  n1 = argv[0];
  n2 = argv[1];
  SCHEME_ASSERT ((SCHEME_NUMBERP(n1) && SCHEME_NUMBERP(n2)),
		 "modulo: args must be numbers");
  if (SCHEME_INTP(n1))
    {
      if (SCHEME_INTP(n2))
	{
	  int i1, i2, i;

	  i1 = SCHEME_INT_VAL(n1);
	  i2 = SCHEME_INT_VAL(n2);
	  i = i1 % i2;
	  return (scheme_make_integer (((i2<0) ? (i>0) : (i<0)) ? i+i2 : i));
	}
      else
	{
	  int i1, i2, i;

	  i1 = SCHEME_INT_VAL(n1);
	  i2 = (int)SCHEME_DBL_VAL(n2);
	  i = i1 % i2;
	  return (scheme_make_integer (((i2<0) ? (i>0) : (i<0)) ? i+i2 : i));
	}
    }
  else
    {
      if (SCHEME_INTP(n2))
	{
	  int i1, i2, i;

	  i1 = (int)SCHEME_DBL_VAL(n1);
	  i2 = SCHEME_INT_VAL(n2);
	  i = i1 % i2;
	  return (scheme_make_integer (((i2<0) ? (i>0) : (i<0)) ? i+i2 : i));
	}
      else
	{
	  int i1, i2, i;

	  i1 = (int)SCHEME_DBL_VAL(n1);
	  i2 = (int)SCHEME_DBL_VAL(n2);
	  i = i1 % i2;
	  return (scheme_make_integer (((i2<0) ? (i>0) : (i<0)) ? i+i2 : i));
	}
    }
}

static Scheme_Value bin_gcd (Scheme_Value n1, Scheme_Value n2);
static Scheme_Value bin_lcm (Scheme_Value n1, Scheme_Value n2);

GEN_NARY_OP(gcd, gcd, bin_gcd, 0)
GEN_NARY_OP(lcm, lcm, bin_lcm, 1)

static Scheme_Value
bin_gcd (Scheme_Value n1, Scheme_Value n2)
{
  int i1, i2, a, b, r;

  SCHEME_ASSERT (SCHEME_NUMBERP(n1) && SCHEME_NUMBERP(n2),
		 "gcd: all arguments must be number");
  i1 = (SCHEME_INTP(n1) ? SCHEME_INT_VAL(n1) : SCHEME_DBL_VAL(n1));
  i2 = (SCHEME_INTP(n2) ? SCHEME_INT_VAL(n2) : SCHEME_DBL_VAL(n2));
  i1 = ABS (i1);
  i2 = ABS (i2);
  a = MAX(i1,i2);
  b = MIN(i1,i2);
  r = 1;
  while (b && (r > 0))
    {
      r = a % b;
      a = b;
      b = r;
    }
  if (SCHEME_DBLP(n1) || SCHEME_DBLP(n2))
    {
      return (scheme_make_double(a));
    }
  else
    {
      return (scheme_make_integer(a));
    }
}

static Scheme_Value
bin_lcm (Scheme_Value n1, Scheme_Value n2)
{
  Scheme_Value d, ret;

  d = bin_gcd (n1, n2);
  if (SCHEME_DBLP (d) && SCHEME_DBL_VAL(d)==0)
    {
      return (d);
    }
  if (SCHEME_INTP (d) && SCHEME_INT_VAL(d)==0)
    {
      return (d);
    }
  ret = (bin_mult (n1, bin_quotient (n2, d)));
  if (SCHEME_DBLP (ret))
    {
      SCHEME_DBL_VAL(ret) = ABS (SCHEME_DBL_VAL(ret));
    }
  if (SCHEME_INTP (ret))
    {
      SCHEME_INT_VAL(ret) = ABS (SCHEME_INT_VAL(ret));
    }
  return (ret);
}

static Scheme_Value
floor_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "floor: wrong number of args");
  if (SCHEME_INTP (argv[0]))
    {
      return (argv[0]);
    }
  else if (SCHEME_DBLP (argv[0]))
    {
      return scheme_make_integer (floor (SCHEME_DBL_VAL(argv[0])));
    }
  else
    {
      scheme_signal_error ("floor: arg must be a number");
    }
}

static Scheme_Value
ceiling (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "ceiling: wrong number of args");
  if (SCHEME_INTP (argv[0]))
    {
      return (argv[0]);
    }
  else if (SCHEME_DBLP (argv[0]))
    {
      return scheme_make_integer (ceil (SCHEME_DBL_VAL(argv[0])));
    }
  else
    {
      scheme_signal_error ("ceiling: arg must be a number");
    }
}

static Scheme_Value
truncate (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "truncate: wrong number of args");
  if (SCHEME_INTP (argv[0]))
    {
      return (argv[0]);
    }
  else if (SCHEME_DBLP (argv[0]))
    {
      return scheme_make_integer ((int)(SCHEME_DBL_VAL(argv[0])));
    }
  else
    {
      scheme_signal_error ("truncate: arg must be a number");
    }
}

static Scheme_Value
scheme_round (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "round: wrong number of args");
  if (SCHEME_INTP (argv[0]))
    {
      return (argv[0]);
    }
  else if (SCHEME_DBLP (argv[0]))
    {
      double val, fl, cl, hf;

      val = SCHEME_DBL_VAL(argv[0]);
      fl = floor (val);
      cl = ceil (val);
      hf = fl + 0.5;
      if (val > hf)
	{
	  return (scheme_make_integer (cl));
	}
      else
	{
	  return (scheme_make_integer (fl));
	}
    }
  else
    {
      scheme_signal_error ("round: arg must be a number");
    }
}

GEN_UNARY_OP(exp_prim, exp, exp)
GEN_UNARY_OP(log_prim, log, log)
GEN_UNARY_OP(sin_prim, sin, sin)
GEN_UNARY_OP(cos_prim, cos, cos)
GEN_UNARY_OP(asin_prim, asin, asin)
GEN_UNARY_OP(acos_prim, acos, acos)

static Scheme_Value
atan_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc==1 || argc==2), "atan: wrong number of args");
  scheme_signal_error ("atan: unimplemented");
}

static Scheme_Value
sqrt_prim (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "sqrt: wrong number of args");
  if (SCHEME_INTP (argv[0]))
    {
      return (scheme_make_double (sqrt (SCHEME_INT_VAL (argv[0]))));
    }
  else if (SCHEME_DBLP (argv[0]))
    {
      return (scheme_make_double (sqrt (SCHEME_DBL_VAL (argv[0]))));
    }
  else
    {
      scheme_signal_error ("sqrt: arg must be a number");
    }
}


GEN_BIN_FUN(expt, expt, pow)

static Scheme_Value
exact_to_inexact (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "exact->inexact: wrong number of args");
  if (SCHEME_INTP (argv[0]))
    {
      return (scheme_make_double (SCHEME_INT_VAL(argv[0])));
    }
  else if (SCHEME_DBLP (argv[0]))
    {
      return (argv[0]);
    }
  else
    {
      scheme_signal_error ("exact->inexact: arg must be a number");
    }
}

static Scheme_Value
inexact_to_exact (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "inexact->exact: wrong number of args");
  if (SCHEME_INTP (argv[0]))
    {
      return (argv[0]);
    }
  else if (SCHEME_DBLP (argv[0]))
    {
      return (scheme_make_integer (SCHEME_DBL_VAL (argv[0])));
    }
  else
    {
      scheme_signal_error ("inexact->exact: arg must be a number");
    }
}

static Scheme_Value integer_to_string (int i, int radix);
static Scheme_Value double_to_string (double d);

static Scheme_Value
number_to_string (int argc, Scheme_Value argv[])
{
  int radix;

  SCHEME_ASSERT ((argc == 1) || (argc == 2), "number->string: wrong number of args");
  if (argc == 2)
    {
      SCHEME_ASSERT (SCHEME_INTP(argv[1]), "number->string: second arg must be an integer");
      radix = SCHEME_INT_VAL(argv[1]);
    }
  else
    {
      radix = 10;
    }
  if (SCHEME_INTP(argv[0]))
    {
      return (integer_to_string (SCHEME_INT_VAL(argv[0]), radix));
    }
  else if (SCHEME_DBLP(argv[0]))
    {
      return (double_to_string (SCHEME_DBL_VAL(argv[0])));
    }
  else
    {
	  scheme_signal_error("number->string: not a number");
	}
}

static Scheme_Value
integer_to_string (int i, int radix)
{
  char buf[256];

  switch ( radix )
    {
    case 2:
      scheme_signal_error ("number->string: cannot handle radix of 2 (bug)");
      break;
    case 8:
      sprintf (buf, "%o", i);
      break;
    case 10:
      sprintf (buf, "%d", i);
      break;
    case 16:
      sprintf (buf, "%x", i);
      break;
    default:
      scheme_signal_error ("number->string: radix must be 2, 8, 10 or 16: %d", radix);
    }
  return (scheme_make_string (buf));
}

static Scheme_Value
double_to_string (double d)
{
  char buf[256];

  sprintf (buf, "%f", d);
  return (scheme_make_string (buf));
}

static Scheme_Value
string_to_number (int argc, Scheme_Value argv[])
{
  int base, val, len, is_float, i;
  char *ptr, *str;
  double d;

  SCHEME_ASSERT ((argc == 1 || argc == 2), "string->number: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "string->number: first arg must be a string");
  if (argc == 2)
    {
      SCHEME_ASSERT (SCHEME_INTP(argv[1]), "string->number: second arg must be an integer");
      base = SCHEME_INT_VAL (argv[1]);
    }
  else
    {
      base = 10;
    }
  str = SCHEME_STR_VAL (argv[0]);
  len = strlen (str);
  if (! len)
    {
      return (scheme_false);
    }
  is_float = 0;
  for ( i=0 ; i<len ; ++i )
    {
      int ch = str[i];
      if ((ch == '.') || (ch == 'e') || (ch == 'E'))
	{
	  is_float = 1;
	}
    }
  if ( is_float )
    {
      d = strtod (str, &ptr);
      if ((ptr - str) < len)
	{
	  return (scheme_false);
	}
      else
	{
	  return (scheme_make_double (d));
	}
    }
  else
    {
      val = strtol (SCHEME_STR_VAL(argv[0]), &ptr, base);
      if ((ptr - str) < len)
	{
	  return (scheme_false);
	}
      else
	{
	  return (scheme_make_integer (val));
	}
    }
}
