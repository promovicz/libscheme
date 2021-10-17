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

#ifndef SCHEME_PRIVATE_H
#define SCHEME_PRIVATE_H

#ifndef IN_LIBSCHEME
#error This is a private header file.
#endif

#include "scheme.h"
#include "scheme_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct Scheme_Hash_Bucket;
struct Scheme_Hash_Table;
struct Scheme_Method;
struct Scheme_Port;

typedef struct Scheme_Hash_Bucket Scheme_Hash_Bucket;
typedef struct Scheme_Hash_Table Scheme_Hash_Table;
typedef struct Scheme_Method Scheme_Method;
typedef struct Scheme_Port Scheme_Port;

struct Scheme_Env
{
  int num_bindings;
  Scheme_Value *symbols;
  Scheme_Value *values;
  Scheme_Hash_Table *globals;
  struct Scheme_Env *next;
};

struct Scheme_Cont
{
  int escaped;
  jmp_buf buffer;
  Scheme_Value retval;
};

struct Scheme_Hash_Bucket
{
  char *key;
  void *val;
  struct Scheme_Hash_Bucket *next;
};

struct Scheme_Hash_Table
{
  int size;
  Scheme_Hash_Bucket **buckets;
};

struct Scheme_Method
{
  Scheme_Value type;
  Scheme_Value fun;
  struct Scheme_Method *next;
};

struct Scheme_Port
{
  FILE *stream;
};

/* init functions */
void scheme_init_char (Scheme_Env *env);
void scheme_init_bool (Scheme_Env *env);
void scheme_init_type (Scheme_Env *env);
void scheme_init_error (Scheme_Env *env);
void scheme_init_syntax (Scheme_Env *env);
void scheme_init_fun (Scheme_Env *env);
void scheme_init_symbol (Scheme_Env *env);
void scheme_init_list (Scheme_Env *env);
void scheme_init_port (Scheme_Env *env);
void scheme_init_proc (Scheme_Env *env);
void scheme_init_vector (Scheme_Env *env);
void scheme_init_string (Scheme_Env *env);
void scheme_init_number (Scheme_Env *env);
void scheme_init_eval (Scheme_Env *env);
void scheme_init_promise (Scheme_Env *env);
void scheme_init_struct (Scheme_Env *env);
void scheme_init_pointer (Scheme_Env *env);

/* hash */
Scheme_Hash_Table *scheme_make_hash_table (int size);
void scheme_add_to_table (Scheme_Hash_Table *table, char *key, void *val);
void scheme_change_in_table (Scheme_Hash_Table *table, char *key, void *new_val);
void *scheme_lookup_in_table (Scheme_Hash_Table *table, char *key);

#ifdef __cplusplus
}
#endif

#endif /* !SCHEME_PRIVATE_H */
