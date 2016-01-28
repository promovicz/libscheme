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

struct Scheme_Struct_Proc
{
  Scheme_Object *struct_type;
  enum {SCHEME_CONSTR, SCHEME_PRED, SCHEME_GETTER, SCHEME_SETTER} proc_type;
  int slot_num;
};
typedef struct Scheme_Struct_Proc Scheme_Struct_Proc;

/* globals */
Scheme_Object *scheme_struct_proc_type;

/* locals */
static Scheme_Object *define_struct_syntax (Scheme_Object *form, Scheme_Env *env);
static Scheme_Object *scheme_make_instance (Scheme_Object *type, int num_fields);
static Scheme_Object *scheme_make_struct_proc (Scheme_Object *type, int proc_type, int field_num);
static Scheme_Object *scheme_make_constructor (Scheme_Object *type, int num_fields);
static Scheme_Object *scheme_make_pred (Scheme_Object *type);
static Scheme_Object *scheme_make_getter (Scheme_Object *type, int field);
static Scheme_Object *scheme_make_setter (Scheme_Object *type, int field);
static char *type_name (char *struct_name);
static char *constructor_name (char *struct_name);
static char *pred_name (char *struct_name);
static char *getter_name (char *struct_name, char *field_name);
static char *setter_name (char *struct_name, char *field_name);

void
scheme_init_struct (Scheme_Env *env)
{
  scheme_struct_proc_type = scheme_make_type ("<struct-procedure>");
  scheme_add_global ("define-struct", scheme_make_syntax (define_struct_syntax), env);
}

Scheme_Object *
scheme_apply_struct_proc (Scheme_Object *sp, Scheme_Object *args)
{
  Scheme_Struct_Proc *proc;

  proc = (Scheme_Struct_Proc *) SCHEME_PTR_VAL (sp);
  switch (proc->proc_type)
    {
    case SCHEME_CONSTR:
      {
	Scheme_Object *inst;
	int i;

	inst = scheme_make_instance (proc->struct_type, proc->slot_num);
	i = 0;
	while (! SCHEME_NULLP (args))
	  {
	    SCHEME_ASSERT ((i<proc->slot_num), "wrong number of args to struct constructor");
	    SCHEME_VEC_ELS(inst)[i] = SCHEME_CAR (args);
	    args = SCHEME_CDR (args);
	    i++;
	  }
	SCHEME_ASSERT ((i==proc->slot_num), "wrong number of args to struct constructor");
	return (inst);
      }
    case SCHEME_PRED:
      if (SCHEME_TYPE (SCHEME_CAR (args)) == proc->struct_type)
	{
	  return (scheme_true);
	}
      else
	{
	  return (scheme_false);
	}
    case SCHEME_GETTER:
      {
	Scheme_Object *inst;

	inst = SCHEME_CAR (args);
	SCHEME_ASSERT ((SCHEME_TYPE (inst)==proc->struct_type), "wrong type to getter function");
	return (SCHEME_VEC_ELS(inst)[proc->slot_num]);
      }
    case SCHEME_SETTER:
      {
	Scheme_Object *inst;

	inst = SCHEME_CAR (args);
	SCHEME_ASSERT ((SCHEME_TYPE (inst)==proc->struct_type), "wrong type to getter function");
	return (SCHEME_VEC_ELS(inst)[proc->slot_num] = SCHEME_CAR (SCHEME_CDR (args)));
      }
    default:
      SCHEME_ASSERT ((0), "unknown struct procedure type");
    }
}

static Scheme_Object *
define_struct_syntax (Scheme_Object *form, Scheme_Env *env)
{
  Scheme_Object *type_symbol, *field_symbols;
  Scheme_Object *getters, *last_getter;
  Scheme_Object *setters, *last_setter;
  Scheme_Object *struct_symbol, *pred_symbol;
  Scheme_Object *constructor_symbol, *type_obj;
  char *struct_name, *struct_type_name, *field_name;
  int slot_num;

  struct_symbol = SCHEME_CAR (SCHEME_CDR (form));
  field_symbols = SCHEME_CAR (SCHEME_CDR (SCHEME_CDR (form)));
  struct_name = SCHEME_STR_VAL (struct_symbol);
  
  struct_type_name = type_name (struct_name);
  type_obj = scheme_make_type (struct_type_name);
  scheme_add_global (struct_type_name, type_obj, env);

  scheme_add_global (constructor_name (struct_name), 
		     scheme_make_constructor (type_obj, scheme_list_length (field_symbols)), env);
  scheme_add_global (pred_name (struct_name), scheme_make_pred (type_obj), env);

  getters = last_getter = setters = last_setter = scheme_null;
  slot_num = 0;
  while (! SCHEME_NULLP (field_symbols))
    {
      SCHEME_ASSERT (SCHEME_PAIRP (field_symbols), "badly construct define-struct form");
      field_name = SCHEME_STR_VAL (SCHEME_CAR (field_symbols));
      scheme_add_global (getter_name (struct_name, field_name), scheme_make_getter (type_obj, slot_num), env);
      scheme_add_global (setter_name (struct_name, field_name), scheme_make_setter (type_obj, slot_num), env);
      field_symbols = SCHEME_CDR (field_symbols);
      slot_num++;
    }
  return (struct_symbol);
}

