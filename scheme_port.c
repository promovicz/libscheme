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
#include <stdio.h>

#define HAS_STANDARD_IOB 1
/* #define HAS_GNU_IOB 1 */

struct Scheme_Indexed_String
{
  char *string;
  int size;
  int index;
};
typedef struct Scheme_Indexed_String Scheme_Indexed_String;

/* globals */
Scheme_Object *scheme_eof;
Scheme_Object *scheme_eof_type;
Scheme_Object *scheme_input_port_type, *scheme_output_port_type;
Scheme_Object *scheme_stdin_port;
Scheme_Object *scheme_stdout_port;
Scheme_Object *scheme_stderr_port;

/* locals */
static Scheme_Object *cur_in_port;
static Scheme_Object *cur_out_port;
static Scheme_Object *scheme_file_input_port_type;
static Scheme_Object *scheme_string_input_port_type;
static Scheme_Object *scheme_file_output_port_type;

/* generic ports */

static Scheme_Object *scheme_make_eof (void);
static Scheme_Object *call_with_input_file (int argc, Scheme_Object *argv[]);
static Scheme_Object *call_with_output_file (int argc, Scheme_Object *argv[]);
static Scheme_Object *input_port_p (int argc, Scheme_Object *argv[]);
static Scheme_Object *output_port_p (int argc, Scheme_Object *argv[]);
static Scheme_Object *current_input_port (int argc, Scheme_Object *argv[]);
static Scheme_Object *current_output_port (int argc, Scheme_Object *argv[]);
static Scheme_Object *with_input_from_file (int argc, Scheme_Object *argv[]);
static Scheme_Object *with_output_to_file (int argc, Scheme_Object *argv[]);
static Scheme_Object *open_input_file (int argc, Scheme_Object *argv[]);
static Scheme_Object *open_output_file (int argc, Scheme_Object *argv[]);
static Scheme_Object *close_input_port (int argc, Scheme_Object *argv[]);
static Scheme_Object *close_output_port (int argc, Scheme_Object *argv[]);
static Scheme_Object *read (int argc, Scheme_Object *argv[]);
static Scheme_Object *read_char (int argc, Scheme_Object *argv[]);
static Scheme_Object *peek_char (int argc, Scheme_Object *argv[]);
static Scheme_Object *eof_object_p (int argc, Scheme_Object *argv[]);
static Scheme_Object *char_ready_p (int argc, Scheme_Object *argv[]);
static Scheme_Object *write (int argc, Scheme_Object *argv[]);
static Scheme_Object *display (int argc, Scheme_Object *argv[]);
static Scheme_Object *newline (int argc, Scheme_Object *argv[]);
static Scheme_Object *write_char (int argc, Scheme_Object *argv[]);
static Scheme_Object *load (int argc, Scheme_Object *argv[]);
/* non-standard */
static Scheme_Object *flush_output (int argc, Scheme_Object *argv[]);
static Scheme_Object *with_input_from_string (int argc, Scheme_Object *argv[]);
static Scheme_Object *open_input_string (int argc, Scheme_Object *argv[]);

