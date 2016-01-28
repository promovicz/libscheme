/* 
 * Copyright 1988, 1989 Hans-J. Boehm, Alan J. Demers
 * Copyright (c) 1991-1994 by Xerox Corporation.  All rights reserved.
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to use or copy this program
 * for any purpose,  provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 */
/* Boehm, July 11, 1994 4:41 pm PDT */


#include <stdio.h>
#include <signal.h>
#ifdef SOLARIS_THREADS
# include <sys/syscall.h>
#endif

#define I_HIDE_POINTERS	/* To make GC_call_with_alloc_lock visible */
#include "gc_priv.h"

# ifdef THREADS
#   ifdef PCR
#     include "il/PCR_IL.h"
      PCR_Th_ML GC_allocate_ml;
#   else
#     ifdef SRC_M3
	/* Critical section counter is defined in the M3 runtime 	*/
	/* That's all we use.						*/
#     else
#	ifdef SOLARIS_THREADS
	  mutex_t GC_allocate_ml;	/* Implicitly initialized.	*/
#	else
	  --> declare allocator lock here
#	endif
#     endif
#   endif
# endif

GC_FAR struct _GC_arrays GC_arrays = { 0 };


bool GC_debugging_started = FALSE;
	/* defined here so we don't have to load debug_malloc.o */

void (*GC_check_heap)() = (void (*)())0;

ptr_t GC_stackbottom = 0;

bool GC_dont_gc = 0;

bool GC_quiet = 0;

extern signed_word GC_mem_found;

# ifdef MERGE_SIZES
    /* Set things up so that GC_size_map[i] >= words(i),		*/
    /* but not too much bigger						*/
    /* and so that size_map contains relatively few distinct entries 	*/
    /* This is stolen from Russ Atkinson's Cedar quantization		*/
    /* alogrithm (but we precompute it).				*/


    void GC_init_size_map()
    {
	register unsigned i;

	/* Map size 0 to 1.  This avoids problems at lower levels. */
	  GC_size_map[0] = 1;
	/* One word objects don't have to be 2 word aligned.	   */
	  for (i = 1; i < sizeof(word); i++) {
	      GC_size_map[i] = 1;
	  }
	  GC_size_map[sizeof(word)] = ROUNDED_UP_WORDS(sizeof(word));
	for (i = sizeof(word) + 1; i <= 8 * sizeof(word); i++) {
#           ifdef ALIGN_DOUBLE
	      GC_size_map[i] = (ROUNDED_UP_WORDS(i) + 1) & (~1);
#           else
	      GC_size_map[i] = ROUNDED_UP_WORDS(i);
#           endif
	}
	for (i = 8*sizeof(word) + 1; i <= 16 * sizeof(word); i++) {
	      GC_size_map[i] = (ROUNDED_UP_WORDS(i) + 1) & (~1);
	}
	/* We leave the rest of the array to be filled in on demand. */
    }
    
    /* Fill in additional entries in GC_size_map, including the ith one */
    /* We assume the ith entry is currently 0.				*/
    /* Note that a filled in section of the array ending at n always    */
    /* has length at least n/4.						*/
    void GC_extend_size_map(i)
    word i;
    {
        word orig_word_sz = ROUNDED_UP_WORDS(i);
        word word_sz = orig_word_sz;
    	register word byte_sz = WORDS_TO_BYTES(word_sz);
    				/* The size we try to preserve.		*/
    				/* Close to to i, unless this would	*/
    				/* introduce too many distinct sizes.	*/
    	word smaller_than_i = byte_sz - (byte_sz >> 3);
    	word much_smaller_than_i = byte_sz - (byte_sz >> 2);
    	register word low_limit;	/* The lowest indexed entry we 	*/
    					/* initialize.			*/
    	register word j;
    	
    	if (GC_size_map[smaller_than_i] == 0) {
    	    low_limit = much_smaller_than_i;
    	    while (GC_size_map[low_limit] != 0) low_limit++;
    	} else {
    	    low_limit = smaller_than_i + 1;
    	    while (GC_size_map[low_limit] != 0) low_limit++;
    	    word_sz = ROUNDED_UP_WORDS(low_limit);
    	    word_sz += word_sz >> 3;
    	    if (word_sz < orig_word_sz) word_sz = orig_word_sz;
    	}
#	ifdef ALIGN_DOUBLE
	    word_sz += 1;
	    word_sz &= ~1;
#	endif
	if (word_sz > MAXOBJSZ) {
	    word_sz = MAXOBJSZ;
	}
    	byte_sz = WORDS_TO_BYTES(word_sz);
#	ifdef ADD_BYTE_AT_END
	    /* We need one extra byte; don't fill in GC_size_map[byte_sz] */
	    byte_sz--;
#	endif

    	for (j = low_limit; j <= byte_sz; j++) GC_size_map[j] = word_sz;  
    }
