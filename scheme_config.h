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

#ifndef SCHEME_CONFIG_H
#define SCHEME_CONFIG_H

#ifndef IN_LIBSCHEME
#error This is a private header file.
#endif

/* maximum number of arguments */
#define SCHEME_MAX_ARGS 256
/* maximum length of symbols */
#define SCHEME_MAX_SYM 1024
/* number of buckets for globals */
#define SCHEME_GLOBAL_BUCKETS 100313
/* number of buckets for symbols */
#define SCHEME_SYMBOL_BUCKETS 1023

#endif /* !SCHEME_CONFIG_H */