void 
scheme_init_port (Scheme_Env *env)
{
  scheme_eof_type = scheme_make_type ("<eof>");
  scheme_add_global ("<eof>", scheme_eof_type, env);
  scheme_eof = scheme_make_eof ();
  scheme_input_port_type = scheme_make_type ("<input-port>");
  scheme_file_input_port_type = scheme_make_type ("<file-input-port>");
  scheme_string_input_port_type = scheme_make_type ("<string-input-port>");
  scheme_file_output_port_type = scheme_make_type ("<file-output-port>");
  scheme_add_global ("<input-port>", scheme_input_port_type, env);
  scheme_output_port_type = scheme_make_type ("<output-port>");
  cur_in_port = scheme_stdin_port = scheme_make_file_input_port (stdin);
  cur_out_port = scheme_stdout_port = scheme_make_file_output_port (stdout);
  scheme_stderr_port = scheme_make_file_output_port (stderr);
  scheme_add_global ("<output-port>", scheme_output_port_type, env);
  scheme_add_global ("call-with-input-file", scheme_make_prim (call_with_input_file), env);
  scheme_add_global ("call-with-output-file", scheme_make_prim (call_with_output_file), env);
  scheme_add_global ("input-port?", scheme_make_prim (input_port_p), env);
  scheme_add_global ("output-port?", scheme_make_prim (output_port_p), env);
  scheme_add_global ("current-input-port", scheme_make_prim (current_input_port), env);
  scheme_add_global ("current-output-port", scheme_make_prim (current_output_port), env);
  scheme_add_global ("with-input-from-file", scheme_make_prim (with_input_from_file), env);
  scheme_add_global ("with-input-from-string", scheme_make_prim (with_input_from_string), env);
  scheme_add_global ("with-output-to-file", scheme_make_prim (with_output_to_file), env);
  scheme_add_global ("open-input-file", scheme_make_prim (open_input_file), env);
  scheme_add_global ("open-input-string", scheme_make_prim (open_input_string), env);
  scheme_add_global ("open-output-file", scheme_make_prim (open_output_file), env);
  scheme_add_global ("close-input-port", scheme_make_prim (close_input_port), env);
  scheme_add_global ("close-output-port", scheme_make_prim (close_output_port), env);
  scheme_add_global ("read", scheme_make_prim (read), env);
  scheme_add_global ("read-char", scheme_make_prim (read_char), env);
  scheme_add_global ("peek-char", scheme_make_prim (peek_char), env);
  scheme_add_global ("eof-object?", scheme_make_prim (eof_object_p), env);
  scheme_add_global ("char-ready?", scheme_make_prim (char_ready_p), env);
  scheme_add_global ("write", scheme_make_prim (write), env);
  scheme_add_global ("display", scheme_make_prim (display), env);
  scheme_add_global ("newline", scheme_make_prim (newline), env);
  scheme_add_global ("write-char", scheme_make_prim (write_char), env);
  scheme_add_global ("load", scheme_make_prim (load), env);
  scheme_add_global ("flush-output", scheme_make_prim (flush_output), env);
  scheme_add_global ("write-to-string", scheme_make_prim (write), env);
  scheme_add_global ("display-to-string", scheme_make_prim (display), env);
}

static Scheme_Object *
scheme_make_eof (void)
{
  Scheme_Object *eof;

  eof = scheme_alloc_object ();
  SCHEME_TYPE (eof) = scheme_eof_type;
  return (eof);
}

Scheme_Input_Port *
scheme_make_input_port (Scheme_Object *subtype,
			void *data,
			int (*getc_fun) (Scheme_Input_Port*),
			void (*ungetc_fun) (int, Scheme_Input_Port*),
			int (*char_ready_fun) (Scheme_Input_Port*),
			void (*close_fun) (Scheme_Input_Port*))
{
  Scheme_Input_Port *ip;

  ip = (Scheme_Input_Port *) scheme_malloc (sizeof (Scheme_Input_Port));
  ip->sub_type = subtype;
  ip->port_data = data;
  ip->getc_fun = getc_fun;
  ip->ungetc_fun = ungetc_fun;
  ip->char_ready_fun = char_ready_fun;
  ip->close_fun = close_fun;
  return (ip);
}

Scheme_Output_Port *
scheme_make_output_port (Scheme_Object *subtype,
			 void *data,
			 void (*write_string_fun) (char *str, Scheme_Output_Port*),
			 void (*close_fun) (Scheme_Output_Port*))
{
  Scheme_Output_Port *op;

  op = (Scheme_Output_Port *) scheme_malloc (sizeof (Scheme_Output_Port));
  op->sub_type = subtype;
  op->port_data = data;
  op->write_string_fun = write_string_fun;
  op->close_fun = close_fun;
  return (op);
}

int
scheme_getc (Scheme_Object *port)
{
  Scheme_Input_Port *ip;
  
  ip = (Scheme_Input_Port *) SCHEME_PTR_VAL (port);
  return ((ip->getc_fun) (ip));
}

void
scheme_ungetc (int ch, Scheme_Object *port)
{
  Scheme_Input_Port *ip;

  ip = (Scheme_Input_Port *) SCHEME_PTR_VAL (port);
  (ip->ungetc_fun) (ch, ip);
}

int
scheme_char_ready (Scheme_Object *port)
{
  Scheme_Input_Port *ip;

  ip = (Scheme_Input_Port *) SCHEME_PTR_VAL (port);
  return ((ip->char_ready_fun) (ip));
}

