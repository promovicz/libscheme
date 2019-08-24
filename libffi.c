
#include <scheme.h>
#include <ffi.h>

/* variables */
static Scheme_Object *libffi_cif_type;
static Scheme_Object *libffi_type_type;

/* utilities */
static Scheme_Object *libffi_make_cif_object(ffi_cif *cif, Scheme_Object *ai);
static Scheme_Object *libffi_make_type_object(ffi_type *typ);

/* functions */
static Scheme_Object *libffi_call (int argc, Scheme_Object *argv[]);
static Scheme_Object *libffi_prep (int argc, Scheme_Object *argv[]);

/* accessors */
static Scheme_Object *libffi_type_size (int argc, Scheme_Object *argv[]);
static Scheme_Object *libffi_type_alignment (int argc, Scheme_Object *argv[]);

/* macros */
#define LIBFFI_CIFP(obj)   (SCHEME_TYPE(obj) == libffi_cif_type)
#define LIBFFI_TYPEP(obj)  (SCHEME_TYPE(obj) == libffi_type_type)

/* functions */

void
scheme_init_libffi (Scheme_Env *env)
{
  /* types */
  libffi_cif_type = scheme_make_type("<ffi-cif>");
  libffi_type_type = scheme_make_type("<ffi-type>");

  /* functions */
  scheme_add_global("ffi-call", scheme_make_prim(libffi_call), env);
  scheme_add_global("ffi-prep", scheme_make_prim(libffi_prep), env);

  /* accessors */
  scheme_add_global("ffi-type-size", scheme_make_prim(libffi_type_size), env);
  scheme_add_global("ffi-type-alignment", scheme_make_prim(libffi_type_alignment), env);

  /* constants */
  scheme_add_global("ffi-void", libffi_make_type_object(&ffi_type_void), env);
#if 0
  scheme_add_global("ffi-uint8", libffi_make_type_object(&ffi_type_uint8), env);
  scheme_add_global("ffi-sint8", libffi_make_type_object(&ffi_type_sint8), env);
  scheme_add_global("ffi-uint16", libffi_make_type_object(&ffi_type_uint16), env);
  scheme_add_global("ffi-sint16", libffi_make_type_object(&ffi_type_sint16), env);
  scheme_add_global("ffi-uint32", libffi_make_type_object(&ffi_type_uint32), env);
  scheme_add_global("ffi-sint32", libffi_make_type_object(&ffi_type_sint32), env);
  scheme_add_global("ffi-uint64", libffi_make_type_object(&ffi_type_uint64), env);
  scheme_add_global("ffi-sint64", libffi_make_type_object(&ffi_type_sint64), env);
#endif
  scheme_add_global("ffi-uchar", libffi_make_type_object(&ffi_type_uchar), env);
  scheme_add_global("ffi-schar", libffi_make_type_object(&ffi_type_schar), env);
  scheme_add_global("ffi-ushort", libffi_make_type_object(&ffi_type_ushort), env);
  scheme_add_global("ffi-sshort", libffi_make_type_object(&ffi_type_sshort), env);
  scheme_add_global("ffi-uint", libffi_make_type_object(&ffi_type_uint), env);
  scheme_add_global("ffi-sint", libffi_make_type_object(&ffi_type_sint), env);
#if 0
  scheme_add_global("ffi-ulong", libffi_make_type_object(&ffi_type_ulong), env);
  scheme_add_global("ffi-slong", libffi_make_type_object(&ffi_type_slong), env);
#endif
  scheme_add_global("ffi-pointer", libffi_make_type_object(&ffi_type_pointer), env);
#if 0
  scheme_add_global("ffi-float", libffi_make_type_object(&ffi_type_float), env);
#endif
  scheme_add_global("ffi-double", libffi_make_type_object(&ffi_type_double), env);
#if 0
  scheme_add_global("ffi-longdouble", libffi_make_type_object(&ffi_type_longdouble), env);
  scheme_add_global("ffi-complex-float", libffi_make_type_object(&ffi_type_complex_float), env);
  scheme_add_global("ffi-complex-double", libffi_make_type_object(&ffi_type_complex_double), env);
  scheme_add_global("ffi-complex-longdouble", libffi_make_type_object(&ffi_type_complex_longdouble), env);
#endif
}

/* utilities */

static Scheme_Object *
libffi_make_cif_object (ffi_cif *cif, Scheme_Object *ai)
{
  Scheme_Object *cif_obj;
  cif_obj = scheme_alloc_object (libffi_cif_type, 0);
  SCHEME_PTR1_VAL (cif_obj) = cif;
  SCHEME_PTR2_VAL (cif_obj) = ai;
  return (cif_obj);
}

