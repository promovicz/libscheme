
#define LIBDL_HAS_INFO

#ifdef LIBDL_HAS_INFO
/* Available on GNU and Solaris, maybe elsewhere */
#define _GNU_SOURCE
#endif

#include <scheme.h>
#include <dlfcn.h>

/* variables */
static Scheme_Value libdl_handle_type;
#ifdef LIBDL_HAS_INFO
static Scheme_Value libdl_info_type;
#endif

/* utilities */
static Scheme_Value libdl_make_handle_object(void *handle);

/* functions */
static Scheme_Value libdl_open (int argc, Scheme_Value argv[]);
static Scheme_Value libdl_close (int argc, Scheme_Value argv[]);
static Scheme_Value libdl_symbol (int argc, Scheme_Value argv[]);
static Scheme_Value libdl_address (int argc, Scheme_Value argv[]);

/* accessors */
#ifdef LIBDL_HAS_INFO
static Scheme_Value libdl_info_file_name (int argc, Scheme_Value argv[]);
static Scheme_Value libdl_info_file_address (int argc, Scheme_Value argv[]);
static Scheme_Value libdl_info_symbol_name (int argc, Scheme_Value argv[]);
static Scheme_Value libdl_info_symbol_address (int argc, Scheme_Value argv[]);
#endif

/* macros */
#define LIBDL_HANDLEP(obj) (SCHEME_TYPE(obj) == libdl_handle_type)
#ifdef LIBDL_HAS_INFO
#define LIBDL_INFOP(obj)   (SCHEME_TYPE(obj) == libdl_info_type)
#endif

/* functions */

void
scheme_init_libdl (Scheme_Env *env)
{
  /* types */
  libdl_handle_type = scheme_make_type("<dl-handle>");
#ifdef LIBDL_HAS_INFO
  libdl_info_type = scheme_make_type("<dl-info>");
#endif

  /* functions */
  scheme_add_prim("dl-open", libdl_open, env);
  scheme_add_prim("dl-close", libdl_close, env);
  scheme_add_prim("dl-symbol", libdl_symbol, env);
#ifdef LIBDL_HAS_INFO
  scheme_add_prim("dl-address", libdl_address, env);
#endif

  /* accessors */
#ifdef LIBDL_HAS_INFO
  scheme_add_prim("dl-info-file-name", libdl_info_file_name, env);
  scheme_add_prim("dl-info-file-address", libdl_info_file_address, env);
  scheme_add_prim("dl-info-symbol-name", libdl_info_symbol_name, env);
  scheme_add_prim("dl-info-symbol-address", libdl_info_symbol_address, env);
#endif

  /* constants */
  scheme_add_global ("RTLD_NOW", scheme_make_integer (RTLD_NOW), env);
  scheme_add_global ("RTLD_LAZY", scheme_make_integer (RTLD_LAZY), env);
  scheme_add_global ("RTLD_GLOBAL", scheme_make_integer (RTLD_GLOBAL), env);
  scheme_add_global ("RTLD_LOCAL", scheme_make_integer (RTLD_LOCAL), env);
#ifdef RTLD_NODELETE
  scheme_add_global ("RTLD_NODELETE", scheme_make_integer (RTLD_NODELETE), env);
#endif
#ifdef RTLD_NOLOAD
  scheme_add_global ("RTLD_NOLOAD", scheme_make_integer (RTLD_NOLOAD), env);
#endif
#ifdef RTLD_DEEPBIND
  scheme_add_global ("RTLD_DEEPBIND", scheme_make_integer (RTLD_DEEPBIND), env);
#endif
}

/* utilities */

static Scheme_Value
libdl_make_handle_object (void *handle)
{
  Scheme_Value handle_obj;
  handle_obj = scheme_alloc_object (libdl_handle_type, 0);
  SCHEME_PTR_VAL (handle_obj) = handle;
  return (handle_obj);
}

/* functions */

static Scheme_Value
libdl_open (int argc, Scheme_Value argv[])
{
  char *filename;
  int flags;
  void *handle;

  /* check arguments */
  SCHEME_ASSERT ((argc == 2), "dl-open: wrong number of args");
  SCHEME_ASSERT ((SCHEME_STRINGP(argv[0]) || SCHEME_FALSEP(argv[0])),
                 "dl-open: first arg must be a string or false");
  SCHEME_ASSERT (SCHEME_INTP(argv[1]), "dl-open: second arg must be an integer");
  /* convert arguments */
  if (SCHEME_FALSEP(argv[0]))
    filename = NULL;
  else
    filename = SCHEME_STR_VAL(argv[0]);
  flags = SCHEME_INT_VAL(argv[1]);
  /* perform the call */
  handle = dlopen(filename, flags);
  /* check result */
  SCHEME_ASSERT((handle != NULL), "dl-open: could not open requested library");
  /* return the handle */
  return libdl_make_handle_object(handle);
}