# endif


/*
 * The following is a gross hack to deal with a problem that can occur
 * on machines that are sloppy about stack frame sizes, notably SPARC.
 * Bogus pointers may be written to the stack and not cleared for
 * a LONG time, because they always fall into holes in stack frames
 * that are not written.  We partially address this by clearing
 * sections of the stack whenever we get control.
 */
word GC_stack_last_cleared = 0;	/* GC_no when we last did this */
# define CLEAR_SIZE 213
# define DEGRADE_RATE 50

word GC_min_sp;		/* Coolest stack pointer value from which we've */
			/* already cleared the stack.			*/
			
# ifdef STACK_GROWS_DOWN
#   define COOLER_THAN >
#   define HOTTER_THAN <
#   define MAKE_COOLER(x,y) if ((word)(x)+(y) > (word)(x)) {(x) += (y);} \
			    else {(x) = (word)ONES;}
#   define MAKE_HOTTER(x,y) (x) -= (y)
# else
#   define COOLER_THAN <
#   define HOTTER_THAN >
#   define MAKE_COOLER(x,y) if ((word)(x)-(y) < (word)(x)) {(x) -= (y);} else {(x) = 0;}
#   define MAKE_HOTTER(x,y) (x) += (y)
# endif

word GC_high_water;
			/* "hottest" stack pointer value we have seen	*/
			/* recently.  Degrades over time.		*/

word GC_stack_upper_bound()
{
    word dummy;
    
    return((word)(&dummy));
}

word GC_words_allocd_at_reset;

#if defined(ASM_CLEAR_CODE) && !defined(THREADS)
  extern ptr_t GC_clear_stack_inner();
#endif  

#if !defined(ASM_CLEAR_CODE) && !defined(THREADS)
/* Clear the stack up to about limit.  Return arg. */
/*ARGSUSED*/
ptr_t GC_clear_stack_inner(arg, limit)
ptr_t arg;
word limit;
{
    word dummy[CLEAR_SIZE];
    
    BZERO(dummy, CLEAR_SIZE*sizeof(word));
    if ((word)(dummy) COOLER_THAN limit) {
        (void) GC_clear_stack_inner(arg, limit);
    }
    /* Make sure the recursive call is not a tail call, and the bzero	*/
    /* call is not recognized as dead code.				*/
    GC_noop(dummy);
    return(arg);
}
#endif


