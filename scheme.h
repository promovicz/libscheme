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

#ifndef SCHEME_H
#define SCHEME_H

#include <stdio.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __GNUC__
#define SCHEME_FUN_CONST    __attribute__((const))
#define SCHEME_FUN_PURE     __attribute__((pure))
#define SCHEME_FUN_NORETURN __attribute__((noreturn))
#else
#define SCHEME_FUN_CONST
#define SCHEME_FUN_PURE
#define SCHEME_FUN_NORETURN
#endif

struct Scheme_Object;
typedef struct Scheme_Object Scheme_Object;
typedef struct Scheme_Object * Scheme_Value;

struct Scheme_Bucket
{
  char *key;
  void *val;
  struct Scheme_Bucket *next;
};
typedef struct Scheme_Bucket Scheme_Bucket;

struct Scheme_Hash_Table
{
  int size;
  Scheme_Bucket **buckets;
};
typedef struct Scheme_Hash_Table Scheme_Hash_Table;

struct Scheme_Env
{
  int num_bindings;
  Scheme_Value *symbols;
  Scheme_Value *values;
  Scheme_Hash_Table *globals;
  struct Scheme_Env *next;
};
typedef struct Scheme_Env Scheme_Env;

struct Scheme_Cont
{
  int escaped;
  jmp_buf buffer;
  Scheme_Value retval;
};
typedef struct Scheme_Cont Scheme_Cont;

struct Scheme_Object
{
  union
    {
      char char_val;
      int int_val;
      double double_val;
      char *string_val;
      void *ptr_val;
      struct Scheme_Cont *cont_val;
      struct { void *ptr1, *ptr2; } two_ptr_val;
      Scheme_Value (*prim_val)
        (int argc, Scheme_Value argv[]);
      Scheme_Value (*syntax_val)
        (Scheme_Value form, struct Scheme_Env *env);
      struct { Scheme_Value car, cdr; } pair_val;
      struct { int size; Scheme_Value *els; } vector_val;
      struct { struct Scheme_Env *env; Scheme_Value code; } closure_val;
      struct { Scheme_Value def; struct Scheme_Method *meths; } methods_val;

    } u;
  Scheme_Value type;
};

/* access macros */
#define SCHEME_TYPE(obj)     ((obj)->type)
#define SCHEME_CHAR_VAL(obj) ((obj)->u.char_val)
#define SCHEME_INT_VAL(obj)  ((obj)->u.int_val)
#define SCHEME_DBL_VAL(obj)  ((obj)->u.double_val)
#define SCHEME_STR_VAL(obj)  ((obj)->u.string_val)
#define SCHEME_PTR_VAL(obj)  ((obj)->u.ptr_val)
#define SCHEME_CONT_VAL(obj) ((obj)->u.cont_val)
#define SCHEME_PTR1_VAL(obj) ((obj)->u.two_ptr_val.ptr1)
#define SCHEME_PTR2_VAL(obj) ((obj)->u.two_ptr_val.ptr2)
#define SCHEME_SYNTAX(obj)   ((obj)->u.syntax_val)
#define SCHEME_PRIM(obj)     ((obj)->u.prim_val)
#define SCHEME_CAR(obj)      ((obj)->u.pair_val.car)
#define SCHEME_CDR(obj)      ((obj)->u.pair_val.cdr)
#define SCHEME_VEC_SIZE(obj) ((obj)->u.vector_val.size)
#define SCHEME_VEC_ELS(obj)  ((obj)->u.vector_val.els)
#define SCHEME_CLOS_ENV(obj) ((obj)->u.closure_val.env)
#define SCHEME_CLOS_CODE(obj)((obj)->u.closure_val.code)
#define SCHEME_METH_DEF(obj) ((obj)->u.methods_val.def)
#define SCHEME_METHS(obj)    ((obj)->u.methods_val.meths)

struct Scheme_Method
{
  Scheme_Value type;
  Scheme_Value fun;
  struct Scheme_Method *next;
};
typedef struct Scheme_Method Scheme_Method;