static Scheme_Value
libdl_close (int argc, Scheme_Value argv[])
{
  int res;
  void *handle;

  /* check arguments */
  SCHEME_ASSERT ((argc == 1), "dl-close: wrong number of args");
  SCHEME_ASSERT (LIBDL_HANDLEP(argv[0]), "dl-close: first arg must be a handle");
  /* convert arguments */
  handle = SCHEME_PTR_VAL(argv[0]);
  /* perform the call */
  res = dlclose(handle);
  /* check result */
  SCHEME_ASSERT((res == 0), "dl-close: failed to close the handle");
  /* return null */
  return scheme_null;
}

static Scheme_Value
libdl_symbol (int argc, Scheme_Value argv[])
{
  void *handle;
  char *symbol;
  void *pointer;

  /* check arguments */
  SCHEME_ASSERT ((argc == 2), "dl-symbol: wrong number of args");
  SCHEME_ASSERT (LIBDL_HANDLEP(argv[0]), "dl-symbol: first arg must be a handle");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[1]), "dl-symbol: second arg must be a string");
  /* convert arguments */
  handle = SCHEME_PTR_VAL(argv[0]);
  symbol = SCHEME_STR_VAL(argv[1]);
  /* perform the call */
  pointer = dlsym(handle, symbol);
  /* check result */
  SCHEME_ASSERT((pointer != NULL), "dl-symbol: failed to resolve symbol");
  /* return result */
  return scheme_make_pointer(pointer);
}

#ifdef LIBDL_HAS_INFO
static Scheme_Value
libdl_address (int argc, Scheme_Value argv[])
{
  int res;
  void *address;
  Dl_info *info;
  Scheme_Value info_obj;

  /* check arguments */
  SCHEME_ASSERT ((argc == 1), "dl-address: wrong number of args");
  SCHEME_ASSERT (SCHEME_POINTERP(argv[0]), "dl-address: first arg must be a pointer");
  /* convert arguments */
  address = SCHEME_PTR_VAL(argv[0]);
  /* allocate result object */
  info_obj = scheme_alloc_object (libdl_info_type, sizeof(Dl_info));
  info = SCHEME_PTR_VAL(info_obj);
  /* perform the call */
  res = dladdr(address, info);
  /* check result */
  SCHEME_ASSERT((res != 0), "dl-address: failed to resolve address");
  /* return result */
  return info_obj;
}
#endif

/* accessors */

#ifdef LIBDL_HAS_INFO

static Scheme_Value
libdl_info_file_name (int argc, Scheme_Value argv[])
{
  Dl_info *info;

  /* check arguments */
  SCHEME_ASSERT ((argc == 1), "dl-info-file-name: wrong number of args");
  SCHEME_ASSERT (LIBDL_INFOP(argv[0]), "dl-info-file-name: first arg must be an info object");
  /* get type */
  info = SCHEME_PTR_VAL(argv[0]);
  /* return result */
  return (scheme_make_string(info->dli_fname));
}

static Scheme_Value
libdl_info_file_address (int argc, Scheme_Value argv[])
{
  Dl_info *info;

  /* check arguments */
  SCHEME_ASSERT ((argc == 1), "dl-info-file-address: wrong number of args");
  SCHEME_ASSERT (LIBDL_INFOP(argv[0]), "dl-info-file-address: first arg must be an info object");
  /* get type */
  info = SCHEME_PTR_VAL(argv[0]);
  /* return result */
  return (scheme_make_pointer(info->dli_fbase));
}

static Scheme_Value
libdl_info_symbol_name (int argc, Scheme_Value argv[])
{
  Dl_info *info;

  /* check arguments */
  SCHEME_ASSERT ((argc == 1), "dl-info-symbol-name: wrong number of args");
  SCHEME_ASSERT (LIBDL_INFOP(argv[0]), "dl-info-symbol-name: first arg must be an info object");
  /* get type */
  info = SCHEME_PTR_VAL(argv[0]);
  /* return result */
  return (scheme_make_string(info->dli_sname));
}

static Scheme_Value
libdl_info_symbol_address (int argc, Scheme_Value argv[])
{
  Dl_info *info;

  /* check arguments */
  SCHEME_ASSERT ((argc == 1), "dl-info-symbol-address: wrong number of args");
  SCHEME_ASSERT (LIBDL_INFOP(argv[0]), "dl-info-symbol-address: first arg must be an info object");
  /* get type */
  info = SCHEME_PTR_VAL(argv[0]);
  /* return result */
  return (scheme_make_pointer(info->dli_fbase));
}

#endif /* LIBDL_HAS_INFO */