/* Clear some of the inaccessible part of the stack.  Returns its	*/
/* argument, so it can be used in a tail call position, hence clearing  */
/* another frame.							*/
ptr_t GC_clear_stack(arg)
ptr_t arg;
{
    register word sp = GC_stack_upper_bound();
    register word limit;
#   ifdef THREADS
        word dummy[CLEAR_SIZE];;
#   endif
    
#   define SLOP 400
	/* Extra bytes we clear every time.  This clears our own	*/
	/* activation record, and should cause more frequent		*/
	/* clearing near the cold end of the stack, a good thing.	*/
#   define GC_SLOP 4000
	/* We make GC_high_water this much hotter than we really saw   	*/
	/* saw it, to cover for GC noise etc. above our current frame.	*/
#   define CLEAR_THRESHOLD 100000
	/* We restart the clearing process after this many bytes of	*/
	/* allocation.  Otherwise very heavily recursive programs	*/
	/* with sparse stacks may result in heaps that grow almost	*/
	/* without bounds.  As the heap gets larger, collection 	*/
	/* frequency decreases, thus clearing frequency would decrease, */
	/* thus more junk remains accessible, thus the heap gets	*/
	/* larger ...							*/
# ifdef THREADS
    BZERO(dummy, CLEAR_SIZE*sizeof(word));
# else
    if (GC_gc_no > GC_stack_last_cleared) {
        /* Start things over, so we clear the entire stack again */
        if (GC_stack_last_cleared == 0) GC_high_water = (word) GC_stackbottom;
        GC_min_sp = GC_high_water;
        GC_stack_last_cleared = GC_gc_no;
        GC_words_allocd_at_reset = GC_words_allocd;
    }
    /* Adjust GC_high_water */
        MAKE_COOLER(GC_high_water, WORDS_TO_BYTES(DEGRADE_RATE) + GC_SLOP);
        if (sp HOTTER_THAN GC_high_water) {
            GC_high_water = sp;
        }
        MAKE_HOTTER(GC_high_water, GC_SLOP);
    limit = GC_min_sp;
    MAKE_HOTTER(limit, SLOP);
    if (sp COOLER_THAN limit) {
        limit &= ~0xf;	/* Make it sufficiently aligned for assembly	*/
        		/* implementations of GC_clear_stack_inner.	*/
        GC_min_sp = sp;
        return(GC_clear_stack_inner(arg, limit));
    } else if (WORDS_TO_BYTES(GC_words_allocd - GC_words_allocd_at_reset)
    	       > CLEAR_THRESHOLD) {
    	/* Restart clearing process, but limit how much clearing we do. */
    	GC_min_sp = sp;
    	MAKE_HOTTER(GC_min_sp, CLEAR_THRESHOLD/4);
    	if (GC_min_sp HOTTER_THAN GC_high_water) GC_min_sp = GC_high_water;
    	GC_words_allocd_at_reset = GC_words_allocd;
    }  
# endif
  return(arg);
}


/* Return a pointer to the base address of p, given a pointer to a	*/
/* an address within an object.  Return 0 o.w.				*/
# ifdef __STDC__
    extern_ptr_t GC_base(extern_ptr_t p)
# else
    extern_ptr_t GC_base(p)
    extern_ptr_t p;
# endif
{
    register word r;
    register struct hblk *h;
    register hdr *candidate_hdr;
    
    r = (word)p;
    h = HBLKPTR(r);
    candidate_hdr = HDR(r);
    if (candidate_hdr == 0) return(0);
    /* If it's a pointer to the middle of a large object, move it	*/
    /* to the beginning.						*/
	while (IS_FORWARDING_ADDR_OR_NIL(candidate_hdr)) {
	   h = h - (int)candidate_hdr;
	   r = (word)h + HDR_BYTES;
	   candidate_hdr = HDR(h);
	}
    if (candidate_hdr -> hb_map == GC_invalid_map) return(0);
    /* Make sure r points to the beginning of the object */
	r &= ~(WORDS_TO_BYTES(1) - 1);
        {
	    register int offset =
	        	(word *)r - (word *)(HBLKPTR(r)) - HDR_WORDS;
	    register signed_word sz = candidate_hdr -> hb_sz;
	    register int correction;
	        
	    correction = offset % sz;
	    r -= (WORDS_TO_BYTES(correction));
	    if (((word *)r + sz) > (word *)(h + 1)
	        && sz <= BYTES_TO_WORDS(HBLKSIZE) - HDR_WORDS) {
	        return(0);
	    }
	}
    return((extern_ptr_t)r);
}

/* Return the size of an object, given a pointer to its base.		*/
/* (For small obects this also happens to work from interior pointers,	*/
/* but that shouldn't be relied upon.)					*/
# ifdef __STDC__
    size_t GC_size(extern_ptr_t p)
# else
    size_t GC_size(p)
    extern_ptr_t p;
# endif
{
    register int sz;
    register hdr * hhdr = HDR(p);
    
    sz = WORDS_TO_BYTES(hhdr -> hb_sz);
    if (sz < 0) {
        return(-sz);
    } else {
        return(sz);
    }
}

size_t GC_get_heap_size(NO_PARAMS)
{
    return ((size_t) GC_heapsize);
}

bool GC_is_initialized = FALSE;

void GC_init()
{
    DCL_LOCK_STATE;
    
    DISABLE_SIGNALS();
    LOCK();
    GC_init_inner();
    UNLOCK();
    ENABLE_SIGNALS();

}

#ifdef MSWIN32
    extern void GC_init_win32();
#endif