static Scheme_Object *
libffi_make_type_object (ffi_type *typ)
{
  Scheme_Object *typ_obj;
  typ_obj = scheme_alloc_object (libffi_type_type, 0);
  SCHEME_PTR_VAL (typ_obj) = typ;
  return (typ_obj);
}

/* functions */

static Scheme_Object *
libffi_call (int argc, Scheme_Object *argv[])
{
  int i, num_av;
  ffi_cif *cif;
  void *fun;
  void **av;
  Scheme_Object *rv;

  /* check arguments */
  SCHEME_ASSERT((argc >= 2), "ffi-call: wrong number of args");
  SCHEME_ASSERT(LIBFFI_CIFP(argv[0]), "ffi-call: first arg must be a cif");
  SCHEME_ASSERT(SCHEME_POINTERP(argv[1]), "ffi-call: second arg must be a pointer");
  /* get main arguments */
  cif = SCHEME_PTR_VAL(argv[0]);
  fun = SCHEME_PTR_VAL(argv[1]);
  /* convert arguments */
  num_av = (argc - 2);
  av = (void **) scheme_malloc(sizeof(void *) * num_av);
  for ( i=0 ; i<num_av ; ++i )
    {
	  av[i] = &argv[2 + i]->u;
	}
  /* allocate return value */
  rv = scheme_malloc(sizeof(Scheme_Object));
  /* call the function */
  ffi_call(cif, fun, &rv->u, av);
  /* return the result */
  return rv;
}

static Scheme_Object *
libffi_prep (int argc, Scheme_Object *argv[])
{
  int i, num_at;
  ffi_status s;
  ffi_cif *cif;
  ffi_type *rt, **at;
  Scheme_Object *r, *al, *a, *ai, **aiv;

  /* check arguments */
  SCHEME_ASSERT ((argc == 2), "ffi-prep: wrong number of args");
  SCHEME_ASSERT (LIBFFI_TYPEP(argv[0]), "ffi-prep: first arg must be a type");
  SCHEME_ASSERT (SCHEME_LISTP(argv[1]), "ffi-prep: second arg must be a list"); 
  /* get main arguments */
  r = argv[0];
  rt = SCHEME_PTR_VAL(r);
  al = argv[1];
  /* create info vector */
  ai = scheme_make_vector(1 + num_at, scheme_false);
  aiv = SCHEME_VEC_ELS(ai);
  aiv[0] = r;
  /* convert ffi argument types */
  num_at = scheme_list_length(al);
  at = (ffi_type **) scheme_malloc(sizeof(ffi_type *) * num_at);
  for ( i=0 ; i<num_at ; ++i )
    {
      a = SCHEME_CAR(al);
      /* check type */
      SCHEME_ASSERT (LIBFFI_TYPEP(a), "ffi-prep: all elements of arg list must be ffi types");
      /* add to type vector */
      at[i] = SCHEME_PTR_VAL(a);
      /* add to info vector */
      aiv[i + 1] = a;
      /* next element */
      al = SCHEME_CDR(al);
    }
  /* allocate cif */
  cif = (ffi_cif *) scheme_malloc(sizeof(ffi_cif));
  /* prepare cif */
  s = ffi_prep_cif(cif, FFI_DEFAULT_ABI, num_at, rt, at);
  SCHEME_ASSERT((s == FFI_OK), "ffi-prep: could not prepare call interface");
  /* return cif as object */
  return (libffi_make_cif_object(cif, ai));
}

/* accessors */

static Scheme_Object *
libffi_type_size (int argc, Scheme_Object *argv[])
{
  ffi_type *t;

  /* check arguments */
  SCHEME_ASSERT ((argc == 1), "ffi-type-size: wrong number of args");
  SCHEME_ASSERT (LIBFFI_TYPEP(argv[0]), "ffi-type-size: first arg must be a type");
  /* get type */
  t = SCHEME_PTR_VAL(argv[0]);
  /* return result */
  return (scheme_make_integer(t->size));
}

static Scheme_Object *
libffi_type_alignment (int argc, Scheme_Object *argv[])
{
  ffi_type *t;

  /* check arguments */
  SCHEME_ASSERT ((argc == 1), "ffi-type-alignment: wrong number of args");
  SCHEME_ASSERT (LIBFFI_TYPEP(argv[0]), "ffi-type-alignment: first arg must be a type");
  /* get type */
  t = SCHEME_PTR_VAL(argv[0]);
  /* return result */
  return (scheme_make_integer(t->alignment));
}