void
scheme_close_input_port (Scheme_Object *port)
{
  Scheme_Input_Port *ip;

  ip = (Scheme_Input_Port *) SCHEME_PTR_VAL (port);
  (ip->close_fun) (ip);
}

void
scheme_close_output_port (Scheme_Object *port)
{
  Scheme_Output_Port *op;

  op = (Scheme_Output_Port *) SCHEME_PTR_VAL (port);
  (op->close_fun) (op);
}

/* file input ports */

static int 
file_getc (Scheme_Input_Port *port)
{
  return (fgetc ((FILE *)port->port_data));
}

static void
file_ungetc (int ch, Scheme_Input_Port *port)
{
  ungetc (ch, (FILE *)port->port_data);
}

static int
file_char_ready (Scheme_Input_Port *port)
{
  FILE *fp = (FILE *) port->port_data;
#ifdef HAS_STANDARD_IOB  
  return (fp->_cnt);
#elif HAS_GNU_IOB
  return (fp->_egptr - fp->_gptr);
#else
  scheme_warning ("char-ready? always returns #f on this platform");
  return (scheme_false);
#endif
}

static void
file_close_input (Scheme_Input_Port *port)
{
  FILE *fp = (FILE *) port->port_data;
  fclose (fp);
}

Scheme_Object *
scheme_make_file_input_port (FILE *fp)
{
  Scheme_Object *port;
  Scheme_Input_Port *ip;

  port = scheme_alloc_object ();
  SCHEME_TYPE (port) = scheme_input_port_type;
  SCHEME_PTR_VAL (port) = 
    scheme_make_input_port (scheme_file_input_port_type,
			    fp,
			    file_getc,
			    file_ungetc,
			    file_char_ready,
			    file_close_input);
  return (port);
}

/* string input ports */

static int 
string_getc (Scheme_Input_Port *port)
{
  Scheme_Indexed_String *is;

  is = (Scheme_Indexed_String *) port->port_data;
  if (is->index >= is->size)
    {
      return (EOF);
    }
  else
    {
      return (is->string[is->index++]);
    }
}

static void
string_ungetc (int ch, Scheme_Input_Port *port)
{
  Scheme_Indexed_String *is;

  is = (Scheme_Indexed_String *) port->port_data;
  if (is->index > 0)
    {
      is->index = is->index - 1;
    }
}

static int
string_char_ready (Scheme_Input_Port *port)
{
  Scheme_Indexed_String *is;

  is = (Scheme_Indexed_String *) port->port_data;
  return (is->index < is->size);
}

static void
string_close (Scheme_Input_Port *port)
{
  return;
}

static Scheme_Indexed_String *
scheme_make_indexed_string (char *str)
{
  Scheme_Indexed_String *is;

  is = (Scheme_Indexed_String *) scheme_malloc (sizeof (Scheme_Indexed_String));
  is->string = scheme_strdup (str);
  is->size = strlen (str);
  is->index = 0;
  return (is);
}

Scheme_Object *
scheme_make_string_input_port (char *str)
{
  Scheme_Object *port;

  port = scheme_alloc_object ();
  SCHEME_TYPE (port) = scheme_input_port_type;
  SCHEME_PTR_VAL (port) =
    scheme_make_input_port (scheme_string_input_port_type,
			    scheme_make_indexed_string (str),
			    string_getc,
			    string_ungetc,
			    string_char_ready,
			    string_close);
  return (port);
}

/* file output ports */

static void
file_write_string (char *str, Scheme_Output_Port *port)
{
  FILE *fp = (FILE *) port->port_data;
  fprintf (fp, "%s", str);
}

static void
file_close_output (Scheme_Output_Port *port)
{
  FILE *fp = (FILE *) port->port_data;
  fclose (fp);
}

Scheme_Object *
scheme_make_file_output_port (FILE *fp)
{
  Scheme_Object *port;

  port = scheme_alloc_object ();
  SCHEME_TYPE(port) = scheme_output_port_type;
  SCHEME_PTR_VAL(port) = 
    scheme_make_output_port (scheme_file_output_port_type,
			     fp,
			     file_write_string,
			     file_close_output);
  return (port);
}