void GC_init_inner()
{
    word dummy;
    
    if (GC_is_initialized) return;
    GC_is_initialized = TRUE;
#   ifdef MSWIN32
 	GC_init_win32();
#   endif
#   ifdef SOLARIS_THREADS
	/* We need dirty bits in order to find live stack sections.	*/
        GC_dirty_init();
#   endif
#   if !defined(THREADS) || defined(SOLARIS_THREADS)
      if (GC_stackbottom == 0) {
	GC_stackbottom = GC_get_stack_base();
      }
#   endif
    if  (sizeof (ptr_t) != sizeof(word)) {
        ABORT("sizeof (ptr_t) != sizeof(word)\n");
    }
    if  (sizeof (signed_word) != sizeof(word)) {
        ABORT("sizeof (signed_word) != sizeof(word)\n");
    }
    if  (sizeof (struct hblk) != HBLKSIZE) {
        ABORT("sizeof (struct hblk) != HBLKSIZE\n");
    }
#   ifndef THREADS
#     if defined(STACK_GROWS_UP) && defined(STACK_GROWS_DOWN)
  	ABORT(
  	  "Only one of STACK_GROWS_UP and STACK_GROWS_DOWN should be defd\n");
#     endif
#     if !defined(STACK_GROWS_UP) && !defined(STACK_GROWS_DOWN)
  	ABORT(
  	  "One of STACK_GROWS_UP and STACK_GROWS_DOWN should be defd\n");
#     endif
#     ifdef STACK_GROWS_DOWN
        if ((word)(&dummy) > (word)GC_stackbottom) {
          GC_err_printf0(
          	"STACK_GROWS_DOWN is defd, but stack appears to grow up\n");
          GC_err_printf2("sp = 0x%lx, GC_stackbottom = 0x%lx\n",
          	   	 (unsigned long) (&dummy),
          	   	 (unsigned long) GC_stackbottom);
          ABORT("stack direction 3\n");
        }
#     else
        if ((word)(&dummy) < (word)GC_stackbottom) {
          GC_err_printf0(
          	"STACK_GROWS_UP is defd, but stack appears to grow down\n");
          GC_err_printf2("sp = 0x%lx, GC_stackbottom = 0x%lx\n",
          	       	 (unsigned long) (&dummy),
          	     	 (unsigned long) GC_stackbottom);
          ABORT("stack direction 4");
        }
#     endif
#   endif
#   if !defined(_AUX_SOURCE) || defined(__GNUC__)
      if ((word)(-1) < (word)0) {
    	GC_err_printf0("The type word should be an unsigned integer type\n");
    	GC_err_printf0("It appears to be signed\n");
    	ABORT("word");
      }
#   endif
    if ((signed_word)(-1) >= (signed_word)0) {
    	GC_err_printf0(
    		"The type signed_word should be a signed integer type\n");
    	GC_err_printf0("It appears to be unsigned\n");
    	ABORT("signed_word");
    }
    
    GC_init_headers();
    /* Add initial guess of root sets */
      GC_register_data_segments();
    GC_bl_init();
    GC_mark_init();
    if (!GC_expand_hp_inner((word)MINHINCR)) {
        GC_err_printf0("Can't start up: not enough memory\n");
        EXIT();
    }
    /* Preallocate large object map.  It's otherwise inconvenient to 	*/
    /* deal with failure.						*/
      if (!GC_add_map_entry((word)0)) {
        GC_err_printf0("Can't start up: not enough memory\n");
        EXIT();
      }
    GC_register_displacement_inner(0L);
#   ifdef MERGE_SIZES
      GC_init_size_map();
#   endif
#   ifdef PCR
      PCR_IL_Lock(PCR_Bool_false, PCR_allSigsBlocked, PCR_waitForever);
      PCR_IL_Unlock();
      GC_pcr_install();
#   endif
    /* Get black list set up */
      GC_gcollect_inner();
#   ifdef STUBBORN_ALLOC
    	GC_stubborn_init();
#   endif
    /* Convince lint that some things are used */
#   ifdef LINT
      {
          extern char * GC_copyright[];
          extern GC_read();
          
          GC_noop(GC_copyright, GC_find_header, GC_print_block_list,
                  GC_push_one, GC_call_with_alloc_lock, GC_read,
                  GC_print_hblkfreelist, GC_dont_expand);
      }
#   endif
}