typedef Scheme_Value
(Scheme_Prim) (int argc, Scheme_Value argv[]);

typedef Scheme_Value
(Scheme_Syntax) (Scheme_Value form, struct Scheme_Env *env);

/* error handling */
extern jmp_buf scheme_error_buf;
SCHEME_FUN_NORETURN void scheme_signal_error (char *msg, ...);
void scheme_warning (char *msg, ...);
void scheme_default_handler (void);
#define SCHEME_CATCH_ERROR(try_expr, err_expr) \
  (setjmp(scheme_error_buf) ? (err_expr) : (try_expr))
#define SCHEME_ASSERT(expr,msg) \
  ((expr) ? 0 : (scheme_signal_error(msg)))

/* types */
extern Scheme_Value scheme_type_type;
extern Scheme_Value scheme_char_type;
extern Scheme_Value scheme_integer_type, scheme_double_type;
extern Scheme_Value scheme_string_type, scheme_symbol_type;
extern Scheme_Value scheme_null_type, scheme_pair_type;
extern Scheme_Value scheme_vector_type;
extern Scheme_Value scheme_prim_type, scheme_closure_type;
extern Scheme_Value scheme_cont_type;
extern Scheme_Value scheme_input_port_type, scheme_output_port_type;
extern Scheme_Value scheme_eof_type;
extern Scheme_Value scheme_true_type, scheme_false_type;
extern Scheme_Value scheme_syntax_type, scheme_macro_type;
extern Scheme_Value scheme_promise_type, scheme_struct_proc_type;
extern Scheme_Value scheme_pointer_type;

/* common symbols */
extern Scheme_Value scheme_quote_symbol;
extern Scheme_Value scheme_quasiquote_symbol;
extern Scheme_Value scheme_unquote_symbol;
extern Scheme_Value scheme_unquote_splicing_symbol;

/* constants */
extern Scheme_Value scheme_eof;
extern Scheme_Value scheme_null;
extern Scheme_Value scheme_true;
extern Scheme_Value scheme_false;

/* basics */
Scheme_Value scheme_read (Scheme_Value port);
Scheme_Value scheme_eval (Scheme_Value obj, Scheme_Env *env);
void scheme_write (Scheme_Value obj, Scheme_Value port);
void scheme_display (Scheme_Value obj, Scheme_Value port);
void scheme_write_string (char *str, Scheme_Value port);
char *scheme_write_to_string (Scheme_Value obj);
char *scheme_display_to_string (Scheme_Value obj);
void scheme_debug_print (Scheme_Value obj);
Scheme_Value scheme_apply (Scheme_Value rator, int num_rands, Scheme_Value *rands);
Scheme_Value scheme_apply_to_list (Scheme_Value rator, Scheme_Value rands);
Scheme_Value scheme_apply_struct_proc (Scheme_Value rator, Scheme_Value rands);
Scheme_Value scheme_alloc_object (Scheme_Value type, size_t nbytes);
void *scheme_malloc (size_t size);
void *scheme_calloc (size_t num, size_t size);
char *scheme_strdup (char *str);

/* hash table interface */
Scheme_Hash_Table *scheme_hash_table (int size);
void scheme_add_to_table (Scheme_Hash_Table *table, char *key, void *val);
void scheme_change_in_table (Scheme_Hash_Table *table, char *key, void *new_val);
void *scheme_lookup_in_table (Scheme_Hash_Table *table, char *key);