static Scheme_Object *
call_with_input_file (int argc, Scheme_Object *argv[])
{
  FILE *fp;
  char *filename;
  Scheme_Object *ret, *port;

  SCHEME_ASSERT ((argc == 2), "call-with-input-file: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP (argv[0]), 
		 "call-with-input-file: first arg must be a string");
  SCHEME_ASSERT (SCHEME_PROCP (argv[1]),
		 "call-with-input-file: second arg must be a procedure");
  filename = SCHEME_STR_VAL (argv[0]);
  fp = fopen (filename, "r");
  if (! fp)
    {
      scheme_signal_error ("cannot open file for input: %s", filename);
    }
  port = scheme_make_file_input_port (fp);
  ret = scheme_apply_to_list (argv[1], scheme_make_pair (port, scheme_null));
  fclose (fp);
  return (ret);
}

static Scheme_Object *
call_with_output_file (int argc, Scheme_Object *argv[])
{
  FILE *fp;
  char *filename;
  Scheme_Object *ret, *port;

  SCHEME_ASSERT ((argc == 2), "call-with-output-file: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP (argv[0]), 
		 "call-with-output-file: first arg must be a string");
  SCHEME_ASSERT (SCHEME_PROCP (argv[1]),
		 "call-with-output-file: second arg must be a procedure");
  filename = SCHEME_STR_VAL (argv[0]);
  fp = fopen (filename, "w");
  if (! fp)
    {
      scheme_signal_error ("cannot open file for output: %s", filename);
    }
  port = scheme_make_file_output_port (fp);
  ret = scheme_apply_to_list (argv[1], scheme_make_pair (port, scheme_null));
  fclose (fp);
  return (ret);
}

static Scheme_Object *
input_port_p (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "input-port?: wrong number of args");
  return (SCHEME_INPORTP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Object *
output_port_p (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "output-port?: wrong number of args");
  return (SCHEME_OUTPORTP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Object *
current_input_port (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 0), "current-input-port: wrong number of args");
  return (cur_in_port);
}

static Scheme_Object *
current_output_port (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 0), "current-output-port: wrong number of args");
  return (cur_out_port);
}

static Scheme_Object *
with_input_from_file (int argc, Scheme_Object *argv[])
{
  FILE *fp;
  char *filename;
  Scheme_Object *ret, *old_port, *new_port;

  SCHEME_ASSERT ((argc == 2), "with-input-from-file: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP (argv[0]), 
		 "with-input-from-file: first arg must be a string");
  SCHEME_ASSERT (SCHEME_PROCP (argv[1]),
		 "with-input-from-file: second arg must be a procedure");
  filename = SCHEME_STR_VAL (argv[0]);
  fp = fopen (filename, "r");
  if (! fp)
    {
      scheme_signal_error ("cannot open file for input: %s", filename);
    }
  new_port = scheme_make_file_input_port (fp);
  old_port = cur_in_port;
  cur_in_port = new_port;
  ret = scheme_apply (argv[1], 0, NULL);
  cur_in_port = old_port;
  scheme_close_input_port (new_port);
  return (ret);
}

static Scheme_Object *
with_input_from_string (int argc, Scheme_Object *argv[])
{
  char *str;
  Scheme_Object *ret, *old_port, *new_port;

  SCHEME_ASSERT ((argc == 2), "with-input-from-string: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP (argv[0]), 
		 "with-input-from-string: first arg must be a string");
  SCHEME_ASSERT (SCHEME_PROCP (argv[1]),
		 "with-input-from-file: second arg must be a procedure");
  str = SCHEME_STR_VAL (argv[0]);
  new_port = scheme_make_string_input_port (str);
  old_port = cur_in_port;
  cur_in_port = new_port;
  ret = scheme_apply (argv[1], 0, NULL);
  cur_in_port = old_port;
  scheme_close_input_port (new_port);
  return (ret);
}

static Scheme_Object *
with_output_to_file (int argc, Scheme_Object *argv[])
{
  FILE *fp, *old;
  char *filename;
  Scheme_Object *ret;

  SCHEME_ASSERT ((argc == 2), "with-output-to-file: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP (argv[0]), 
		 "with-output-to-file: first arg must be a string");
  SCHEME_ASSERT (SCHEME_PROCP (argv[1]),
		 "with-output-to-file: second arg must be a procedure");
  filename = SCHEME_STR_VAL (argv[0]);
  fp = fopen (filename, "w");
  if (! fp)
    {
      scheme_signal_error ("cannot open file for output: %s", filename);
    }
  old = (FILE *)SCHEME_PTR_VAL (cur_out_port);
  SCHEME_PTR_VAL (cur_out_port) = fp;
  ret = scheme_apply_to_list (argv[1], scheme_null);
  SCHEME_PTR_VAL (cur_out_port) = old;
  fclose (fp);
  return (ret);
}