void GC_enable_incremental(NO_PARAMS)
{
    DCL_LOCK_STATE;
    
# ifndef FIND_LEAK
    DISABLE_SIGNALS();
    LOCK();
    if (GC_incremental) goto out;
#   ifndef SOLARIS_THREADS
        GC_dirty_init();
#   endif
    if (!GC_is_initialized) {
        GC_init_inner();
    }
    if (GC_dont_gc) {
        /* Can't easily do it. */
        UNLOCK();
    	ENABLE_SIGNALS();
    	return;
    }
    if (GC_words_allocd > 0) {
    	/* There may be unmarked reachable objects	*/
    	GC_gcollect_inner();
    }   /* else we're OK in assuming everything's	*/
    	/* clean since nothing can point to an	  	*/
    	/* unmarked object.			  	*/
    GC_read_dirty();
    GC_incremental = TRUE;
out:
    UNLOCK();
    ENABLE_SIGNALS();
# endif
}

#if defined(OS2) || defined(MSWIN32)
    FILE * GC_stdout = NULL;
    FILE * GC_stderr = NULL;
#endif

#ifdef MSWIN32
  void GC_set_files()
  {
    if (GC_stdout == NULL) {
	GC_stdout = fopen("gc.log", "wt");
    }
    if (GC_stderr == NULL) {
	GC_stderr = GC_stdout;
    }
  }
#endif

#ifdef OS2
  void GC_set_files()
  {
      if (GC_stdout == NULL) {
	GC_stdout = stdout;
    }
    if (GC_stderr == NULL) {
	GC_stderr = stderr;
    }
  }
#endif

#ifdef SOLARIS_THREADS
#   define WRITE(f, buf, len) syscall(SYS_write, (f), (buf), (len))
#else
#   define WRITE(f, buf, len) write((f), (buf), (len))
#endif

/* A version of printf that is unlikely to call malloc, and is thus safer */
/* to call from the collector in case malloc has been bound to GC_malloc. */
/* Assumes that no more than 1023 characters are written at once.	  */
/* Assumes that all arguments have been converted to something of the	  */
/* same size as long, and that the format conversions expect something	  */
/* of that size.							  */
void GC_printf(format, a, b, c, d, e, f)
char * format;
long a, b, c, d, e, f;
{
    char buf[1025];
    
    if (GC_quiet) return;
    buf[1024] = 0x15;
    (void) sprintf(buf, format, a, b, c, d, e, f);
    if (buf[1024] != 0x15) ABORT("GC_printf clobbered stack");
#   if defined(OS2) || defined(MSWIN32)
      GC_set_files();
      /* We hope this doesn't allocate */
      if (fwrite(buf, 1, strlen(buf), GC_stdout) != strlen(buf))
          ABORT("write to stdout failed");
      fflush(GC_stdout);
#   else
      if (WRITE(1, buf, strlen(buf)) < 0) ABORT("write to stdout failed");
#   endif
}

void GC_err_printf(format, a, b, c, d, e, f)
char * format;
long a, b, c, d, e, f;
{
    char buf[1025];
    
    buf[1024] = 0x15;
    (void) sprintf(buf, format, a, b, c, d, e, f);
    if (buf[1024] != 0x15) ABORT("GC_err_printf clobbered stack");
#   if defined(OS2) || defined(MSWIN32)
      GC_set_files();
      /* We hope this doesn't allocate */
      if (fwrite(buf, 1, strlen(buf), GC_stderr) != strlen(buf))
          ABORT("write to stderr failed");
      fflush(GC_stderr);
#   else
      if (WRITE(2, buf, strlen(buf)) < 0) ABORT("write to stderr failed");
#   endif
}

void GC_err_puts(s)
char *s;
{
#   if defined(OS2) || defined(MSWIN32)
      GC_set_files();
      /* We hope this doesn't allocate */
      if (fwrite(s, 1, strlen(s), GC_stderr) != strlen(s))
          ABORT("write to stderr failed");
      fflush(GC_stderr);
#   else
      if (WRITE(2, s, strlen(s)) < 0) ABORT("write to stderr failed");
#   endif
}

#ifndef PCR
void GC_abort(msg)
char * msg;
{
    GC_err_printf1("%s\n", msg);
    (void) abort();
}
#endif

# ifdef SRC_M3
void GC_enable()
{
    GC_dont_gc--;
}

void GC_disable()
{
    GC_dont_gc++;
}
# endif
