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

#include "scheme_private.h"
#include <string.h>
#include <stdio.h>

/* globals */
Scheme_Value scheme_eof;
Scheme_Value scheme_eof_type;
Scheme_Value scheme_input_port_type;
Scheme_Value scheme_output_port_type;
Scheme_Value scheme_stdin_port;
Scheme_Value scheme_stdout_port;
Scheme_Value scheme_stderr_port;

/* locals */
static Scheme_Value cur_in_port;
static Scheme_Value cur_out_port;

/* static function declarations */
static Scheme_Value eof_object_p (int argc, Scheme_Value argv[]);
static Scheme_Value input_port_p (int argc, Scheme_Value argv[]);
static Scheme_Value output_port_p (int argc, Scheme_Value argv[]);
static Scheme_Value open_input_file (int argc, Scheme_Value argv[]);
static Scheme_Value open_output_file (int argc, Scheme_Value argv[]);
static Scheme_Value open_input_string (int argc, Scheme_Value argv[]);
static Scheme_Value open_output_string (int argc, Scheme_Value argv[]);
static Scheme_Value close_input_port (int argc, Scheme_Value argv[]);
static Scheme_Value close_output_port (int argc, Scheme_Value argv[]);
static Scheme_Value current_input_port (int argc, Scheme_Value argv[]);
static Scheme_Value current_output_port (int argc, Scheme_Value argv[]);
static Scheme_Value call_with_input_file (int argc, Scheme_Value argv[]);
static Scheme_Value call_with_output_file (int argc, Scheme_Value argv[]);
static Scheme_Value with_input_from_file (int argc, Scheme_Value argv[]);
static Scheme_Value with_output_to_file (int argc, Scheme_Value argv[]);
static Scheme_Value read (int argc, Scheme_Value argv[]);
static Scheme_Value read_char (int argc, Scheme_Value argv[]);
static Scheme_Value read_line (int argc, Scheme_Value argv[]);
static Scheme_Value peek_char (int argc, Scheme_Value argv[]);
static Scheme_Value char_ready_p (int argc, Scheme_Value argv[]);
static Scheme_Value write (int argc, Scheme_Value argv[]);
static Scheme_Value display (int argc, Scheme_Value argv[]);
static Scheme_Value newline (int argc, Scheme_Value argv[]);
static Scheme_Value write_char (int argc, Scheme_Value argv[]);
static Scheme_Value load (int argc, Scheme_Value argv[]);
static Scheme_Value read_from_string (int argc, Scheme_Value argv[]);
static Scheme_Value write_to_string (int argc, Scheme_Value argv[]);
static Scheme_Value display_to_string (int argc, Scheme_Value argv[]);
/* non-standard */
static Scheme_Value drain_input (int argc, Scheme_Value argv[]);
static Scheme_Value flush_output (int argc, Scheme_Value argv[]);
static Scheme_Value port_string (int argc, Scheme_Value argv[]);
//static Scheme_Value with_input_from_string (int argc, Scheme_Value argv[]);
//static Scheme_Value open_input_string (int argc, Scheme_Value argv[]);

/* internal functions */
static Scheme_Value get_port_string(Scheme_Value port);

/* exported functions */

void
scheme_init_port (Scheme_Env *env)
{
  /* end-of-file object */
  scheme_eof_type = scheme_make_type ("<eof>");
  scheme_add_global ("<eof>", scheme_eof_type, env);
  scheme_add_prim ("eof-object?", eof_object_p, env);
  scheme_eof = scheme_alloc_object(scheme_eof_type, 0);

  /* port types */
  scheme_input_port_type = scheme_make_type ("<input-port>");
  scheme_output_port_type = scheme_make_type ("<output-port>");
  scheme_add_global ("<input-port>", scheme_input_port_type, env);
  scheme_add_prim ("input-port?", input_port_p, env);
  scheme_add_global ("<output-port>", scheme_output_port_type, env);
  scheme_add_prim ("output-port?", output_port_p, env);

  /* opening and closing */
  scheme_add_prim ("open-input-file", open_input_file, env);
  scheme_add_prim ("open-output-file", open_output_file, env);
  scheme_add_prim ("open-input-string", open_input_string, env);
  scheme_add_prim ("open-output-string", open_output_string, env);
  scheme_add_prim ("close-input-port", close_input_port, env);
  scheme_add_prim ("close-output-port", close_output_port, env);

  /* current port */
  scheme_add_prim ("current-input-port", current_input_port, env);
  scheme_add_prim ("current-output-port", current_output_port, env);
  scheme_add_prim ("call-with-input-file", call_with_input_file, env);
  scheme_add_prim ("call-with-output-file", call_with_output_file, env);
  scheme_add_prim ("with-input-from-file", with_input_from_file, env);
  scheme_add_prim ("with-output-to-file", with_output_to_file, env);

  /* port operations */
  scheme_add_prim ("read", read, env);
  scheme_add_prim ("read-char", read_char, env);
  scheme_add_prim ("read-line", read_line, env);
  scheme_add_prim ("peek-char", peek_char, env);
  scheme_add_prim ("char-ready?", char_ready_p, env);
  scheme_add_prim ("write", write, env);
  scheme_add_prim ("display", display, env);
  scheme_add_prim ("newline", newline, env);
  scheme_add_prim ("write-char", write_char, env);
  scheme_add_prim ("load", load, env);

  /* reading to/from strings */
  scheme_add_prim ("read-from-string", read_from_string, env);
  scheme_add_prim ("write-to-string", write_to_string, env);
  scheme_add_prim ("display-to-string", display_to_string, env);

  /* buffering */
  scheme_add_prim ("drain-input", drain_input, env);
  scheme_add_prim ("flush-output", flush_output, env);

  /* string ports */
  scheme_add_prim ("port-string", port_string, env);

  /* standard ports */
  cur_in_port = scheme_stdin_port = scheme_make_input_port (stdin);
  cur_out_port = scheme_stdout_port = scheme_make_output_port (stdout);
  scheme_stderr_port = scheme_make_output_port (stderr);
}