/* constructors */
Scheme_Value scheme_make_prim (Scheme_Prim *prim);
Scheme_Value scheme_make_closure (Scheme_Env *env, Scheme_Value code);
Scheme_Value scheme_make_cont ();
Scheme_Value scheme_make_type (const char *name);
Scheme_Value scheme_make_pair (Scheme_Value car, Scheme_Value cdr);
Scheme_Value scheme_make_string (const char *chars);
Scheme_Value scheme_alloc_string (int size, char fill);
Scheme_Value scheme_make_vector (int size, Scheme_Value fill);
Scheme_Value scheme_make_integer (int i);
Scheme_Value scheme_make_double (double d);
Scheme_Value scheme_make_char (char ch);
Scheme_Value scheme_make_syntax (Scheme_Syntax *syntax);
Scheme_Value scheme_make_promise (Scheme_Value expr, Scheme_Env *env);
Scheme_Value scheme_make_pointer (void *ptr);

/* generic port support */

struct Scheme_Input_Port
{
  Scheme_Value sub_type;
  void *port_data;
  int (*getc_fun) (struct Scheme_Input_Port *port);
  void (*ungetc_fun) (int ch, struct Scheme_Input_Port *port);
  int (*char_ready_fun) (struct Scheme_Input_Port *port);
  void (*close_fun) (struct Scheme_Input_Port *port);
};
typedef struct Scheme_Input_Port Scheme_Input_Port;

struct Scheme_Output_Port
{
  Scheme_Value sub_type;
  void *port_data;
  void (*write_string_fun) (char *str, struct Scheme_Output_Port *);
  void (*close_fun) (struct Scheme_Output_Port *);
};
typedef struct Scheme_Output_Port Scheme_Output_Port;

int scheme_getc (Scheme_Value port);
void scheme_ungetc (int ch, Scheme_Value port);
int scheme_char_ready (Scheme_Value port);
void scheme_close_input_port (Scheme_Value port);
void scheme_close_output_port (Scheme_Value port);

Scheme_Value
scheme_make_input_port (
  Scheme_Value subtype,
  void *data,
  int (*getc_fun) (Scheme_Input_Port*),
  void (*ungetc_fun) (int, Scheme_Input_Port*),
  int (*char_ready_fun) (Scheme_Input_Port*),
  void (*close_fun) (Scheme_Input_Port*)
);
Scheme_Value
scheme_make_output_port (
  Scheme_Value subtype,
  void *data,
  void (*write_string_fun) (char *str, Scheme_Output_Port*),
  void (*close_fun) (Scheme_Output_Port*)
);
Scheme_Value scheme_make_file_input_port (FILE *fp);
Scheme_Value scheme_make_string_input_port (char *str);
Scheme_Value scheme_make_file_output_port (FILE *fp);
Scheme_Value scheme_make_string_output_port (char *str);
extern Scheme_Value scheme_stdin_port;
extern Scheme_Value scheme_stdout_port;
extern Scheme_Value scheme_stderr_port;

/* environment */
void scheme_add_global (char *name, Scheme_Value val, Scheme_Env *env);
Scheme_Env *scheme_new_frame (int num_bindings);
void scheme_add_binding (int index, Scheme_Value sym, Scheme_Value val, Scheme_Env *frame);
Scheme_Env *scheme_extend_env (Scheme_Env *frame, Scheme_Env *env);
Scheme_Env *scheme_add_frame (Scheme_Value syms, Scheme_Value vals, Scheme_Env *env);
Scheme_Env *scheme_pop_frame (Scheme_Env *env);
void scheme_set_value (Scheme_Value var, Scheme_Value val, Scheme_Env *env);
Scheme_Value scheme_lookup_value (Scheme_Value symbol, Scheme_Env *env);
Scheme_Value scheme_lookup_global (Scheme_Value symbol, Scheme_Env *env);
extern Scheme_Env *scheme_env;

/* symbols */
Scheme_Value scheme_intern_symbol (char *name);

/* initialization */
Scheme_Env *scheme_basic_env (void);
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