static Scheme_Object *
scheme_make_instance (Scheme_Object *type, int num_fields)
{
  Scheme_Object *inst;

  inst = scheme_alloc_object ();
  SCHEME_TYPE (inst) = type;
  SCHEME_VEC_SIZE (inst) = num_fields;
  SCHEME_VEC_ELS (inst) = (Scheme_Object **) scheme_malloc (sizeof (Scheme_Object*));
  return (inst);
}

static Scheme_Object *
scheme_make_struct_proc (Scheme_Object *type, int proc_type, int field_num)
{
  Scheme_Object *obj;
  Scheme_Struct_Proc *proc;

  proc = (Scheme_Struct_Proc *) scheme_malloc (sizeof (Scheme_Struct_Proc));
  proc->struct_type = type;
  proc->proc_type = proc_type;
  proc->slot_num = field_num;
  obj = scheme_alloc_object ();
  SCHEME_TYPE (obj) = scheme_struct_proc_type;
  SCHEME_PTR_VAL (obj) = proc;
  return (obj);
}

static Scheme_Object *
scheme_make_constructor (Scheme_Object *type, int num_fields)
{
  return (scheme_make_struct_proc (type, SCHEME_CONSTR, num_fields));
}

static Scheme_Object *
scheme_make_pred (Scheme_Object *type)
{
  return (scheme_make_struct_proc (type, SCHEME_PRED, 0));
}

static Scheme_Object *
scheme_make_getter (Scheme_Object *type, int field)
{
  return (scheme_make_struct_proc (type, SCHEME_GETTER, field));
}

static Scheme_Object *
scheme_make_setter (Scheme_Object *type, int field)
{
  return (scheme_make_struct_proc (type, SCHEME_SETTER, field));
}

static char *
type_name (char *struct_name)
{
  int orig_len, add_len;
  char *name;

  orig_len = strlen (struct_name);
  add_len = 2;			/* strlen("<") + strlen(">") */
  name = (char *) scheme_malloc (sizeof(char) * (orig_len + add_len + 1));
  name[0] = '<';
  name[1] = '\0';
  strcat (name, struct_name);
  name[orig_len+1] = '>';
  name[orig_len+2] = '\0';
  return (name);
}

static char *
constructor_name (char *struct_name)
{
  int orig_len, make_len;
  char *name;

  orig_len = strlen (struct_name);
  make_len = 5;			/* strlen ("make-") */
  name = (char *) scheme_malloc (sizeof (char) * (orig_len + make_len + 1));
  strcpy (name, "make-");
  strcat (name, struct_name);
  return (name);
}

static char *
pred_name (char *struct_name)
{
  int orig_len;
  char *name;

  orig_len = strlen (struct_name);
  name = (char *) scheme_malloc (sizeof(char) * (orig_len + 1 + 1));
  strcpy (name, struct_name);
  name[orig_len] = '?';
  name[orig_len+1] = '\0';
  return (name);
}

static char *
getter_name (char *struct_name, char *field_name)
{
  int name_len, field_len, dash_len;
  char *name;

  name_len = strlen (struct_name);
  field_len = strlen (field_name);
  dash_len = 1;			/* strlen ("-") */
  name = (char *) scheme_malloc (sizeof (char) * (name_len + dash_len + field_len + 1));
  strcpy (name, struct_name);
  strcat (name, "-");
  strcat (name, field_name);
  return (name);
}

static char *
setter_name (char *struct_name, char *field_name)
{
  int set_len, name_len, field_len, dash_len, bang_len;
  char *name;

  name_len = strlen (struct_name);
  field_len = strlen (field_name);
  set_len = 4;			/* strlen ("set-") */
  dash_len = 1;			/* strlen ("-") */
  bang_len = 1;			/* strlen ("!") */
  name = (char *) 
    scheme_malloc (sizeof (char)*(set_len + name_len + dash_len + field_len + bang_len + 1));
  strcpy (name, "set-");
  strcat (name, struct_name);
  strcat (name, "-");
  strcat (name, field_name);
  strcat (name, "!");
  return (name);
}