Scheme_Value
scheme_make_input_port (FILE *stream)
{
  Scheme_Value obj;
  Scheme_Port *ip;

  obj = scheme_alloc_object(scheme_input_port_type, sizeof(Scheme_Port));
  ip = (Scheme_Port *) SCHEME_PTR_VAL(obj);
  ip->stream = stream;
  return (obj);
}

Scheme_Value
scheme_make_output_port (FILE *stream)
{
  Scheme_Value obj;
  Scheme_Port *op;

  obj = scheme_alloc_object(scheme_output_port_type, sizeof(Scheme_Port));
  op = (Scheme_Port *) SCHEME_PTR_VAL(obj);
  op->stream = stream;
  return (obj);
}

Scheme_Value
scheme_make_string_input_port(const char *buf, size_t len)
{
  FILE *is = fmemopen((char*)buf, len, "r");
  Scheme_Value port = scheme_make_input_port(is);
  Scheme_Port *p = (Scheme_Port *)SCHEME_PTR_VAL(port);
  p->buf = (char*)buf;
  p->len = len;
  return port;
}

Scheme_Value
scheme_make_string_output_port(size_t maxlen)
{
  char *buf = scheme_calloc(1, maxlen + 1);
  FILE *is = fmemopen(buf, maxlen, "w");
  Scheme_Value port = scheme_make_output_port(is);
  Scheme_Port *p = (Scheme_Port *)SCHEME_PTR_VAL(port);
  p->buf = buf;
  p->len = maxlen;
  return port;
}

void
scheme_close_input_port (Scheme_Value port)
{
  Scheme_Port *ip;

  ip = (Scheme_Port *) SCHEME_PTR_VAL (port);
  if(ip->stream) fclose(ip->stream);
  ip->stream = NULL;
}

void
scheme_close_output_port (Scheme_Value port)
{
  Scheme_Port *op;

  op = (Scheme_Port *) SCHEME_PTR_VAL (port);
  if(op->stream) fclose(op->stream);
  op->stream = NULL;
}

int
scheme_getc (Scheme_Value port)
{
  Scheme_Port *ip;

  ip = (Scheme_Port *) SCHEME_PTR_VAL (port);
  return fgetc(ip->stream);
}

void
scheme_ungetc (int ch, Scheme_Value port)
{
  Scheme_Port *ip;

  ip = (Scheme_Port *) SCHEME_PTR_VAL (port);
  ungetc(ch, ip->stream);
}

void
scheme_puts (char *str, Scheme_Value port)
{
  Scheme_Port *op;

  op = (Scheme_Port *) SCHEME_PTR_VAL (port);
  fputs(str, op->stream);
}

/* static functions */

static Scheme_Value
eof_object_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "eof-object?: wrong number of args");
  return (SCHEME_EOFP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Value
call_with_input_file (int argc, Scheme_Value argv[])
{
  FILE *fp;
  char *filename;
  Scheme_Value ret, port;

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
  port = scheme_make_input_port (fp);
  ret = scheme_apply_to_list (argv[1], scheme_make_pair (port, scheme_null));
  fclose (fp);
  return (ret);
}

static Scheme_Value
call_with_output_file (int argc, Scheme_Value argv[])
{
  FILE *fp;
  char *filename;
  Scheme_Value ret, port;

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
  port = scheme_make_output_port (fp);
  ret = scheme_apply_to_list (argv[1], scheme_make_pair (port, scheme_null));
  fclose (fp);
  return (ret);
}

static Scheme_Value
input_port_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "input-port?: wrong number of args");
  return (SCHEME_INPORTP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Value
output_port_p (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "output-port?: wrong number of args");
  return (SCHEME_OUTPORTP(argv[0]) ? scheme_true : scheme_false);
}