static Scheme_Object *
open_input_file (int argc, Scheme_Object *argv[])
{
  FILE *fp;

  SCHEME_ASSERT ((argc == 1), "open-input-file: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "open-input-file: arg must be a filename");
  fp = fopen (SCHEME_STR_VAL(argv[0]), "r");
  if (!fp)
    {
      scheme_signal_error ("Cannot open input file %s", SCHEME_STR_VAL(argv[0]));
    }
  return (scheme_make_file_input_port (fp));
}

static Scheme_Object *
open_input_string (int argc, Scheme_Object *argv[])
{
  char *str;

  SCHEME_ASSERT ((argc == 1), "open-input-string: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "open-input-string: arg must be a string");
  str = SCHEME_STR_VAL (argv[0]);
  return (scheme_make_string_input_port (str));
}

static Scheme_Object *
open_output_file (int argc, Scheme_Object *argv[])
{
  FILE *fp;

  SCHEME_ASSERT ((argc == 1), "open-output-file: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "open-output-file: arg must be a filename");
  fp = fopen (SCHEME_STR_VAL(argv[0]), "w");
  if (!fp)
    {
      scheme_signal_error ("Cannot open output file %s", SCHEME_STR_VAL(argv[0]));
    }
  return (scheme_make_file_output_port (fp));
}

static Scheme_Object *
close_input_port (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "close-input-port: wrong number of args");
  SCHEME_ASSERT (SCHEME_INPORTP(argv[0]), "close-input-port: arg must be an input port");
  scheme_close_input_port (argv[0]);
  return (scheme_true);
}

static Scheme_Object *
close_output_port (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "close-output-port: wrong number of args");
  SCHEME_ASSERT (SCHEME_OUTPORTP(argv[0]), "close-output-port: arg must be an output port");
  scheme_close_output_port (argv[0]);
  return (scheme_true);
}

static Scheme_Object *
read (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc==0 || argc==1), "read: wrong number of args");
  if (argc == 1)
    {
      SCHEME_ASSERT (SCHEME_INPORTP(argv[0]), "read: arg must be an input port");
      return (scheme_read (argv[0]));
    }
  else
    {
      return (scheme_read (cur_in_port));
    }
}

static Scheme_Object *
read_char (int argc, Scheme_Object *argv[])
{
  int ch;

  SCHEME_ASSERT ((argc==0 || argc==1), "read-char: wrong number of args");
  if (argc == 1)
    {
      SCHEME_ASSERT (SCHEME_INPORTP(argv[0]), "read-char: arg must be an input port");
      ch = scheme_getc (argv[0]);
    }
  else
    {
      ch = scheme_getc (cur_in_port);
    }
  if (ch == EOF)
    {
      return (scheme_eof);
    }
  else
    {
      return (scheme_make_char (ch));
    }
}

static Scheme_Object *
peek_char (int argc, Scheme_Object *argv[])
{
  Scheme_Object *port;
  int ch;

  SCHEME_ASSERT ((argc==0 || argc==1), "peek-char: wrong number of args");
  if (argc == 1)
    {
      SCHEME_ASSERT (SCHEME_INPORTP(argv[0]), "peek-char: arg must be an input port");
      port = argv[0];
    }
  else
    {
      port = cur_in_port;
    }
  ch = scheme_getc (port);
  if (ch == EOF)
    {
      return (scheme_eof);
    }
  else
    {
      scheme_ungetc (ch, port);
      return (scheme_make_char (ch));
    }
}