/* misc */
SCHEME_FUN_CONST int scheme_eq (Scheme_Value obj1, Scheme_Value obj2);
SCHEME_FUN_PURE  int scheme_eqv (Scheme_Value obj1, Scheme_Value obj2);
SCHEME_FUN_PURE  int scheme_equal (Scheme_Value obj1, Scheme_Value obj2);
SCHEME_FUN_PURE  int scheme_list_length (Scheme_Value list);
Scheme_Value scheme_alloc_list (int size);
Scheme_Value scheme_map_1 (Scheme_Value (*fun)(Scheme_Object*), Scheme_Value lst);
SCHEME_FUN_PURE  Scheme_Value scheme_car (Scheme_Value pair);
SCHEME_FUN_PURE  Scheme_Value scheme_cdr (Scheme_Value pair);
SCHEME_FUN_PURE  Scheme_Value scheme_cadr (Scheme_Value pair);
SCHEME_FUN_PURE  Scheme_Value scheme_caddr (Scheme_Value pair);
Scheme_Value scheme_vector_to_list (Scheme_Value vec);
Scheme_Value scheme_list_to_vector (Scheme_Value list);

/* convenience macros */
#define SCHEME_CHARP(obj)    (SCHEME_TYPE(obj) == scheme_char_type)
#define SCHEME_INTP(obj)     (SCHEME_TYPE(obj) == scheme_integer_type)
#define SCHEME_DBLP(obj)     (SCHEME_TYPE(obj) == scheme_double_type)
#define SCHEME_NUMBERP(obj)  (SCHEME_INTP(obj) || SCHEME_DBLP(obj))
#define SCHEME_STRINGP(obj)  (SCHEME_TYPE(obj) == scheme_string_type)
#define SCHEME_SYMBOLP(obj)  (SCHEME_TYPE(obj) == scheme_symbol_type)
#define SCHEME_BOOLP(obj)    ((obj == scheme_true) || (obj == scheme_false))
#define SCHEME_TRUEP(obj)    (obj == scheme_true)
#define SCHEME_FALSEP(obj)   (obj == scheme_false)
#define SCHEME_SYNTAXP(obj)  (SCHEME_TYPE(obj) == scheme_syntax_type)
#define SCHEME_PRIMP(obj)    (SCHEME_TYPE(obj) == scheme_prim_type)
#define SCHEME_CONTP(obj)    (SCHEME_TYPE(obj) == scheme_cont_type)
#define SCHEME_NULLP(obj)    (obj == scheme_null)
#define SCHEME_PAIRP(obj)    (SCHEME_TYPE(obj) == scheme_pair_type)
#define SCHEME_LISTP(obj)    (SCHEME_NULLP(obj) || SCHEME_PAIRP(obj))
#define SCHEME_VECTORP(obj)  (SCHEME_TYPE(obj) == scheme_vector_type)
#define SCHEME_CLOSUREP(obj) (SCHEME_TYPE(obj) == scheme_closure_type)
#define SCHEME_PROCP(obj)    (SCHEME_PRIMP(obj) || SCHEME_CLOSUREP(obj) || SCHEME_CONTP(obj))
#define SCHEME_INPORTP(obj)  (SCHEME_TYPE(obj) == scheme_input_port_type)
#define SCHEME_OUTPORTP(obj) (SCHEME_TYPE(obj) == scheme_output_port_type)
#define SCHEME_PORTP(obj)    (SCHEME_INPORTP(obj) || SCHEME_OUTPORTP(obj))
#define SCHEME_EOFP(obj)     (SCHEME_TYPE(obj) == scheme_eof_type)
#define SCHEME_PROMP(obj)    (SCHEME_TYPE(obj) == scheme_promise_type)
#define SCHEME_POINTERP(obj) (SCHEME_TYPE(obj) == scheme_pointer_type)
/* other */
#define SCHEME_CADR(obj)     (SCHEME_CAR (SCHEME_CDR (obj)))
#define SCHEME_CAAR(obj)     (SCHEME_CAR (SCHEME_CAR (obj)))
#define SCHEME_CDDR(obj)     (SCHEME_CDR (SCHEME_CDR (obj)))

/* constants */
#define SCHEME_MAX_ARGS 256	/* max number of args to function */

#ifdef __cplusplus
}
#endif

#endif /* ! SCHEME_H */