static Scheme_Value
current_input_port (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 0), "current-input-port: wrong number of args");
  return (cur_in_port);
}

static Scheme_Value
current_output_port (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 0), "current-output-port: wrong number of args");
  return (cur_out_port);
}

static Scheme_Value
with_input_from_file (int argc, Scheme_Value argv[])
{
  FILE *fp;
  char *filename;
  Scheme_Value ret, old_port, new_port;

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
  new_port = scheme_make_input_port (fp);
  old_port = cur_in_port;
  cur_in_port = new_port;
  ret = scheme_apply (argv[1], 0, NULL);
  cur_in_port = old_port;
  scheme_close_input_port (new_port);
  return (ret);
}

static Scheme_Value
with_output_to_file (int argc, Scheme_Value argv[])
{
  FILE *fp, *old;
  char *filename;
  Scheme_Value ret;

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

static Scheme_Value
open_input_file (int argc, Scheme_Value argv[])
{
  FILE *fp;

  SCHEME_ASSERT ((argc == 1), "open-input-file: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "open-input-file: arg must be a filename");
  fp = fopen (SCHEME_STR_VAL(argv[0]), "r");
  if (!fp)
    {
      scheme_signal_error ("Cannot open input file %s", SCHEME_STR_VAL(argv[0]));
    }
  return (scheme_make_input_port (fp));
}

static Scheme_Value
open_output_file (int argc, Scheme_Value argv[])
{
  FILE *fp;

  SCHEME_ASSERT ((argc == 1), "open-output-file: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "open-output-file: arg must be a filename");
  fp = fopen (SCHEME_STR_VAL(argv[0]), "w");
  if (!fp)
    {
      scheme_signal_error ("Cannot open output file %s", SCHEME_STR_VAL(argv[0]));
    }
  return (scheme_make_output_port (fp));
}

static Scheme_Value
open_input_string (int argc, Scheme_Value argv[])
{
  char *s;
  size_t l;

  SCHEME_ASSERT ((argc == 1), "open-input-string: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "open-input-string: arg must be a string");

  s = SCHEME_STR_VAL(argv[0]);
  l = strlen(s);

  return (scheme_make_string_input_port (s, l));
}

static Scheme_Value
open_output_string (int argc, Scheme_Value argv[])
{
  size_t max = 2^16; // XXX

  SCHEME_ASSERT ((argc == 0 || argc == 1), "open-output-string: wrong number of args");
  if(argc == 1) {
    SCHEME_ASSERT (SCHEME_INTP(argv[0]), "open-output-string: arg must be an integer");
    max = SCHEME_INT_VAL(argv[0]);
  }
  return (scheme_make_string_output_port (max));
}

static Scheme_Value
close_input_port (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "close-input-port: wrong number of args");
  SCHEME_ASSERT (SCHEME_INPORTP(argv[0]), "close-input-port: arg must be an input port");
  scheme_close_input_port (argv[0]);
  return (scheme_true);
}

static Scheme_Value
close_output_port (int argc, Scheme_Value argv[])
{
  SCHEME_ASSERT ((argc == 1), "close-output-port: wrong number of args");
  SCHEME_ASSERT (SCHEME_OUTPORTP(argv[0]), "close-output-port: arg must be an output port");
  scheme_close_output_port (argv[0]);
  return (scheme_true);
}

static Scheme_Value
read (int argc, Scheme_Value argv[])
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

static Scheme_Value
read_char (int argc, Scheme_Value argv[])
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

static Scheme_Value
read_line (int argc, Scheme_Value argv[])
{
  Scheme_Value port, res;
  Scheme_Port *ip;
  char *s = NULL;
  size_t l = 0;

  SCHEME_ASSERT ((argc==0 || argc==1), "read-line: wrong number of args");
  if (argc == 1)
    {
      SCHEME_ASSERT (SCHEME_INPORTP(argv[0]), "read-line: arg must be an input port");
      port = argv[0];
    }
  else
    {
      port = cur_in_port;
    }

  ip = (Scheme_Port *) SCHEME_PTR_VAL (port);
  if(getline(&s, &l, ip->stream) == -1) {
    scheme_signal_error("read-line: read error");
  }
  res = scheme_make_string(s);
  free(s);
  return res;
}

static Scheme_Value
peek_char (int argc, Scheme_Value argv[])
{
  Scheme_Value port;
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

static Scheme_Value
char_ready_p (int argc, Scheme_Value argv[])
{
  Scheme_Value port;

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

  scheme_signal_error("char-ready?: not implemented");
}

static Scheme_Value
write (int argc, Scheme_Value argv[])
{
  Scheme_Value port;

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

static Scheme_Value
display (int argc, Scheme_Value argv[])
{
  Scheme_Value port;

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

static Scheme_Value
newline (int argc, Scheme_Value argv[])
{
  Scheme_Value port;

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
  scheme_puts ("\n", port);
  return (scheme_true);
}

static Scheme_Value
write_char (int argc, Scheme_Value argv[])
{
  Scheme_Value port;
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
  scheme_puts (buf, port);
  return (scheme_true);
}

static Scheme_Value
load (int argc, Scheme_Value argv[])
{
  Scheme_Value obj, ret = scheme_null, port;
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
  port = scheme_make_input_port (fp);
  while ((obj = scheme_read (port)) != scheme_eof)
    {
      ret = scheme_eval (obj, scheme_env);
    }
  printf ("; done loading %s\n", filename);
  fclose (fp);
  return (ret);
}

static Scheme_Value
read_from_string (int argc, Scheme_Value argv[])
{
  Scheme_Value port;
  char *str;
  size_t len;

  SCHEME_ASSERT ((argc==1), "read-from-string: wrong number of args");
  SCHEME_ASSERT (SCHEME_STRINGP(argv[0]), "read-from-string: arg must be a string");

  str = SCHEME_STR_VAL(argv[0]);
  len = strlen(str);
  port = scheme_make_string_input_port(str, len);

  return (scheme_read (port));
}

static Scheme_Value
write_to_string (int argc, Scheme_Value argv[])
{
  Scheme_Value port;

  SCHEME_ASSERT ((argc==1), "write-to-string: wrong number of args");
  port = scheme_make_string_output_port(2^16); // XXX
  scheme_write (argv[0], port);
  scheme_close_output_port(port);

  return get_port_string(port);
}

static Scheme_Value
display_to_string (int argc, Scheme_Value argv[])
{
  Scheme_Value port;

  SCHEME_ASSERT ((argc==1), "display-to-string: wrong number of args");
  port = scheme_make_string_output_port(2^16); // XXX
  scheme_display (argv[0], port);
  scheme_close_output_port(port);

  return get_port_string(port);
}

static Scheme_Value
drain_input (int argc, Scheme_Value argv[])
{
  Scheme_Value port;
  Scheme_Port *ip;

  SCHEME_ASSERT ((argc == 0 || argc == 1), "drain-input: wrong number of args");
  if (argc == 1)
    {
      SCHEME_ASSERT (SCHEME_INPORTP(argv[0]), "drain-input: arg must be an input port");
      port = argv[0];
    }
  else
    {
      port = cur_in_port;
    }

  ip = (Scheme_Port *) SCHEME_PTR_VAL (port);
  fflush(ip->stream);

  return (scheme_true);
}

static Scheme_Value
flush_output (int argc, Scheme_Value argv[])
{
  Scheme_Value port;
  Scheme_Port *op;

  SCHEME_ASSERT ((argc == 0 || argc == 1), "flush-output: wrong number of args");
  if (argc == 1)
    {
      SCHEME_ASSERT (SCHEME_OUTPORTP(argv[0]), "flush-output: arg must be an output port");
      port = argv[0];
    }
  else
    {
      port = cur_out_port;
    }

  op = (Scheme_Port *) SCHEME_PTR_VAL (port);
  fflush(op->stream);

  return (scheme_true);
}

static Scheme_Value
port_string (int argc, Scheme_Value argv[])
{
  Scheme_Value port;
  Scheme_Port *p;

  SCHEME_ASSERT ((argc == 1), "port-string: wrong number of args");
  SCHEME_ASSERT (SCHEME_OUTPORTP(argv[0]) || SCHEME_INPORTP(argv[0]), "port-string: arg must be a port");
  port = argv[0];
  p = (Scheme_Port *) SCHEME_PTR_VAL (port);
  SCHEME_ASSERT ((p->buf != NULL), "port-string: arg must be a string port");

  return get_port_string(port);
}

/* internal functions */

static Scheme_Value
get_port_string(Scheme_Value port)
{
  Scheme_Port *p;
  p = (Scheme_Port *) SCHEME_PTR_VAL (port);
  return scheme_make_string(p->buf);
}