static Scheme_Object *
eof_object_p (int argc, Scheme_Object *argv[])
{
  SCHEME_ASSERT ((argc == 1), "eof-object?: wrong number of args");
  return (SCHEME_EOFP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Object *
char_ready_p (int argc, Scheme_Object *argv[])
{
  Scheme_Object *port;

  SCHEME_ASSERT ((argc==0 || argc==1), "char-ready?: wrong number of args");
  if (argc == 1)
    {
      SCHEME_ASSERT (SCHEME_INPORTP(argv[0]), "char-ready?: arg must be an input port");
      port = argv[0];
    }
  else
    {
      port = cur_in_port;
    }
  if (scheme_char_ready (port))
    {
      return (scheme_true);
    }
  else
    {
      return (scheme_false);
    }
}

static Scheme_Object *
write (int argc, Scheme_Object *argv[])
{
  Scheme_Object *port;
  
  SCHEME_ASSERT ((argc==1 || argc==2), "write: wrong number of args");
  if (argc == 2)
    {
      SCHEME_ASSERT (SCHEME_OUTPORTP(argv[1]), "write: second arg must be an output port");
      port = argv[1];
    }
  else
    {
      port = cur_out_port;
    }
  scheme_write (argv[0], port);
  return (scheme_true);
}

static Scheme_Object *
display (int argc, Scheme_Object *argv[])
{
  Scheme_Object *port;
  
  SCHEME_ASSERT ((argc==1 || argc==2), "display: wrong number of args");
  if (argc == 2)
    {
      SCHEME_ASSERT (SCHEME_OUTPORTP(argv[1]), "display: second arg must be an output port");
      port = argv[1];
    }
  else
    {
      port = cur_out_port;
    }
  scheme_display (argv[0], port);
  return (scheme_true);
}

static Scheme_Object *
newline (int argc, Scheme_Object *argv[])
{
  Scheme_Object *port;
  
  SCHEME_ASSERT ((argc==0 || argc==1), "newline: wrong number of args");
  if (argc == 1)
    {
      SCHEME_ASSERT (SCHEME_OUTPORTP(argv[0]), "newline: arg must be an output port");
      port = argv[0];
    }
  else
    {
      port = cur_out_port;
    }
  scheme_write_string ("\n", port);
  return (scheme_true);
}

static Scheme_Object *
write_char (int argc, Scheme_Object *argv[])
{
  Scheme_Object *port;
  char buf[256];
  
  SCHEME_ASSERT ((argc==1 || argc==2), "write-char: wrong number of args");
  if (argc == 2)
    {
      SCHEME_ASSERT (SCHEME_OUTPORTP(argv[1]), "write-char: second arg must be an output port");
      port = argv[1];
    }
  else
    {
      port = cur_out_port;
    }
  SCHEME_ASSERT (SCHEME_CHARP(argv[0]), "write-char: first arg must be a character");
  sprintf (buf, "%c", SCHEME_CHAR_VAL (argv[0]));
  scheme_write_string (buf, port);
  return (scheme_true);
}

static Scheme_Object *
load (int argc, Scheme_Object *argv[])
{
  Scheme_Object *obj, *ret, *port;
  char *filename;
  FILE *fp;

  SCHEME_ASSERT ((argc == 1), "load: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP (argv[0]), "load: arg must be a filename (string)");
  filename = SCHEME_STR_VAL (argv[0]);
  printf ("; loading %s\n", filename);
  fp = fopen (filename, "r");
  if (! fp)
    {
      scheme_signal_error ("load: could not open file for input: %s", filename);
    }
  /* skip `#!' to end of line if possible */
  fscanf (fp, "#!%*s\n");
  /* now read all expressions */
  port = scheme_make_file_input_port (fp);
  while ((obj = scheme_read (port)) != scheme_eof)
    {
      ret = scheme_eval (obj, scheme_env);
    }
  printf ("; done loading %s\n", filename);
  fclose (fp);
  return (ret);
}

static Scheme_Object *
flush_output (int argc, Scheme_Object *argv[])
{
#if 0
  SCHEME_ASSERT ((argc == 1), "flush-output: wrong number of args");
  SCHEME_ASSERT (SCHEME_OUTPORTP (argv[0]), "flush-output: arg must be an output port");
  fflush ((FILE *)SCHEME_PTR_VAL (argv[0]));
#endif
  scheme_warning ("flush-output temporarily disabled");
  return (scheme_true);
}

static Scheme_Object *
write_to_string (int argc, Scheme_Object *argv[])
{
  char *str;
  
  SCHEME_ASSERT ((argc == 1), "write-to-string: wrong number of args");
  str = scheme_write_to_string (argv[0]);
  return (scheme_make_string (str));
}

static Scheme_Object *
display_to_string (int argc, Scheme_Object *argv[])
{
  char *str;
  
  SCHEME_ASSERT ((argc == 1), "display-to-string: wrong number of args");
  str = scheme_display_to_string (argv[0]);
  return (scheme_make_string (str));
}

