/*  sexp.h -- header for sexp library                         */
/*  Copyright (c) 2009-2022 Alex Shinn.  All rights reserved. */
/*  BSD-style license: http://synthcode.com/license.txt       */

#ifndef SEXP_H
#define SEXP_H

#ifdef __cplusplus
extern "C" {
#define SEXP_FLEXIBLE_ARRAY [SEXP_FLEXIBLE_ARRAY_SIZE]
#else
#define SEXP_FLEXIBLE_ARRAY []
#endif

#define SEXP_MODULE_PATH_VAR "CHIBI_MODULE_PATH"
#define SEXP_NO_SYSTEM_PATH_VAR "CHIBI_IGNORE_SYSTEM_PATH"

#include "chibi/features.h"
#include "chibi/install.h"

#ifdef _WIN32
#include <windows.h>
#include <errno.h>
#define sexp_isalpha(x) (isalpha(x))
#define sexp_isxdigit(x) (isxdigit(x))
#define sexp_isdigit(x) (isdigit(x))
#define sexp_tolower(x) (tolower(x))
#define sexp_toupper(x) (toupper(x))
#define SEXP_USE_POLL_PORT 0
#define sexp_poll_input(ctx, port) usleep(SEXP_POLL_SLEEP_TIME)
#define sexp_poll_output(ctx, port) usleep(SEXP_POLL_SLEEP_TIME)
#else
#if SEXP_USE_DL
#include <dlfcn.h>
#endif
#ifndef PLAN9
#include <errno.h>
#include <unistd.h>
#define SEXP_USE_POLL_PORT 1
#define sexp_poll_input(ctx, port) sexp_poll_port(ctx, port, 1)
#define sexp_poll_output(ctx, port) sexp_poll_port(ctx, port, 0)
#else
#define SEXP_USE_POLL_PORT 0
#define sexp_poll_input(ctx, port) sleep(SEXP_POLL_SLEEP_TIME_MS)
#define sexp_poll_output(ctx, port) sleep(SEXP_POLL_SLEEP_TIME_MS)
#endif
#if SEXP_USE_GREEN_THREADS
#include <sys/time.h>
#include <sys/select.h>
#include <fcntl.h>
#include <poll.h>
#endif
#define sexp_isalpha(x) (isalpha(x))
#define sexp_isxdigit(x) (isxdigit(x))
#define sexp_isdigit(x) (isdigit(x))
#define sexp_tolower(x) (tolower(x))
#define sexp_toupper(x) (toupper(x))
#endif

#if SEXP_USE_GC_FILE_DESCRIPTORS
#define sexp_out_of_file_descriptors() (errno == EMFILE)
#else
#define sexp_out_of_file_descriptors() (0)
#endif

#ifdef __GNUC__
#define SEXP_NO_WARN_UNUSED __attribute__((unused))
#else
#define SEXP_NO_WARN_UNUSED
#endif

#ifdef PLAN9
#include <u.h>
#include <libc.h>
#include <fcall.h>
#include <thread.h>
#include <9p.h>
typedef unsigned long size_t;
typedef long long off_t;
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define exit(x)           exits(TOSTRING(x))
#define fabsl          fabs
#define M_LN10         2.30258509299404568402  /* log_e 10 */
#define FLT_RADIX 2
#define isfinite(x) !(isNaN(x) || isInf(x,0))
typedef u32int uint32_t;
typedef s32int int32_t;
typedef u64int uint64_t;
typedef s64int int64_t;
#else
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#if !(defined _WIN32) || defined(__CYGWIN__)
#include <sys/socket.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#define _REENTRANT 1
#include <math.h>
#if SEXP_USE_FLONUMS
#include <float.h>
#include <limits.h>
#endif
#endif

#if SEXP_USE_TRACK_ALLOC_BACKTRACE
#include <execinfo.h>
#endif

#include <ctype.h>
#include <stdio.h>

/* tagging system
 *   bits end in     1:  fixnum
 *                  00:  pointer
 *                 010:  string cursor (optional)
 *                0110:  immediate symbol (optional)
 *            00001110:  immediate flonum (optional)
 *            00011110:  char
 *            00101110:  reader label (optional)
 *            00111110:  unique immediate (NULL, TRUE, FALSE)
 */

#define SEXP_FIXNUM_BITS 1
#define SEXP_POINTER_BITS 2
#define SEXP_STRING_CURSOR_BITS 3
#define SEXP_IMMEDIATE_BITS 4
#define SEXP_EXTENDED_BITS 8

#define SEXP_FIXNUM_MASK ((1<<SEXP_FIXNUM_BITS)-1)
#define SEXP_POINTER_MASK ((1<<SEXP_POINTER_BITS)-1)
#define SEXP_STRING_CURSOR_MASK ((1<<SEXP_STRING_CURSOR_BITS)-1)
#define SEXP_IMMEDIATE_MASK ((1<<SEXP_IMMEDIATE_BITS)-1)
#define SEXP_EXTENDED_MASK ((1<<SEXP_EXTENDED_BITS)-1)

#define SEXP_POINTER_TAG 0
#define SEXP_FIXNUM_TAG 1
#define SEXP_STRING_CURSOR_TAG 2
#define SEXP_ISYMBOL_TAG 6
#define SEXP_IFLONUM_TAG 14
#define SEXP_CHAR_TAG 30
#define SEXP_READER_LABEL_TAG 46
#define SEXP_EXTENDED_TAG 62

#ifndef SEXP_POINTER_MAGIC
#define SEXP_POINTER_MAGIC 0xFDCA9764uL /* arbitrary */
#endif

#if SEXP_USE_HASH_SYMS
#define SEXP_SYMBOL_TABLE_SIZE 389
#else
#define SEXP_SYMBOL_TABLE_SIZE 1
#endif

enum sexp_types {
  SEXP_OBJECT,
  SEXP_TYPE,
  SEXP_FIXNUM,
  SEXP_NUMBER,
  SEXP_CHAR,
  SEXP_BOOLEAN,
  SEXP_PAIR,
  SEXP_SYMBOL,
  SEXP_BYTES,
  SEXP_STRING,
  SEXP_VECTOR,
  SEXP_FLONUM,
  SEXP_BIGNUM,
#if SEXP_USE_STABLE_ABI || SEXP_USE_RATIOS
  SEXP_RATIO,
#endif
#if SEXP_USE_STABLE_ABI || SEXP_USE_COMPLEX
  SEXP_COMPLEX,
#endif
#if SEXP_USE_STABLE_ABI || SEXP_USE_DISJOINT_STRING_CURSORS
  SEXP_STRING_CURSOR,
#endif
  SEXP_IPORT,
  SEXP_OPORT,
  SEXP_FILENO,
  SEXP_EXCEPTION,
  SEXP_PROCEDURE,
  SEXP_MACRO,
  SEXP_SYNCLO,
  SEXP_ENV,
  SEXP_BYTECODE,
  SEXP_CORE,
#if SEXP_USE_STABLE_ABI || SEXP_USE_DL
  SEXP_DL,
#endif
  SEXP_OPCODE,
  SEXP_LAMBDA,
  SEXP_CND,
  SEXP_REF,
  SEXP_SET,
  SEXP_SET_SYN,
  SEXP_SEQ,
  SEXP_LIT,
  SEXP_STACK,
  SEXP_CONTEXT,
  SEXP_CPOINTER,
  SEXP_UNIFORM_VECTOR,
#if SEXP_USE_STABLE_ABI || SEXP_USE_AUTO_FORCE
  SEXP_PROMISE,
#endif
#if SEXP_USE_STABLE_ABI || SEXP_USE_WEAK_REFERENCES
  SEXP_EPHEMERON,
#endif
  SEXP_NUM_CORE_TYPES
};

#if !SEXP_USE_DISJOINT_STRING_CURSORS
#define SEXP_STRING_CURSOR SEXP_FIXNUM
#endif

#ifdef _WIN32
#if SEXP_64_BIT
typedef unsigned int sexp_tag_t;
typedef unsigned long long sexp_uint_t;
typedef long long sexp_sint_t;
#define SEXP_PRIdFIXNUM "lld"
#else
typedef unsigned short sexp_tag_t;
typedef unsigned int sexp_uint_t;
typedef int sexp_sint_t;
#define SEXP_PRIdFIXNUM "d"
#endif
#define sexp_heap_align(n) sexp_align(n, 5)
#define sexp_heap_chunks(n) (sexp_heap_align(n)>>5)
#elif SEXP_64_BIT
#if PLAN9
typedef uintptr sexp_tag_t;
typedef uintptr sexp_uint_t;
typedef intptr sexp_sint_t;
#else
typedef unsigned int sexp_tag_t;
typedef unsigned long sexp_uint_t;
typedef long sexp_sint_t;
#endif
#define SEXP_PRIdFIXNUM "ld"
#define sexp_heap_align(n) sexp_align(n, 5)
#define sexp_heap_chunks(n) (sexp_heap_align(n)>>5)
#elif defined(__CYGWIN__)
typedef unsigned short sexp_tag_t;
typedef unsigned int sexp_uint_t;
typedef int sexp_sint_t;
#define SEXP_PRIdFIXNUM "d"
#define sexp_heap_align(n) sexp_align(n, 5)
#define sexp_heap_chunks(n) (sexp_heap_align(n)>>5)
#elif PLAN9
typedef uintptr sexp_tag_t;
typedef unsigned int sexp_uint_t;
typedef int sexp_sint_t;
#define SEXP_PRIdFIXNUM "d"
#define sexp_heap_align(n) sexp_align(n, 4)
#define sexp_heap_chunks(n) (sexp_heap_align(n)>>4)
#else
typedef unsigned short sexp_tag_t;
typedef unsigned int sexp_uint_t;
typedef int sexp_sint_t;
#define SEXP_PRIdFIXNUM "d"
#define sexp_heap_align(n) sexp_align(n, 4)
#define sexp_heap_chunks(n) (sexp_heap_align(n)>>4)
#endif

/* procedure flags */
#define SEXP_PROC_NONE ((sexp_uint_t)0)
#define SEXP_PROC_VARIADIC ((sexp_uint_t)1)
#define SEXP_PROC_UNUSED_REST ((sexp_uint_t)2)
#define SEXP_PROC_VARIABLE_TRANSFORMER ((sexp_uint_t)4)


#ifdef SEXP_USE_INTTYPES
#ifdef PLAN9
#include <ape/stdint.h>
#else
#include <stdint.h>
#endif
# ifdef UINT8_MAX
#  define SEXP_UINT8_DEFINED 1
typedef uint8_t  sexp_uint8_t;
# endif
# ifdef UINT32_MAX
#  define SEXP_UINT32_DEFINED 1
typedef uint32_t sexp_uint32_t;
typedef int32_t sexp_int32_t;
# endif
#else
# ifdef PLAN9
# include <ape/limits.h>
# else
# include <limits.h>
# if SEXP_USE_UNIFORM_VECTOR_LITERALS
# ifdef PLAN9
# include <ape/stdint.h>
# else
# include <stdint.h>
# endif
# endif
# endif
# if UCHAR_MAX == 255
#  define SEXP_UINT8_DEFINED 1
typedef unsigned char sexp_uint8_t;
# endif
# if UINT_MAX == 4294967295U
#  define SEXP_UINT32_DEFINED 1
typedef unsigned int sexp_uint32_t;
typedef int sexp_int32_t;
# elif ULONG_MAX == 4294967295UL
#  define SEXP_UINT32_DEFINED 1
typedef unsigned long sexp_uint32_t;
typedef long sexp_int32_t;
# elif USHRT_MAX == 4294967295U
#  define SEXP_UINT32_DEFINED 1
typedef unsigned short sexp_uint32_t;
typedef short sexp_int32_t;
# endif
#endif  /* SEXP_USE_INTTYPES */

#if defined(__APPLE__) || defined(_WIN64) || (defined(__CYGWIN__) && __SIZEOF_POINTER__ == 8)
#define SEXP_PRIdOFF "lld"
#else
#define SEXP_PRIdOFF "ld"
#endif

#if SEXP_USE_LONG_PROCEDURE_ARGS
typedef int sexp_proc_num_args_t;
#else
typedef short sexp_proc_num_args_t;
#endif

typedef struct sexp_struct *sexp;

#define sexp_heap_pad_size(s) (sizeof(struct sexp_heap_t) + (s) + sexp_heap_align(1))
#define sexp_free_chunk_size (sizeof(struct sexp_free_list_t))
#define sexp_heap_first_block(h) ((sexp)(h->data + sexp_heap_align(sexp_free_chunk_size)))
#define sexp_heap_last_block(h) ((sexp)((char*)h->data + h->size - sexp_heap_align(sexp_free_chunk_size)))
#define sexp_heap_end(h) ((sexp)((char*)h->data + h->size))

#define __HALF_MAX_SIGNED(type) ((type)1 << (sizeof(type)*8-2))
#define __MAX_SIGNED(type) (__HALF_MAX_SIGNED(type) - 1 + __HALF_MAX_SIGNED(type))
#define __MIN_SIGNED(type) (-1 - __MAX_SIGNED(type))

#define SEXP_UINT_T_MAX ((sexp_uint_t)-1)
#define SEXP_UINT_T_MIN (0)
#define SEXP_SINT_T_MAX __MAX_SIGNED(sexp_sint_t)
#define SEXP_SINT_T_MIN __MIN_SIGNED(sexp_sint_t)

#define SEXP_MAX_FIXNUM ((((sexp_sint_t)1)<<(sizeof(sexp_sint_t)*8-SEXP_FIXNUM_BITS-1))-1)
#define SEXP_MIN_FIXNUM (-SEXP_MAX_FIXNUM-1)

#if SEXP_USE_SELF_PARAMETER
#define sexp_api_params(self, n) , sexp self, sexp_sint_t n
#define sexp_api_pass(self, n) , self, n
#else
#define sexp_api_params(self, n)
#define sexp_api_pass(self, n)
#endif

/* procedure types */
typedef sexp (*sexp_proc1) (sexp, sexp, sexp_sint_t);
typedef sexp (*sexp_proc2) (sexp, sexp, sexp_sint_t, sexp);
typedef sexp (*sexp_proc3) (sexp, sexp, sexp_sint_t, sexp, sexp);
typedef sexp (*sexp_proc4) (sexp, sexp, sexp_sint_t, sexp, sexp, sexp);
typedef sexp (*sexp_proc5) (sexp, sexp, sexp_sint_t, sexp, sexp, sexp, sexp);
typedef sexp (*sexp_proc6) (sexp, sexp, sexp_sint_t, sexp, sexp, sexp, sexp, sexp);
typedef sexp (*sexp_proc7) (sexp, sexp, sexp_sint_t, sexp, sexp, sexp, sexp, sexp, sexp);
typedef sexp (*sexp_init_proc)(sexp, sexp, sexp_sint_t, sexp, const char*, const sexp_abi_identifier_t);
SEXP_API sexp sexp_init_library(sexp, sexp, sexp_sint_t, sexp, const char*, const sexp_abi_identifier_t);

typedef struct sexp_free_list_t *sexp_free_list;
struct sexp_free_list_t {
  sexp_uint_t size;
  sexp_free_list next;
};

typedef struct sexp_heap_t *sexp_heap;
struct sexp_heap_t {
  sexp_uint_t size, max_size, chunk_size;
  sexp_free_list free_list;
  sexp_heap next;
  /* note this must be aligned on a proper heap boundary, */
  /* so we can't just use char data[] */
  char *data;
};

struct sexp_gc_var_t {
  sexp *var;
#if SEXP_USE_DEBUG_GC
  char *name;
#endif
  struct sexp_gc_var_t *next;
};

struct sexp_library_entry_t {   /* for static builds and user exported C */
  const char *name;             /* libaries */
  sexp_init_proc init;
};

struct sexp_type_struct {
  sexp name, cpl, slots, getters, setters, id, print, dl, finalize_name;
  sexp_tag_t tag;
  short field_base, field_eq_len_base, field_len_base, field_len_off;
  unsigned short field_len_scale;
  short size_base, size_off;
  unsigned short size_scale;
  short weak_base, weak_len_base, weak_len_off, weak_len_scale, weak_len_extra;
  short depth;
  sexp_proc2 finalize;
};

struct sexp_opcode_struct {
  sexp name, data, data2, proc, ret_type, arg1_type, arg2_type, arg3_type,
    argn_type, methods, dl;
  unsigned char op_class, code, num_args, flags, inverse;
  sexp_proc1 func;
};

struct sexp_core_form_struct {
  char code;
  sexp name;
};

struct sexp_mark_stack_ptr_t {
  sexp *start, *end;
  struct sexp_mark_stack_ptr_t *prev; /* TODO: remove for allocations on stack */
};

/* Note this must be kept in sync with the _sexp_type_specs type            */
/* registry in sexp.c.  The structure of a sexp type is:                    */
/*   [ HEADER [[EQ_FIELDS... ] GC_FIELDS...] [WEAK_FIELDS...] [OTHER...] ]  */
/* Thus all sexp's must be contiguous and align at the start of the type.   */
/* This is used by the gc, equal? and slot-ref (although only the latter    */
/* expects the alignment at the start of the type). */
struct sexp_struct {
  sexp_tag_t tag;
  char markedp;
  unsigned int immutablep:1;
  unsigned int freep:1;
  unsigned int brokenp:1;
  unsigned int syntacticp:1;
  unsigned int copyonwritep:1;
#if SEXP_USE_TRACK_ALLOC_SOURCE
  const char* source;
  void* backtrace[SEXP_BACKTRACE_SIZE];
#endif
#if SEXP_USE_HEADER_MAGIC
  unsigned int magic;
#endif
  union {
    /* basic types */
    double flonum;
    char flonum_bits[sizeof(double)];  /* for eqv? comparison on flonums */
    struct sexp_type_struct type;
    struct {
      sexp car, cdr;
      sexp source;
    } pair;
    struct {
      sexp_uint_t length;
    } vector;
    struct {
      sexp_uint_t length;
    } bytes;
    struct {
      sexp bytes;
      unsigned char element_type;
      sexp_sint_t length;
    } uvector;
    struct {
#if SEXP_USE_PACKED_STRINGS
#if SEXP_USE_STRING_INDEX_TABLE
      sexp charlens;
#endif
      sexp_uint_t length;
#else
      sexp bytes;
#if SEXP_USE_STRING_INDEX_TABLE
      sexp charlens;
#elif SEXP_USE_STRING_REF_CACHE
      sexp_uint_t cached_char_idx;
      sexp cached_cursor;
#endif
      sexp_uint_t offset, length;
#endif
    } string;
    struct {
      sexp_uint_t length;
    } symbol;
    struct {
      sexp name;
      sexp cookie;
      sexp fd;
      FILE *stream;
      char *buf;
      char openp, bidirp, binaryp, shutdownp, no_closep, sourcep,
        blockedp, fold_casep;
      sexp_uint_t offset, line, flags;
      size_t size;
    } port;
    struct {
      char openp, no_closep;
      sexp_sint_t fd, count;
    } fileno;
    struct {
      sexp kind, message, irritants, procedure, source, stack_trace;
    } exception;
    struct {
      signed char sign;
      sexp_uint_t length;
    } bignum;
    struct {
      sexp numerator, denominator;
    } ratio;
    struct {
      sexp real, imag;
    } complex;
    struct {
      sexp parent;
      sexp_uint_t length;
      void *value;
    } cpointer;
    /* runtime types */
    struct {
      sexp parent, lambda, bindings;
#if SEXP_USE_STABLE_ABI || SEXP_USE_RENAME_BINDINGS
      sexp renames;
#endif
    } env;
    struct {
      sexp name, literals, source;
      sexp_uint_t length, max_depth;
    } bytecode;
    struct {
      sexp bc, vars;
      char flags;  /* a boxed fixnum truncated to char */
      sexp_proc_num_args_t num_args;
    } procedure;
    struct {
      sexp proc, env, source, aux;
    } macro;
    struct {
      sexp env, free_vars, expr, rename;
    } synclo;
    struct {
      sexp file;
      void* handle;
    } dl;
    struct sexp_opcode_struct opcode;
    struct sexp_core_form_struct core;
    /* ast types */
    struct {
      sexp name, params, body, defs, locals, flags, fv, sv, ret, types, source;
    } lambda;
    struct {
      sexp test, pass, fail, source;
    } cnd;
    struct {
      sexp var, value, source;
    } set;
    struct {
      sexp var, value, source;
    } set_syn;
    struct {
      sexp name, cell, source;
    } ref;
    struct {
      sexp ls, source;
    } seq;
    struct {
      sexp value, source;
    } lit;
    /* compiler state */
    struct {
      sexp_uint_t length, top;
    } stack;
    struct {
      sexp stack, env, parent, child,
        globals, dk, params, proc, name, specific, event, result;
#if SEXP_USE_STABLE_ABI || SEXP_USE_DL
      sexp dl;
#endif
      sexp_heap heap;
      struct sexp_mark_stack_ptr_t mark_stack[SEXP_MARK_STACK_COUNT];
      struct sexp_mark_stack_ptr_t *mark_stack_ptr;
      struct sexp_gc_var_t *saves;
#if SEXP_USE_GREEN_THREADS
      sexp_sint_t refuel;
      unsigned char* ip;
      struct timeval tval;
#endif
      char tailp, tracep, timeoutp, waitp, errorp, interruptp;
      sexp_uint_t last_fp;
      sexp_uint_t gc_count;
#if SEXP_USE_TIME_GC
      sexp_uint_t gc_usecs;
#endif
#if SEXP_USE_TRACK_ALLOC_TIMES
      sexp_uint_t alloc_count, alloc_usecs;
      double alloc_usecs_sq;
#endif
#if SEXP_USE_TRACK_ALLOC_SIZES
      sexp_uint_t alloc_histogram[SEXP_ALLOC_HISTOGRAM_BUCKETS];
#endif
    } context;
#if SEXP_USE_STABLE_ABI || SEXP_USE_AUTO_FORCE
    struct {
      sexp value;
      int donep;
    } promise;
#endif
#if SEXP_USE_STABLE_ABI || SEXP_USE_WEAK_REFERENCES
    struct {
      sexp key, value;
    } ephemeron;
#endif
  } value;
};

#define SEXP_MAKE_IMMEDIATE(n)  ((sexp) ((n<<SEXP_EXTENDED_BITS) \
                                          + SEXP_EXTENDED_TAG))

#define SEXP_FALSE  SEXP_MAKE_IMMEDIATE(0) /* 14 0x0e */
#define SEXP_TRUE   SEXP_MAKE_IMMEDIATE(1) /* 30 0x1e */
#define SEXP_NULL   SEXP_MAKE_IMMEDIATE(2) /* 46 0x2e */
#define SEXP_EOF    SEXP_MAKE_IMMEDIATE(3) /* 62 0x3e */
#define SEXP_VOID   SEXP_MAKE_IMMEDIATE(4) /* the unspecified value */
#define SEXP_UNDEF  SEXP_MAKE_IMMEDIATE(5) /* internal use */
#define SEXP_CLOSE  SEXP_MAKE_IMMEDIATE(6) /* internal use */
#define SEXP_RAWDOT SEXP_MAKE_IMMEDIATE(7) /* internal use */
#define SEXP_STRING_OPORT SEXP_MAKE_IMMEDIATE(8)  /* internal use */
#define SEXP_TRAMPOLINE   SEXP_MAKE_IMMEDIATE(9)  /* internal use */
#define SEXP_UNCAUGHT     SEXP_MAKE_IMMEDIATE(10) /* internal use */
#define SEXP_ABI_ERROR    SEXP_MAKE_IMMEDIATE(11) /* internal use */
#if SEXP_USE_OBJECT_BRACE_LITERALS
#define SEXP_CLOSE_BRACE  SEXP_MAKE_IMMEDIATE(12) /* internal use */
#endif

#if SEXP_USE_LIMITED_MALLOC
void* sexp_malloc(size_t size);
void sexp_free(void* ptr);
#else
#define sexp_malloc malloc
#define sexp_free free
#endif

#if SEXP_USE_BOEHM

#define sexp_gc(ctx, sum)

#define sexp_gc_var(x, y)            sexp x = SEXP_VOID;
#define sexp_gc_preserve(ctx, x, y)
#define sexp_gc_release(ctx, x, y)

#define sexp_preserve_object(ctx, x)
#define sexp_release_object(ctx, x)

#include "gc/gc.h"
#define sexp_alloc(ctx, size)        GC_malloc(size)
#define sexp_alloc_atomic(ctx, size) GC_malloc_atomic(size)

#else

SEXP_API sexp sexp_gc(sexp ctx, size_t *sum_freed);

#define sexp_gc_var(x, y)                       \
  sexp x = SEXP_VOID;                           \
  struct sexp_gc_var_t y = {NULL, NULL};

#if SEXP_USE_DEBUG_GC
#define sexp_gc_preserve_name(ctx, x, y) (y).name = #x
#else
#define sexp_gc_preserve_name(ctx, x, y)
#endif

#define sexp_gc_preserve(ctx, x, y)     \
  do {                                  \
    sexp_gc_preserve_name(ctx, x, y);   \
    (y).var = &(x);                     \
    (y).next = sexp_context_saves(ctx); \
    sexp_context_saves(ctx) = &(y);     \
  } while (0)

#define sexp_gc_release(ctx, x, y)   (sexp_context_saves(ctx) = y.next)

SEXP_API void sexp_preserve_object(sexp ctx, sexp x);
SEXP_API void sexp_release_object(sexp ctx, sexp x);

#if SEXP_USE_MALLOC
#define sexp_alloc(ctx, size)        sexp_malloc(size)
#define sexp_alloc_atomic(ctx, size) sexp_malloc(size)
#else  /* native gc */
void* sexp_alloc(sexp ctx, size_t size);
#define sexp_alloc_atomic            sexp_alloc
#endif
#endif

#define sexp_gc_var1(x) sexp_gc_var(x, __sexp_gc_preserver1)
#define sexp_gc_var2(x, y) sexp_gc_var1(x) sexp_gc_var(y, __sexp_gc_preserver2)
#define sexp_gc_var3(x, y, z) sexp_gc_var2(x, y) sexp_gc_var(z, __sexp_gc_preserver3)
#define sexp_gc_var4(x, y, z, w) sexp_gc_var3(x, y, z) sexp_gc_var(w, __sexp_gc_preserver4)
#define sexp_gc_var5(x, y, z, w, v) sexp_gc_var4(x, y, z, w) sexp_gc_var(v, __sexp_gc_preserver5)
#define sexp_gc_var6(x, y, z, w, v, u) sexp_gc_var5(x, y, z, w, v) sexp_gc_var(u, __sexp_gc_preserver6)
#define sexp_gc_var7(x, y, z, w, v, u, t) sexp_gc_var6(x, y, z, w, v, u) sexp_gc_var(t, __sexp_gc_preserver7)

#define sexp_gc_preserve1(ctx, x) sexp_gc_preserve(ctx, x, __sexp_gc_preserver1)
#define sexp_gc_preserve2(ctx, x, y) sexp_gc_preserve1(ctx, x); sexp_gc_preserve(ctx, y, __sexp_gc_preserver2)
#define sexp_gc_preserve3(ctx, x, y, z) sexp_gc_preserve2(ctx, x, y); sexp_gc_preserve(ctx, z, __sexp_gc_preserver3)
#define sexp_gc_preserve4(ctx, x, y, z, w) sexp_gc_preserve3(ctx, x, y, z); sexp_gc_preserve(ctx, w, __sexp_gc_preserver4)
#define sexp_gc_preserve5(ctx, x, y, z, w, v) sexp_gc_preserve4(ctx, x, y, z, w); sexp_gc_preserve(ctx, v, __sexp_gc_preserver5)
#define sexp_gc_preserve6(ctx, x, y, z, w, v, u) sexp_gc_preserve5(ctx, x, y, z, w, v); sexp_gc_preserve(ctx, u, __sexp_gc_preserver6)
#define sexp_gc_preserve7(ctx, x, y, z, w, v, u, t) sexp_gc_preserve6(ctx, x, y, z, w, v, u); sexp_gc_preserve(ctx, t, __sexp_gc_preserver7)

#define sexp_gc_release1(ctx) sexp_gc_release(ctx, NULL, __sexp_gc_preserver1)
#define sexp_gc_release2(ctx) sexp_gc_release(ctx, NULL, __sexp_gc_preserver1)
#define sexp_gc_release3(ctx) sexp_gc_release(ctx, NULL, __sexp_gc_preserver1)
#define sexp_gc_release4(ctx) sexp_gc_release(ctx, NULL, __sexp_gc_preserver1)
#define sexp_gc_release5(ctx) sexp_gc_release(ctx, NULL, __sexp_gc_preserver1)
#define sexp_gc_release6(ctx) sexp_gc_release(ctx, NULL, __sexp_gc_preserver1)
#define sexp_gc_release7(ctx) sexp_gc_release(ctx, NULL, __sexp_gc_preserver1)

#define sexp_align(n, bits) (((n)+(1<<(bits))-1)&(((sexp_uint_t)-1)-((1<<(bits))-1)))

#if SEXP_64_BIT
#define sexp_word_align(n) sexp_align((n), 3)
#else
#define sexp_word_align(n) sexp_align((n), 2)
#endif

#define sexp_sizeof(x) (offsetof(struct sexp_struct, value) \
                         + sizeof(((sexp)0)->value.x))
#define sexp_offsetof(type, f) (offsetof(struct sexp_struct, value.type.f))
#define sexp_offsetof_slot0 (offsetof(struct sexp_struct, value))
#define sexp_sizeof_header (sexp_sizeof(flonum) - sizeof(double))

#if SEXP_USE_TRACK_ALLOC_SOURCE
#define sexp_with_current_source0(file, line) file ": " #line
#define sexp_with_current_source(file, line) , sexp_with_current_source0(file, line)
#else
#define sexp_with_current_source(file, line)
#endif

#define sexp_alloc_tagged(ctx, size, tag) sexp_alloc_tagged_aux(ctx, size, tag sexp_with_current_source(__FILE__, __LINE__))

#define sexp_alloc_type(ctx, type, tag) sexp_alloc_tagged(ctx, sexp_sizeof(type), tag)
#define sexp_alloc_bytecode(ctx, i) sexp_alloc_tagged(ctx, sexp_sizeof(bytecode) + i, SEXP_BYTECODE)

#if SEXP_USE_BIGNUMS
#include "chibi/bignum.h"
#endif

/***************************** predicates *****************************/

#define sexp_truep(x)    ((x) != SEXP_FALSE)
#define sexp_not(x)      ((x) == SEXP_FALSE)

#define sexp_nullp(x)    ((x) == SEXP_NULL)
#define sexp_pointerp(x) (((sexp_uint_t)(x) & SEXP_POINTER_MASK) == SEXP_POINTER_TAG)
#define sexp_fixnump(x)  (((sexp_uint_t)(x) & SEXP_FIXNUM_MASK) == SEXP_FIXNUM_TAG)
#if SEXP_USE_DISJOINT_STRING_CURSORS
#define sexp_string_cursorp(x)  (((sexp_uint_t)(x) & SEXP_STRING_CURSOR_MASK) == SEXP_STRING_CURSOR_TAG)
#else
#define sexp_string_cursorp(x) sexp_fixnump(x)
#endif
#define sexp_isymbolp(x) (((sexp_uint_t)(x) & SEXP_IMMEDIATE_MASK) == SEXP_ISYMBOL_TAG)
#define sexp_charp(x)    (((sexp_uint_t)(x) & SEXP_EXTENDED_MASK) == SEXP_CHAR_TAG)
#define sexp_reader_labelp(x) (((sexp_uint_t)(x) & SEXP_EXTENDED_MASK) == SEXP_READER_LABEL_TAG)
#define sexp_booleanp(x) (((x) == SEXP_TRUE) || ((x) == SEXP_FALSE))

#define sexp_pointer_tag(x)      ((x)->tag)
#define sexp_markedp(x)          ((x)->markedp)
#define sexp_flags(x)            ((x)->flags)
#define sexp_immutablep(x)       ((x)->immutablep)
#define sexp_mutablep(x)         (!(x)->immutablep)
#define sexp_freep(x)            ((x)->freep)
#define sexp_brokenp(x)          ((x)->brokenp)
#define sexp_pointer_magic(x)    ((x)->magic)
#define sexp_copy_on_writep(x)   ((x)->copyonwritep)

#if SEXP_USE_TRACK_ALLOC_SOURCE
#define sexp_pointer_source(x)   ((x)->source)
#else
#define sexp_pointer_source(x)   ""
#endif

#define sexp_check_tag(x,t)  (sexp_pointerp(x) && (sexp_pointer_tag(x) == (t)))

#define sexp_slot_ref(x,i)   (((sexp*)&((x)->value))[i])
#define sexp_slot_set(x,i,v) (((sexp*)&((x)->value))[i] = (v))

#define sexp_isa(a, b) (sexp_pointerp(a) && sexp_typep(b) && (sexp_pointer_tag(a) == sexp_type_tag(b)))

union sexp_flonum_conv {
  float flonum;
  unsigned int bits;
};

#if SEXP_USE_IMMEDIATE_FLONUMS
#define sexp_flonump(x)      (((sexp_uint_t)(x) & SEXP_EXTENDED_MASK) == SEXP_IFLONUM_TAG)
SEXP_API sexp sexp_flonum_predicate (sexp ctx, sexp x);
#if SEXP_64_BIT
SEXP_API float sexp_flonum_value (sexp x);
#define sexp_flonum_value_set(f, x) (f = sexp_make_flonum(NULL, x))
#define sexp_flonum_bits(f) ((char*)&f)
SEXP_API sexp sexp_make_flonum(sexp ctx, float f);
#else
#define sexp_make_flonum(ctx, x)  ((sexp) ((((union sexp_flonum_conv)((float)(x))).bits & ~SEXP_EXTENDED_MASK) + SEXP_IFLONUM_TAG))
#define sexp_flonum_value(x) (((union sexp_flonum_conv)(((unsigned int)(x)) & ~SEXP_EXTENDED_MASK)).flonum)
#endif
#else
#define sexp_flonump(x)      (sexp_check_tag(x, SEXP_FLONUM))
#define sexp_flonum_value(f) ((f)->value.flonum)
#define sexp_flonum_value_set(f, x) ((f)->value.flonum = x)
#define sexp_flonum_bits(f) ((f)->value.flonum_bits)
SEXP_API sexp sexp_make_flonum(sexp ctx, double f);
#endif

#define sexp_typep(x)       (sexp_check_tag(x, SEXP_TYPE))
#define sexp_pairp(x)       (sexp_check_tag(x, SEXP_PAIR))
#define sexp_stringp(x)     (sexp_check_tag(x, SEXP_STRING))
#define sexp_lsymbolp(x)    (sexp_check_tag(x, SEXP_SYMBOL))
#define sexp_bytesp(x)      (sexp_check_tag(x, SEXP_BYTES))
#define sexp_vectorp(x)     (sexp_check_tag(x, SEXP_VECTOR))
#define sexp_iportp(x)      (sexp_check_tag(x, SEXP_IPORT))
#if SEXP_USE_BIDIRECTIONAL_PORTS
#define sexp_oportp(x)      (sexp_check_tag(x, SEXP_OPORT) || (sexp_check_tag(x, SEXP_IPORT) && sexp_port_bidirp(x)))
#else
#define sexp_oportp(x)      (sexp_check_tag(x, SEXP_OPORT))
#endif
#define sexp_filenop(x)     (sexp_check_tag(x, SEXP_FILENO))
#if SEXP_USE_BIGNUMS
#define sexp_bignump(x)     (sexp_check_tag(x, SEXP_BIGNUM))
#else
#define sexp_bignump(x)     0
#endif
#if SEXP_USE_RATIOS
#define sexp_ratiop(x)      (sexp_check_tag(x, SEXP_RATIO))
#else
#define sexp_ratiop(x)      0
#endif
#if SEXP_USE_COMPLEX
#define sexp_complexp(x)    (sexp_check_tag(x, SEXP_COMPLEX))
#else
#define sexp_complexp(x)    0
#endif
#define sexp_cpointerp(x)   (sexp_check_tag(x, SEXP_CPOINTER))
#define sexp_exceptionp(x)  (sexp_check_tag(x, SEXP_EXCEPTION))
#define sexp_procedurep(x)  (sexp_check_tag(x, SEXP_PROCEDURE))
#define sexp_envp(x)        (sexp_check_tag(x, SEXP_ENV))
#define sexp_bytecodep(x)   (sexp_check_tag(x, SEXP_BYTECODE))
#define sexp_corep(x)       (sexp_check_tag(x, SEXP_CORE))
#define sexp_dlp(x)         (sexp_check_tag(x, SEXP_DL))
#define sexp_opcodep(x)     (sexp_check_tag(x, SEXP_OPCODE))
#define sexp_macrop(x)      (sexp_check_tag(x, SEXP_MACRO))
#define sexp_syntacticp(x)  (sexp_corep(x) || sexp_macrop(x))
#define sexp_synclop(x)     (sexp_check_tag(x, SEXP_SYNCLO))
#define sexp_lambdap(x)     (sexp_check_tag(x, SEXP_LAMBDA))
#define sexp_cndp(x)        (sexp_check_tag(x, SEXP_CND))
#define sexp_refp(x)        (sexp_check_tag(x, SEXP_REF))
#define sexp_setp(x)        (sexp_check_tag(x, SEXP_SET))
#define sexp_set_synp(x)    (sexp_check_tag(x, SEXP_SET_SYN))
#define sexp_seqp(x)        (sexp_check_tag(x, SEXP_SEQ))
#define sexp_litp(x)        (sexp_check_tag(x, SEXP_LIT))
#define sexp_contextp(x)    (sexp_check_tag(x, SEXP_CONTEXT))
#define sexp_promisep(x)    (sexp_check_tag(x, SEXP_PROMISE))
#define sexp_ephemeronp(x)  (sexp_check_tag(x, SEXP_EPHEMERON))

#if SEXP_USE_UNIFORM_VECTOR_LITERALS
#define sexp_uvectorp(x)    (sexp_check_tag(x, SEXP_UNIFORM_VECTOR))
#define sexp_u1vectorp(x)   (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_U1)
#define sexp_u8vectorp(x)   (sexp_bytesp(x))
#define sexp_s8vectorp(x)   (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_S8)
#define sexp_u16vectorp(x)  (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_U16)
#define sexp_s16vectorp(x)  (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_S16)
#define sexp_u32vectorp(x)  (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_U32)
#define sexp_s32vectorp(x)  (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_S32)
#define sexp_u64vectorp(x)  (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_U64)
#define sexp_s64vectorp(x)  (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_S64)
#define sexp_f8vectorp(x)   (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_F8)
#define sexp_f16vectorp(x)  (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_F16)
#define sexp_f32vectorp(x)  (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_F32)
#define sexp_f64vectorp(x)  (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_F64)
#define sexp_c64vectorp(x)  (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_C64)
#define sexp_c128vectorp(x) (sexp_uvectorp(x) && sexp_uvector_type(x)==SEXP_C128)
#else
#define sexp_uvectorp(x)    (sexp_vectorp(x))
#define sexp_u1vectorp(x)   (sexp_vectorp(x))
#define sexp_u8vectorp(x)   (sexp_bytesp(x))
#define sexp_s8vectorp(x)   (sexp_vectorp(x))
#define sexp_u16vectorp(x)  (sexp_vectorp(x))
#define sexp_s16vectorp(x)  (sexp_vectorp(x))
#define sexp_u32vectorp(x)  (sexp_vectorp(x))
#define sexp_s32vectorp(x)  (sexp_vectorp(x))
#define sexp_u64vectorp(x)  (sexp_vectorp(x))
#define sexp_s64vectorp(x)  (sexp_vectorp(x))
#define sexp_f8vectorp(x)   (sexp_vectorp(x))
#define sexp_f16vectorp(x)  (sexp_vectorp(x))
#define sexp_f32vectorp(x)  (sexp_vectorp(x))
#define sexp_f64vectorp(x)  (sexp_vectorp(x))
#define sexp_c64vectorp(x)  (sexp_vectorp(x))
#define sexp_c128vectorp(x) (sexp_vectorp(x))
#endif

#define sexp_applicablep(x) (sexp_procedurep(x) || sexp_opcodep(x))

#if SEXP_USE_HUFF_SYMS
#define sexp_symbolp(x)     (sexp_isymbolp(x) || sexp_lsymbolp(x))
#else
#define sexp_symbolp(x)     (sexp_lsymbolp(x))
#endif

SEXP_API sexp sexp_id_name(sexp x);
SEXP_API int sexp_idp(sexp x);

#define sexp_portp(x) (sexp_check_tag(x, SEXP_IPORT) || sexp_check_tag(x, SEXP_OPORT))

#define sexp_stream_portp(x) (sexp_port_stream(x) != NULL)

#define sexp_port_customp(x) (sexp_vectorp(sexp_port_cookie(x)) && sexp_vector_length(sexp_port_cookie(x)) == 6)

/* only valid on custom ports */
#define sexp_port_buffer(x)  (sexp_vector_ref(sexp_port_cookie(x), SEXP_ONE))
#define sexp_port_reader(x)  (sexp_vector_ref(sexp_port_cookie(x), SEXP_TWO))
#define sexp_port_writer(x)  (sexp_vector_ref(sexp_port_cookie(x), SEXP_THREE))
#define sexp_port_seeker(x)  (sexp_vector_ref(sexp_port_cookie(x), SEXP_FOUR))
#define sexp_port_closer(x)  (sexp_vector_ref(sexp_port_cookie(x), SEXP_FIVE))

/***************************** constructors ****************************/

#define sexp_make_boolean(x) ((x) ? SEXP_TRUE : SEXP_FALSE)
#define sexp_unbox_boolean(x) (((x) == SEXP_FALSE) ? 0 : 1)

#if SEXP_USE_SIGNED_SHIFTS
#define sexp_make_fixnum(n)    ((sexp) ((((sexp_sint_t)(n))<<SEXP_FIXNUM_BITS) + SEXP_FIXNUM_TAG))
#define sexp_unbox_fixnum(n)   (((sexp_sint_t)(n))>>SEXP_FIXNUM_BITS)
#else
#define sexp_make_fixnum(n)    ((sexp) ((((sexp_sint_t)(n))*(sexp_sint_t)((sexp_sint_t)1<<SEXP_FIXNUM_BITS)) | SEXP_FIXNUM_TAG))
#define sexp_unbox_fixnum(n)   (((sexp_sint_t)((sexp_uint_t)(n) & ~SEXP_FIXNUM_TAG))/(sexp_sint_t)((sexp_sint_t)1<<SEXP_FIXNUM_BITS))
#endif

#define SEXP_NEG_ONE sexp_make_fixnum(-1)
#define SEXP_ZERO    sexp_make_fixnum(0)
#define SEXP_ONE     sexp_make_fixnum(1)
#define SEXP_TWO     sexp_make_fixnum(2)
#define SEXP_THREE   sexp_make_fixnum(3)
#define SEXP_FOUR    sexp_make_fixnum(4)
#define SEXP_FIVE    sexp_make_fixnum(5)
#define SEXP_SIX     sexp_make_fixnum(6)
#define SEXP_SEVEN   sexp_make_fixnum(7)
#define SEXP_EIGHT   sexp_make_fixnum(8)
#define SEXP_NINE    sexp_make_fixnum(9)
#define SEXP_TEN     sexp_make_fixnum(10)

#if SEXP_USE_DISJOINT_STRING_CURSORS
#if SEXP_USE_SIGNED_SHIFTS
#define sexp_make_string_cursor(n)    ((sexp) ((((sexp_sint_t)(n))<<SEXP_STRING_CURSOR_BITS) + SEXP_STRING_CURSOR_TAG))
#define sexp_unbox_string_cursor(n)   (((sexp_sint_t)(n))>>SEXP_STRING_CURSOR_BITS)
#else
#define sexp_make_string_cursor(n)    ((sexp) ((((sexp_sint_t)(n))*(sexp_sint_t)(1uL<<SEXP_STRING_CURSOR_BITS)) | SEXP_STRING_CURSOR_TAG))
#define sexp_unbox_string_cursor(n)   (((sexp_sint_t)((sexp_uint_t)(n) & ~SEXP_STRING_CURSOR_TAG))/(sexp_sint_t)(1uL<<SEXP_STRING_CURSOR_BITS))
#endif
#define sexp_string_cursor_to_fixnum(n) sexp_make_fixnum(sexp_unbox_string_cursor(n))
#define sexp_fixnum_to_string_cursor(n) sexp_make_string_cursor(sexp_unbox_fixnum(n))
#else
#define sexp_make_string_cursor(n)    sexp_make_fixnum(n)
#define sexp_unbox_string_cursor(n)   sexp_unbox_fixnum(n)
#define sexp_string_cursor_to_fixnum(n) (n)
#define sexp_fixnum_to_string_cursor(n) (n)
#endif

#define sexp_make_character(n)  ((sexp) ((((sexp_sint_t)(n))<<SEXP_EXTENDED_BITS) + SEXP_CHAR_TAG))
#define sexp_unbox_character(n) ((int) (((sexp_sint_t)(n))>>SEXP_EXTENDED_BITS))

#define sexp_make_reader_label(n)  ((sexp) ((((sexp_sint_t)(n))<<SEXP_EXTENDED_BITS) + SEXP_READER_LABEL_TAG))
#define sexp_unbox_reader_label(n) ((int) (((sexp_sint_t)(n))>>SEXP_EXTENDED_BITS))

#define sexp_fixnum_to_double(x) ((double)sexp_unbox_fixnum(x))

#if SEXP_USE_PLACEHOLDER_DIGITS
#define sexp_placeholder_digit_p(c) ((c) == SEXP_PLACEHOLDER_DIGIT)
#else
#define sexp_placeholder_digit_p(c) 0
#endif

#define sexp_placeholder_digit_value(base) ((base)/2)

#if SEXP_USE_FLONUMS
#define sexp_fp_integerp(x) (sexp_flonum_value(x) == trunc(sexp_flonum_value(x)))
#define _or_integer_flonump(x) || (sexp_flonump(x) && sexp_fp_integerp(x))
#else
#define _or_integer_flonump(x)
#endif

#if SEXP_USE_BIGNUMS
SEXP_API sexp sexp_make_integer_from_lsint(sexp ctx, sexp_lsint_t x);
SEXP_API sexp sexp_make_unsigned_integer_from_luint(sexp ctx, sexp_luint_t x);
#if SEXP_USE_CUSTOM_LONG_LONGS
SEXP_API sexp sexp_make_integer(sexp ctx, long long x);
SEXP_API sexp sexp_make_unsigned_integer(sexp ctx, unsigned long long x);
#else
SEXP_API sexp sexp_make_integer(sexp ctx, sexp_lsint_t x);
SEXP_API sexp sexp_make_unsigned_integer(sexp ctx, sexp_luint_t x);
#endif
#define sexp_exact_integerp(x) (sexp_fixnump(x) || sexp_bignump(x))
#else
#define sexp_make_integer(ctx, x) sexp_make_fixnum(x)
#define sexp_make_unsigned_integer(ctx, x) sexp_make_fixnum(x)
#define sexp_exact_integerp(x) sexp_fixnump(x)
#endif

#define sexp_integerp(x) (sexp_exact_integerp(x) _or_integer_flonump(x))

#if SEXP_USE_RATIOS
#define sexp_exactp(x) (sexp_exact_integerp(x) || sexp_ratiop(x))
#else
#define sexp_exactp(x) sexp_exact_integerp(x)
#endif

#if SEXP_USE_FLONUMS
#define sexp_fixnum_to_flonum(ctx, x) (sexp_make_flonum(ctx, sexp_unbox_fixnum(x)))
#if SEXP_USE_RATIOS
#define sexp_realp(x) (sexp_exact_integerp(x) || sexp_flonump(x) || sexp_ratiop(x))
#else
#define sexp_realp(x) (sexp_exact_integerp(x) || sexp_flonump(x))
#endif
#else
#define sexp_fixnum_to_flonum(ctx, x) (x)
#define sexp_realp(x) sexp_exact_integerp(x)
#endif

#if SEXP_USE_COMPLEX
#define sexp_numberp(x) (sexp_realp(x) || sexp_complexp(x))
#define sexp_real_part(x) (sexp_complexp(x) ? sexp_complex_real(x) : x)
#define sexp_imag_part(x) (sexp_complexp(x) ? sexp_complex_imag(x) : SEXP_ZERO)
#else
#define sexp_numberp(x) (sexp_realp(x))
#define sexp_real_part(x) (x)
#define sexp_imag_part(x) SEXP_ZERO
#endif

#define sexp_exact_negativep(x) (sexp_fixnump(x) ? (sexp_unbox_fixnum(x) < 0) \
                                 : ((SEXP_USE_BIGNUMS && sexp_bignump(x)) \
                                    && (sexp_bignum_sign(x) < 0)))
#define sexp_exact_positivep(x) (sexp_fixnump(x) ? (sexp_unbox_fixnum(x) > 0) \
                                 : ((SEXP_USE_BIGNUMS && sexp_bignump(x)) \
                                    && (sexp_bignum_sign(x) > 0)))
#define sexp_negativep(x) (sexp_exact_negativep(x) ||                   \
                           (sexp_flonump(x) && sexp_flonum_value(x) < 0))
#define sexp_positivep(x) (!(sexp_negativep(x)))
#define sexp_pedantic_negativep(x) (                 \
  sexp_exact_negativep(x) ||                         \
  (sexp_ratiop(x) &&                                 \
   sexp_exact_negativep(sexp_ratio_numerator(x))) || \
  (sexp_flonump(x) &&                                \
   ((sexp_flonum_value(x) < 0) ||                    \
    (sexp_flonum_value(x) == 0 &&                    \
     1.0 / sexp_flonum_value(x) < 0))))

#if SEXP_USE_BIGNUMS
#define sexp_oddp(x) (sexp_fixnump(x) ? sexp_unbox_fixnum(x) & 1 : \
                      sexp_bignump(x) && (sexp_bignum_data(x)[0] & 1))
#else
#define sexp_oddp(x) (sexp_fixnump(x) && (sexp_unbox_fixnum(x) & 1))
#endif
#define sexp_evenp(x) (!(sexp_oddp(x)))

#define sexp_negate_exact(x)                            \
  if (sexp_bignump(x))                                  \
    sexp_bignum_sign(x) = -sexp_bignum_sign(x);         \
  else if (sexp_fixnump(x))                             \
    x = sexp_fx_neg(x);

#if SEXP_USE_IMMEDIATE_FLONUMS
#define sexp_negate_flonum(x) (x) = sexp_make_flonum(NULL, -(sexp_flonum_value(x)))
#else
#define sexp_negate_flonum(x) sexp_flonum_value(x) = -(sexp_flonum_value(x))
#endif

/* TODO: Doesn't support x == SEXP_MIN_FIXNUM. */
#define sexp_negate(x)                                  \
  if (sexp_flonump(x))                                  \
    sexp_negate_flonum(x);                              \
  else                                                  \
    sexp_negate_exact(x)

#define sexp_negate_maybe_ratio(x)                      \
  if (sexp_ratiop(x)) {                                 \
    sexp_negate_exact(sexp_ratio_numerator(x));         \
  } else {                                              \
    sexp_negate(x);                                     \
  }

#if SEXP_USE_FLONUMS || SEXP_USE_BIGNUMS

#if SEXP_64_BIT
#define sexp_bignum_to_sint(x) (sexp_bignum_sign(x)*sexp_bignum_data(x)[0])
#define sexp_bignum_to_uint(x) (sexp_bignum_data(x)[0])
#else
SEXP_API long long sexp_bignum_to_sint(sexp x);
SEXP_API unsigned long long sexp_bignum_to_uint(sexp x);
#endif

#define sexp_uint_value(x) ((unsigned long long)(sexp_fixnump(x) ? sexp_unbox_fixnum(x) : sexp_bignump(x) ? sexp_bignum_to_uint(x) : 0))
#define sexp_sint_value(x) ((long long)(sexp_fixnump(x) ? sexp_unbox_fixnum(x) : sexp_bignump(x) ? sexp_bignum_to_sint(x) : 0))

#else

#define sexp_uint_value(x) ((sexp_uint_t)sexp_unbox_fixnum(x))
#define sexp_sint_value(x) ((sexp_sint_t)sexp_unbox_fixnum(x))

#endif	/* SEXP_USE_FLONUMS || SEXP_USE_BIGNUMS */

#define sexp_shift_epoch(x) ((x)-SEXP_EPOCH_OFFSET)
#define sexp_unshift_epoch(x) ((x)+SEXP_EPOCH_OFFSET)

#define sexp_infp(x) (sexp_flonump(x) && isinf(sexp_flonum_value(x)))
#define sexp_nanp(x) (sexp_flonump(x) && isnan(sexp_flonum_value(x)))

#if SEXP_USE_IEEE_EQV
#define sexp_flonum_eqv(x, y) (memcmp(sexp_flonum_bits(x), sexp_flonum_bits(y), sizeof(double)) == 0)
#else
#define sexp_flonum_eqv(x, y) (sexp_flonum_value(x) == sexp_flonum_value(y))
#endif

#if SEXP_USE_MINI_FLOAT_UNIFORM_VECTORS
SEXP_API double sexp_quarter_to_double(unsigned char q);
SEXP_API unsigned char sexp_double_to_quarter(double f);
SEXP_API double sexp_half_to_double(unsigned short x);
SEXP_API unsigned short sexp_double_to_half(double x);
#endif

/*************************** field accessors **************************/

#if SEXP_USE_SAFE_ACCESSORS
#if 0
#define sexp_field(x, type, id, field) (*(((x) && sexp_check_tag(x, id)) ? &((x)->value.type.field) : (fprintf(stderr, "invalid field access in %s line %d: %p (%d) isn't a "#type"\n", __FILE__, __LINE__, x, (int)(sexp_pointerp(x) ? sexp_pointer_tag(x) : -1)), &(((sexp)NULL)->value.type.field))))
#define sexp_pred_field(x, type, pred, field) (*(((x) && pred(x)) ? &((x)->value.type.field) : (fprintf(stderr, "invalid field access in %s line %d: %p (%d) isn't a "#type"\n", __FILE__, __LINE__, x, (int)(sexp_pointerp(x) ? sexp_pointer_tag(x) : -1)), &(((sexp)NULL)->value.type.field))))
#define sexp_cpointer_field(x, field) (*(((x) && sexp_pointerp(x) && sexp_pointer_tag(x) >= SEXP_CPOINTER) ? &((x)->value.cpointer.field) : (fprintf(stderr, "invalid field access in %s line %d: %p (%d) isn't a cpointer\n", __FILE__, __LINE__, x, (int)(sexp_pointerp(x) ? sexp_pointer_tag(x) : -1)), &(((sexp)NULL)->value.cpointer.field))))
#else
#define sexp_field(x, type, id, field) (*({sexp _x=x; (((_x) && sexp_check_tag(_x, id)) ? &((_x)->value.type.field) : (fprintf(stderr, "invalid field access in %s line %d: %p (%d) isn't a "#type"\n", __FILE__, __LINE__, _x, (int)(sexp_pointerp(_x) ? sexp_pointer_tag(_x) : -1)), &(((sexp)NULL)->value.type.field)));}))
#define sexp_pred_field(x, type, pred, field) (*({sexp _x=x; (((_x) && pred(_x)) ? &((_x)->value.type.field) : (fprintf(stderr, "invalid field access in %s line %d: %p (%d) isn't a "#type"\n", __FILE__, __LINE__, _x, (int)(sexp_pointerp(_x) ? sexp_pointer_tag(_x) : -1)), &(((sexp)NULL)->value.type.field)));}))
#define sexp_cpointer_field(x, field) (*({sexp _x=x; (((_x) && sexp_pointerp(_x) && sexp_pointer_tag(_x) >= SEXP_CPOINTER) ? &((_x)->value.cpointer.field) : (fprintf(stderr, "invalid field access in %s line %d: %p (%d) isn't a cpointer\n", __FILE__, __LINE__, _x, (int)(sexp_pointerp(_x) ? sexp_pointer_tag(_x) : -1)), &(((sexp)NULL)->value.cpointer.field)));}))
#endif
#else
#define sexp_field(x, type, id, field) ((x)->value.type.field)
#define sexp_pred_field(x, type, pred, field) ((x)->value.type.field)
#define sexp_cpointer_field(x, field) ((x)->value.cpointer.field)
#endif

#define sexp_flexible_array_field(x, type, field_type) \
  ((field_type*)((char*)(x)+sexp_sizeof(type)))

#define sexp_vector_length(x) (sexp_field(x, vector, SEXP_VECTOR, length))
#define sexp_vector_data(x)   sexp_flexible_array_field(x, vector, sexp)

#if SEXP_USE_SAFE_VECTOR_ACCESSORS
#define sexp_vector_ref(x,i)   (sexp_unbox_fixnum(i)>=0 && sexp_unbox_fixnum(i)<sexp_vector_length(x) ? sexp_vector_data(x)[sexp_unbox_fixnum(i)] : (fprintf(stderr, "vector-ref length out of range %s on line %d: vector %p (length %lu): %ld\n", __FILE__, __LINE__, x, sexp_vector_length(x), sexp_unbox_fixnum(i)), SEXP_VOID))
#define sexp_vector_set(x,i,v) (sexp_unbox_fixnum(i)>=0 && sexp_unbox_fixnum(i)<sexp_vector_length(x) ? sexp_vector_data(x)[sexp_unbox_fixnum(i)]=(v) : (fprintf(stderr, "vector-set! length out of range in %s on line %d: vector %p (length %lu): %ld\n", __FILE__, __LINE__, x, sexp_vector_length(x), sexp_unbox_fixnum(i)), SEXP_VOID))
#else
#define sexp_vector_ref(x,i)   (sexp_vector_data(x)[sexp_unbox_fixnum(i)])
#define sexp_vector_set(x,i,v) (sexp_vector_data(x)[sexp_unbox_fixnum(i)]=(v))
#endif

#define sexp_procedure_num_args(x)   (sexp_field(x, procedure, SEXP_PROCEDURE, num_args))
#define sexp_procedure_flags(x)      (sexp_field(x, procedure, SEXP_PROCEDURE, flags))
#define sexp_procedure_variadic_p(x) (sexp_unbox_fixnum(sexp_procedure_flags(x)) & SEXP_PROC_VARIADIC)
#define sexp_procedure_unused_rest_p(x) (sexp_unbox_fixnum(sexp_procedure_flags(x)) & SEXP_PROC_UNUSED_REST)
#define sexp_procedure_variable_transformer_p(x) (sexp_unbox_fixnum(sexp_procedure_flags(x)) & SEXP_PROC_VARIABLE_TRANSFORMER)
#define sexp_procedure_code(x)       (sexp_field(x, procedure, SEXP_PROCEDURE, bc))
#define sexp_procedure_vars(x)       (sexp_field(x, procedure, SEXP_PROCEDURE, vars))
#define sexp_procedure_source(x)     sexp_bytecode_source(sexp_procedure_code(x))

#define sexp_bytes_length(x)  (sexp_field(x, bytes, SEXP_BYTES, length))
#define sexp_bytes_data(x)   sexp_flexible_array_field(x, bytes, char)
#define sexp_bytes_maybe_null_data(x) (sexp_not(x) ? NULL : sexp_bytes_data(x))

static const unsigned char sexp_uvector_sizes[] = {
  0, 1, 8, 8, 16, 16, 32, 32, 64, 64, 32, 64, 64, 128, 8, 16};
static const unsigned char sexp_uvector_chars[] = "#ususususuffccff";

enum sexp_uniform_vector_type {
  SEXP_NOT_A_UNIFORM_TYPE,
  SEXP_U1,
  SEXP_S8,
  SEXP_U8,
  SEXP_S16,
  SEXP_U16,
  SEXP_S32,
  SEXP_U32,
  SEXP_S64,
  SEXP_U64,
  SEXP_F32,
  SEXP_F64,
  SEXP_C64,
  SEXP_C128,
  SEXP_F8,
  SEXP_F16,
  SEXP_END_OF_UNIFORM_TYPES
};

#define sexp_uvector_freep(x) (sexp_freep(x))
#define sexp_uvector_element_size(uvt) (sexp_uvector_sizes[uvt])
#define sexp_uvector_prefix(uvt) (sexp_uvector_chars[uvt])

#define sexp_uvector_length(x) (sexp_field(x, uvector, SEXP_UNIFORM_VECTOR, length))
#define sexp_uvector_type(x)   (sexp_field(x, uvector, SEXP_UNIFORM_VECTOR, element_type))
#define sexp_uvector_data(x) sexp_bytes_data(sexp_uvector_bytes(x))
#define sexp_uvector_maybe_null_data(x) (sexp_not(x) ? NULL : sexp_uvector_data(x))
#define sexp_uvector_bytes(x)  (sexp_field(x, uvector, SEXP_UNIFORM_VECTOR, bytes))

#define sexp_bit_ref(u1v, i)    (((sexp_uvector_data(u1v)[i/8])>>(i%8))&1)
#define sexp_bit_set(u1v, i, x) (x ? (sexp_uvector_data(u1v)[i/8]|=(1<<(i%8))) : (sexp_uvector_data(u1v)[i/8]&=~(1<<(i%8))))

#define sexp_string_size(x)     (sexp_field(x, string, SEXP_STRING, length))
#define sexp_string_charlens(x) (sexp_field(x, string, SEXP_STRING, charlens))
#if SEXP_USE_PACKED_STRINGS
#define sexp_string_data(x)   sexp_flexible_array_field(x, string, char)
#define sexp_string_bytes(x)  (x)
#else
#define sexp_string_bytes(x)  (sexp_field(x, string, SEXP_STRING, bytes))
#define sexp_string_offset(x) (sexp_field(x, string, SEXP_STRING, offset))
#define sexp_string_data(x)   (sexp_bytes_data(sexp_string_bytes(x))+sexp_string_offset(x))
#endif
#if SEXP_USE_STRING_REF_CACHE
#define sexp_cached_char_idx(x) (sexp_field(x, string, SEXP_STRING, cached_char_idx))
#define sexp_cached_cursor(x) (sexp_field(x, string, SEXP_STRING, cached_cursor))
#endif
#define sexp_string_maybe_null_data(x) (sexp_not(x) ? NULL : sexp_string_data(x))

#if SEXP_USE_PACKED_STRINGS
#define sexp_string_to_bytes(ctx, x)  ((x)->tag = SEXP_BYTES, x)
#else
#define sexp_string_to_bytes(ctx, x)  sexp_string_bytes(x)
#endif

#define sexp_bytes_ref(x, i)    (sexp_make_fixnum((unsigned char)sexp_bytes_data(x)[sexp_unbox_fixnum(i)]))
#define sexp_bytes_set(x, i, v) (sexp_bytes_data(x)[sexp_unbox_fixnum(i)] = sexp_unbox_fixnum(v))

#define sexp_lsymbol_data(x)   sexp_flexible_array_field(x, symbol, char)
#define sexp_lsymbol_length(x) (sexp_field(x, symbol, SEXP_SYMBOL, length))

#define sexp_port_stream(p)     (sexp_pred_field(p, port, sexp_portp, stream))
#define sexp_port_name(p)       (sexp_pred_field(p, port, sexp_portp, name))
#define sexp_port_line(p)       (sexp_pred_field(p, port, sexp_portp, line))
#define sexp_port_openp(p)      (sexp_pred_field(p, port, sexp_portp, openp))
#define sexp_port_bidirp(p)     (sexp_pred_field(p, port, sexp_portp, bidirp))
#define sexp_port_binaryp(p)    (sexp_pred_field(p, port, sexp_portp, binaryp))
#define sexp_port_shutdownp(p)  (sexp_pred_field(p, port, sexp_portp, shutdownp))
#define sexp_port_no_closep(p)  (sexp_pred_field(p, port, sexp_portp, no_closep))
#define sexp_port_sourcep(p)    (sexp_pred_field(p, port, sexp_portp, sourcep))
#define sexp_port_blockedp(p)   (sexp_pred_field(p, port, sexp_portp, blockedp))
#define sexp_port_fold_casep(p) (sexp_pred_field(p, port, sexp_portp, fold_casep))
#define sexp_port_cookie(p)     (sexp_pred_field(p, port, sexp_portp, cookie))
#define sexp_port_buf(p)        (sexp_pred_field(p, port, sexp_portp, buf))
#define sexp_port_size(p)       (sexp_pred_field(p, port, sexp_portp, size))
#define sexp_port_offset(p)     (sexp_pred_field(p, port, sexp_portp, offset))
#define sexp_port_flags(p)      (sexp_pred_field(p, port, sexp_portp, flags))
#define sexp_port_fd(p)         (sexp_pred_field(p, port, sexp_portp, fd))

#define sexp_fileno_fd(f)        (sexp_pred_field(f, fileno, sexp_filenop, fd))
#define sexp_fileno_count(f)     (sexp_pred_field(f, fileno, sexp_filenop, count))
#define sexp_fileno_openp(f)     (sexp_pred_field(f, fileno, sexp_filenop, openp))
#define sexp_fileno_socketp(f)   (sexp_pred_field(f, fileno, sexp_filenop, socketp))
#define sexp_fileno_no_closep(f) (sexp_pred_field(f, fileno, sexp_filenop, no_closep))

#define sexp_ratio_numerator(q)   (sexp_pred_field(q, ratio, sexp_ratiop, numerator))
#define sexp_ratio_denominator(q) (sexp_pred_field(q, ratio, sexp_ratiop, denominator))

#define sexp_complex_real(q)   (sexp_pred_field(q, complex, sexp_complexp, real))
#define sexp_complex_imag(q)   (sexp_pred_field(q, complex, sexp_complexp, imag))

#define sexp_exception_kind(x)      (sexp_field(x, exception, SEXP_EXCEPTION, kind))
#define sexp_exception_message(x)   (sexp_field(x, exception, SEXP_EXCEPTION, message))
#define sexp_exception_irritants(x) (sexp_field(x, exception, SEXP_EXCEPTION, irritants))
#define sexp_exception_procedure(x) (sexp_field(x, exception, SEXP_EXCEPTION, procedure))
#define sexp_exception_source(x)    (sexp_field(x, exception, SEXP_EXCEPTION, source))
#define sexp_exception_stack_trace(x) (sexp_field(x, exception, SEXP_EXCEPTION, stack_trace))

#define sexp_trampolinep(x) (sexp_exceptionp(x) && sexp_exception_kind(x) == SEXP_TRAMPOLINE)
#define sexp_trampoline_procedure(x) sexp_exception_procedure(x)
#define sexp_trampoline_args(x) sexp_exception_irritants(x)
#define sexp_trampoline_abortp(x) (sexp_exception_message(x) == SEXP_TRAMPOLINE)

#define sexp_cpointer_freep(x)      (sexp_freep(x))
#define sexp_cpointer_length(x)     (sexp_cpointer_field(x, length))
#define sexp_cpointer_parent(x)     (sexp_cpointer_field(x, parent))
#define sexp_cpointer_value(x)      (sexp_cpointer_field(x, value))
#define sexp_cpointer_maybe_null_value(x) (sexp_not(x) ? NULL : sexp_cpointer_value(x))

#define sexp_bytecode_length(x)   (sexp_field(x, bytecode, SEXP_BYTECODE, length))
#define sexp_bytecode_max_depth(x) (sexp_field(x, bytecode, SEXP_BYTECODE, max_depth))
#define sexp_bytecode_name(x)     (sexp_field(x, bytecode, SEXP_BYTECODE, name))
#define sexp_bytecode_literals(x) (sexp_field(x, bytecode, SEXP_BYTECODE, literals))
#define sexp_bytecode_source(x)   (sexp_field(x, bytecode, SEXP_BYTECODE, source))
#define sexp_bytecode_data(x)     sexp_flexible_array_field(x, bytecode, unsigned char)

#define sexp_env_cell_syntactic_p(x)   ((x)->syntacticp)

#define sexp_env_syntactic_p(x)   ((x)->syntacticp)
#define sexp_env_parent(x)        (sexp_field(x, env, SEXP_ENV, parent))
#define sexp_env_bindings(x)      (sexp_field(x, env, SEXP_ENV, bindings))
#define sexp_env_renames(x)       (sexp_field(x, env, SEXP_ENV, renames))
#define sexp_env_local_p(x)       (sexp_env_parent(x))
#define sexp_env_global_p(x)      (! sexp_env_local_p(x))
#define sexp_env_lambda(x)        (sexp_field(x, env, SEXP_ENV, lambda))

#define sexp_macro_proc(x)        (sexp_field(x, macro, SEXP_MACRO, proc))
#define sexp_macro_env(x)         (sexp_field(x, macro, SEXP_MACRO, env))
#define sexp_macro_source(x)      (sexp_field(x, macro, SEXP_MACRO, source))
#define sexp_macro_aux(x)         (sexp_field(x, macro, SEXP_MACRO, aux))

#define sexp_synclo_env(x)        (sexp_field(x, synclo, SEXP_SYNCLO, env))
#define sexp_synclo_free_vars(x)  (sexp_field(x, synclo, SEXP_SYNCLO, free_vars))
#define sexp_synclo_expr(x)       (sexp_field(x, synclo, SEXP_SYNCLO, expr))
#define sexp_synclo_rename(x)     (sexp_field(x, synclo, SEXP_SYNCLO, rename))

#define sexp_core_code(x)         (sexp_field(x, core, SEXP_CORE, code))
#define sexp_core_name(x)         (sexp_field(x, core, SEXP_CORE, name))

#define sexp_dl_file(x)            (sexp_field(x, dl, SEXP_DL, file))
#define sexp_dl_handle(x)          (sexp_field(x, dl, SEXP_DL, handle))

#define sexp_opcode_class(x)       (sexp_field(x, opcode, SEXP_OPCODE, op_class))
#define sexp_opcode_code(x)        (sexp_field(x, opcode, SEXP_OPCODE, code))
#define sexp_opcode_num_args(x)    (sexp_field(x, opcode, SEXP_OPCODE, num_args))
#define sexp_opcode_flags(x)       (sexp_field(x, opcode, SEXP_OPCODE, flags))
#define sexp_opcode_inverse(x)     (sexp_field(x, opcode, SEXP_OPCODE, inverse))
#define sexp_opcode_dl(x)          (sexp_field(x, opcode, SEXP_OPCODE, dl))
#define sexp_opcode_name(x)        (sexp_field(x, opcode, SEXP_OPCODE, name))
#define sexp_opcode_data(x)        (sexp_field(x, opcode, SEXP_OPCODE, data))
#define sexp_opcode_data2(x)       (sexp_field(x, opcode, SEXP_OPCODE, data2))
#define sexp_opcode_proc(x)        (sexp_field(x, opcode, SEXP_OPCODE, proc))
#define sexp_opcode_return_type(x) (sexp_field(x, opcode, SEXP_OPCODE, ret_type))
#define sexp_opcode_arg1_type(x)   (sexp_field(x, opcode, SEXP_OPCODE, arg1_type))
#define sexp_opcode_arg2_type(x)   (sexp_field(x, opcode, SEXP_OPCODE, arg2_type))
#define sexp_opcode_arg3_type(x)   (sexp_field(x, opcode, SEXP_OPCODE, arg3_type))
#define sexp_opcode_argn_type(x)   (sexp_field(x, opcode, SEXP_OPCODE, argn_type))
#define sexp_opcode_methods(x)     (sexp_field(x, opcode, SEXP_OPCODE, methods))
#define sexp_opcode_func(x)        (sexp_field(x, opcode, SEXP_OPCODE, func))

#define sexp_opcode_variadic_p(x)  (sexp_opcode_flags(x) & 1)
#define sexp_opcode_opt_param_p(x) (sexp_opcode_flags(x) & 2)
#define sexp_opcode_ref_trans_p(x) (sexp_opcode_flags(x) & 4)
#define sexp_opcode_static_param_p(x) (sexp_opcode_flags(x) & 8)
#define sexp_opcode_tail_call_p(x) (sexp_opcode_flags(x) & 16)

#define sexp_lambda_name(x)        (sexp_field(x, lambda, SEXP_LAMBDA, name))
#define sexp_lambda_params(x)      (sexp_field(x, lambda, SEXP_LAMBDA, params))
#define sexp_lambda_locals(x)      (sexp_field(x, lambda, SEXP_LAMBDA, locals))
#define sexp_lambda_defs(x)        (sexp_field(x, lambda, SEXP_LAMBDA, defs))
#define sexp_lambda_flags(x)       (sexp_field(x, lambda, SEXP_LAMBDA, flags))
#define sexp_lambda_body(x)        (sexp_field(x, lambda, SEXP_LAMBDA, body))
#define sexp_lambda_fv(x)          (sexp_field(x, lambda, SEXP_LAMBDA, fv))
#define sexp_lambda_sv(x)          (sexp_field(x, lambda, SEXP_LAMBDA, sv))
#define sexp_lambda_return_type(x) (sexp_field(x, lambda, SEXP_LAMBDA, ret))
#define sexp_lambda_param_types(x) (sexp_field(x, lambda, SEXP_LAMBDA, types))
#define sexp_lambda_source(x)      (sexp_field(x, lambda, SEXP_LAMBDA, source))

#define sexp_cnd_test(x)      (sexp_field(x, cnd, SEXP_CND, test))
#define sexp_cnd_pass(x)      (sexp_field(x, cnd, SEXP_CND, pass))
#define sexp_cnd_fail(x)      (sexp_field(x, cnd, SEXP_CND, fail))
#define sexp_cnd_source(x)    (sexp_field(x, cnd, SEXP_CND, source))

#define sexp_set_var(x)       (sexp_field(x, set, SEXP_SET, var))
#define sexp_set_value(x)     (sexp_field(x, set, SEXP_SET, value))
#define sexp_set_source(x)    (sexp_field(x, set, SEXP_SET, source))

#define sexp_set_syn_var(x)    (sexp_field(x, set, SEXP_SET_SYN, var))
#define sexp_set_syn_value(x)  (sexp_field(x, set, SEXP_SET_SYN, value))
#define sexp_set_syn_source(x) (sexp_field(x, set, SEXP_SET_SYN, source))

#define sexp_ref_name(x)      (sexp_field(x, ref, SEXP_REF, name))
#define sexp_ref_cell(x)      ((x)->value.ref.cell)
#define sexp_ref_loc(x)       (sexp_cdr(sexp_ref_cell(x)))
#define sexp_ref_source(x)    (sexp_field(x, ref, SEXP_REF, source))

#define sexp_seq_ls(x)        (sexp_field(x, seq, SEXP_SEQ, ls))
#define sexp_seq_source(x)    (sexp_field(x, seq, SEXP_SEQ, source))

#define sexp_lit_value(x)     (sexp_field(x, lit, SEXP_LIT, value))
#define sexp_lit_source(x)    (sexp_field(x, lit, SEXP_LIT, source))

#define sexp_stack_length(x)  (sexp_field(x, stack, SEXP_STACK, length))
#define sexp_stack_top(x)     (sexp_field(x, stack, SEXP_STACK, top))
#define sexp_stack_data(x)    sexp_flexible_array_field(x, stack, sexp)

#define sexp_promise_donep(x) (sexp_field(x, promise, SEXP_PROMISE, donep))
#define sexp_promise_value(x) (sexp_field(x, promise, SEXP_PROMISE, value))

#define sexp_ephemeron_key(x)   (sexp_field(x, ephemeron, SEXP_EPHEMERON, key))
#define sexp_ephemeron_value(x) (sexp_field(x, ephemeron, SEXP_EPHEMERON, value))

#define sexp_context_env(x)      (sexp_field(x, context, SEXP_CONTEXT, env))
#define sexp_context_stack(x)    (sexp_field(x, context, SEXP_CONTEXT, stack))
#define sexp_context_parent(x)   (sexp_field(x, context, SEXP_CONTEXT, parent))
#define sexp_context_child(x)    (sexp_field(x, context, SEXP_CONTEXT, child))
#define sexp_context_mark_stack(x)     (sexp_field(x, context, SEXP_CONTEXT, mark_stack))
#define sexp_context_mark_stack_ptr(x) (sexp_field(x, context, SEXP_CONTEXT, mark_stack_ptr))
#define sexp_context_saves(x)    (sexp_field(x, context, SEXP_CONTEXT, saves))
#define sexp_context_tailp(x)    (sexp_field(x, context, SEXP_CONTEXT, tailp))
#define sexp_context_tracep(x)   (sexp_field(x, context, SEXP_CONTEXT, tracep))
#define sexp_context_globals(x)  (sexp_field(x, context, SEXP_CONTEXT, globals))
#define sexp_context_dk(x)       (sexp_field(x, context, SEXP_CONTEXT, dk))
#define sexp_context_params(x)   (sexp_field(x, context, SEXP_CONTEXT, params))
#define sexp_context_last_fp(x)  (sexp_field(x, context, SEXP_CONTEXT, last_fp))
#define sexp_context_gc_count(x) (sexp_field(x, context, SEXP_CONTEXT, gc_count))
#if SEXP_USE_TIME_GC
#define sexp_context_gc_usecs(x) (sexp_field(x, context, SEXP_CONTEXT, gc_usecs))
#else
#define sexp_context_gc_usecs(x) 0
#endif
#if SEXP_USE_TRACK_ALLOC_TIMES
#define sexp_context_alloc_count(x) (sexp_field(x, context, SEXP_CONTEXT, alloc_count))
#define sexp_context_alloc_usecs(x) (sexp_field(x, context, SEXP_CONTEXT, alloc_usecs))
#define sexp_context_alloc_usecs_sq(x) (sexp_field(x, context, SEXP_CONTEXT, alloc_usecs_sq))
#endif
#if SEXP_USE_TRACK_ALLOC_SIZES
#define sexp_context_alloc_histogram(x) (sexp_field(x, context, SEXP_CONTEXT, alloc_histogram))
#endif
#define sexp_context_refuel(x)   (sexp_field(x, context, SEXP_CONTEXT, refuel))
#define sexp_context_ip(x)       (sexp_field(x, context, SEXP_CONTEXT, ip))
#define sexp_context_proc(x)     (sexp_field(x, context, SEXP_CONTEXT, proc))
#define sexp_context_timeval(x)  (sexp_field(x, context, SEXP_CONTEXT, tval))
#define sexp_context_name(x)     (sexp_field(x, context, SEXP_CONTEXT, name))
#define sexp_context_specific(x) (sexp_field(x, context, SEXP_CONTEXT, specific))
#define sexp_context_event(x)    (sexp_field(x, context, SEXP_CONTEXT, event))
#define sexp_context_timeoutp(x) (sexp_field(x, context, SEXP_CONTEXT, timeoutp))
#define sexp_context_waitp(x)    (sexp_field(x, context, SEXP_CONTEXT, waitp))
#define sexp_context_dl(x)       (sexp_field(x, context, SEXP_CONTEXT, dl))

#define sexp_context_result(x)   (sexp_field(x, context, SEXP_CONTEXT, result))
#define sexp_context_errorp(x)   (sexp_field(x, context, SEXP_CONTEXT, errorp))
#define sexp_context_interruptp(x) (sexp_field(x, context, SEXP_CONTEXT, interruptp))

/* during compilation, sexp_context_specific is set to a vector */
/* containing the following elements: */

#define sexp_context_bc(x)       (sexp_vector_ref(sexp_context_specific(x), SEXP_ZERO))
#define sexp_context_fv(x)       (sexp_vector_ref(sexp_context_specific(x), SEXP_ONE))
#define sexp_context_lambda(x)   (sexp_vector_ref(sexp_context_specific(x), SEXP_TWO))
#define sexp_context_pos(x)      (sexp_vector_ref(sexp_context_specific(x), SEXP_THREE))
#define sexp_context_depth(x)    (sexp_vector_ref(sexp_context_specific(x), SEXP_FOUR))
#define sexp_context_max_depth(x) (sexp_vector_ref(sexp_context_specific(x), SEXP_FIVE))
#define sexp_context_exception(x) (sexp_vector_ref(sexp_context_specific(x), SEXP_SIX))

#if SEXP_USE_ALIGNED_BYTECODE
SEXP_API void sexp_context_align_pos(sexp ctx);
#else
#define sexp_context_align_pos(ctx)
#endif

#define sexp_global(ctx,x)      (sexp_vector_data(sexp_context_globals(ctx))[x])

#if SEXP_USE_GLOBAL_HEAP
#if ! SEXP_USE_BOEHM
SEXP_API sexp_heap sexp_global_heap;
#endif
#define sexp_context_heap(ctx)     sexp_global_heap
#define sexp_context_max_size(ctx) 0
#else
#define sexp_context_heap(ctx)     ((ctx)->value.context.heap)
#define sexp_context_max_size(ctx) sexp_context_heap(ctx)->max_size
#endif

#if SEXP_USE_GLOBAL_SYMBOLS
#define sexp_context_symbols(ctx) sexp_symbol_table
SEXP_API sexp sexp_symbol_table[SEXP_SYMBOL_TABLE_SIZE];
#else
#define sexp_context_symbols(ctx) sexp_vector_data(sexp_global(ctx, SEXP_G_SYMBOLS))
#endif

#define sexp_context_types(ctx)    sexp_vector_data(sexp_global(ctx, SEXP_G_TYPES))
#define sexp_type_by_index(ctx,i)  (sexp_context_types(ctx)[i])
#define sexp_context_num_types(ctx)             \
  sexp_unbox_fixnum(sexp_global(ctx, SEXP_G_NUM_TYPES))
#define sexp_context_type_array_size(ctx)                               \
  sexp_vector_length(sexp_global(ctx, SEXP_G_TYPES))

#define sexp_object_type(ctx,x)        (sexp_type_by_index(ctx, ((x)->tag)))
#define sexp_object_type_name(ctx,x)   (sexp_type_name(sexp_object_type(ctx, x)))
#define sexp_type_name_by_index(ctx,i) (sexp_type_name(sexp_type_by_index(ctx,i)))

#define sexp_type_size_of_object(t, x)                                  \
  (((sexp_uint_t*)((char*)x + sexp_type_size_off(t)))[0]                \
   * sexp_type_size_scale(t)                                            \
   + sexp_type_size_base(t))
#define sexp_type_num_slots_of_object(t, x)                             \
  (((sexp_uint_t*)((char*)x + sexp_type_field_len_off(t)))[0]           \
   * sexp_type_field_len_scale(t)                                       \
   + sexp_type_field_len_base(t))
#define sexp_type_num_eq_slots_of_object(t, x)                          \
  (((sexp_uint_t*)((char*)x + sexp_type_field_len_off(t)))[0]           \
   * sexp_type_field_len_scale(t)                                       \
   + sexp_type_field_eq_len_base(t))
#define sexp_type_num_weak_slots_of_object(t, x)                        \
  (((sexp_uint_t*)((char*)x + sexp_type_weak_len_off(t)))[0]            \
   * sexp_type_weak_len_scale(t)                                        \
   + sexp_type_weak_len_base(t))

#define sexp_context_top(x)     (sexp_stack_top(sexp_context_stack(x)))

#define sexp_type_tag(x)               (sexp_field(x, type, SEXP_TYPE, tag))
#define sexp_type_field_base(x)        (sexp_field(x, type, SEXP_TYPE, field_base))
#define sexp_type_field_eq_len_base(x) (sexp_field(x, type, SEXP_TYPE, field_eq_len_base))
#define sexp_type_field_len_base(x)    (sexp_field(x, type, SEXP_TYPE, field_len_base))
#define sexp_type_field_len_off(x)     (sexp_field(x, type, SEXP_TYPE, field_len_off))
#define sexp_type_field_len_scale(x)   (sexp_field(x, type, SEXP_TYPE, field_len_scale))
#define sexp_type_size_base(x)         (sexp_field(x, type, SEXP_TYPE, size_base))
#define sexp_type_size_off(x)          (sexp_field(x, type, SEXP_TYPE, size_off))
#define sexp_type_size_scale(x)        (sexp_field(x, type, SEXP_TYPE, size_scale))
#define sexp_type_weak_base(x)         (sexp_field(x, type, SEXP_TYPE, weak_base))
#define sexp_type_weak_len_base(x)     (sexp_field(x, type, SEXP_TYPE, weak_len_base))
#define sexp_type_weak_len_off(x)      (sexp_field(x, type, SEXP_TYPE, weak_len_off))
#define sexp_type_weak_len_scale(x)    (sexp_field(x, type, SEXP_TYPE, weak_len_scale))
#define sexp_type_weak_len_extra(x)    (sexp_field(x, type, SEXP_TYPE, weak_len_extra))
#define sexp_type_depth(x)             (sexp_field(x, type, SEXP_TYPE, depth))
#define sexp_type_name(x)              (sexp_field(x, type, SEXP_TYPE, name))
#define sexp_type_cpl(x)               (sexp_field(x, type, SEXP_TYPE, cpl))
#define sexp_type_slots(x)             (sexp_field(x, type, SEXP_TYPE, slots))
#define sexp_type_getters(x)           (sexp_field(x, type, SEXP_TYPE, getters))
#define sexp_type_setters(x)           (sexp_field(x, type, SEXP_TYPE, setters))
#define sexp_type_finalize(x)          (sexp_field(x, type, SEXP_TYPE, finalize))
#define sexp_type_finalize_name(x)     (sexp_field(x, type, SEXP_TYPE, finalize_name))
#define sexp_type_print(x)             (sexp_field(x, type, SEXP_TYPE, print))
#define sexp_type_dl(x)                (sexp_field(x, type, SEXP_TYPE, dl))
#define sexp_type_id(x)                (sexp_field(x, type, SEXP_TYPE, id))

#define sexp_bignum_sign(x)            (sexp_field(x, bignum, SEXP_BIGNUM, sign))
#define sexp_bignum_length(x)          (sexp_field(x, bignum, SEXP_BIGNUM, length))
#define sexp_bignum_data(x)            sexp_flexible_array_field(x, bignum, sexp_uint_t)

/****************************** arithmetic ****************************/

#define sexp_fx_add(a, b) ((sexp)(((sexp_sint_t)a)+((sexp_sint_t)b)-SEXP_FIXNUM_TAG))
#define sexp_fx_sub(a, b) ((sexp)(((sexp_sint_t)a)-((sexp_sint_t)b)+SEXP_FIXNUM_TAG))
#define sexp_fx_mul(a, b) ((sexp)((((((sexp_sint_t)a)-SEXP_FIXNUM_TAG)*(((sexp_sint_t)b)>>SEXP_FIXNUM_BITS))+SEXP_FIXNUM_TAG)))
#define sexp_fx_div(a, b) (sexp_make_fixnum(sexp_unbox_fixnum(a) / sexp_unbox_fixnum(b)))
#define sexp_fx_rem(a, b) (sexp_make_fixnum(sexp_unbox_fixnum(a) % sexp_unbox_fixnum(b)))
#define sexp_fx_sign(a)   (+1 | (((sexp_sint_t)(a)) >> (sizeof(sexp_sint_t)*8 - 1)))
#define sexp_fx_neg(a)    (sexp_make_fixnum(-(sexp_unbox_fixnum(a))))
#define sexp_fx_abs(a)    ((((sexp_sint_t)a) < 0) ? sexp_fx_neg(a) : a)

#define sexp_unbox_fx_abs(a) ((((sexp_sint_t)a) < 0) ? -sexp_unbox_fixnum(a) : sexp_unbox_fixnum(a))

#define sexp_fp_add(x,a,b) (sexp_make_flonum(x, sexp_flonum_value(a) + sexp_flonum_value(b)))
#define sexp_fp_sub(x,a,b) (sexp_make_flonum(x, sexp_flonum_value(a) - sexp_flonum_value(b)))
#define sexp_fp_mul(x,a,b) (sexp_make_flonum(x, sexp_flonum_value(a) * sexp_flonum_value(b)))
#define sexp_fp_div(x,a,b) (sexp_make_flonum(x, sexp_flonum_value(a) / sexp_flonum_value(b)))

#if ! (SEXP_USE_FLONUMS || SEXP_USE_BIGNUMS)
#define sexp_add(ctx, a, b) sexp_fx_add(a, b)
#define sexp_sub(ctx, a, b) sexp_fx_sub(a, b)
#define sexp_mul(ctx, a, b) sexp_fx_mul(a, b)
#define sexp_div(ctx, a, b) sexp_fx_div(a, b)
#endif

/****************************** utilities *****************************/

enum sexp_context_globals {
#if SEXP_USE_STABLE_ABI || ! SEXP_USE_GLOBAL_SYMBOLS
  SEXP_G_SYMBOLS,
#endif
  SEXP_G_ENDIANNESS,
  SEXP_G_TYPES,
  SEXP_G_FEATURES,
  SEXP_G_NUM_TYPES,
  SEXP_G_OOM_ERROR,             /* out of memory exception object */
  SEXP_G_OOS_ERROR,             /* out of stack exception object */
  SEXP_G_ABI_ERROR,             /* incompatible ABI loading library */
  SEXP_G_INTERRUPT_ERROR,       /* C-c in the repl */
  SEXP_G_OPTIMIZATIONS,
  SEXP_G_SIGNAL_HANDLERS,
  SEXP_G_META_ENV,
  SEXP_G_MODULE_PATH,
  SEXP_G_QUOTE_SYMBOL,
  SEXP_G_QUASIQUOTE_SYMBOL,
  SEXP_G_UNQUOTE_SYMBOL,
  SEXP_G_UNQUOTE_SPLICING_SYMBOL,
  SEXP_G_SYNTAX_SYMBOL,
  SEXP_G_QUASISYNTAX_SYMBOL,
  SEXP_G_UNSYNTAX_SYMBOL,
  SEXP_G_UNSYNTAX_SPLICING_SYMBOL,
  SEXP_G_EMPTY_VECTOR,
  SEXP_G_CUR_IN_SYMBOL,
  SEXP_G_CUR_OUT_SYMBOL,
  SEXP_G_CUR_ERR_SYMBOL,
  SEXP_G_INTERACTION_ENV_SYMBOL,
  SEXP_G_CONTINUABLE_SYMBOL,
  SEXP_G_ERR_HANDLER,
  SEXP_G_RESUMECC_BYTECODE,
  SEXP_G_FINAL_RESUMER,
  SEXP_G_RANDOM_SOURCE,
  SEXP_G_STRICT_P,
  SEXP_G_NO_TAIL_CALLS_P,
#if SEXP_USE_STABLE_ABI || SEXP_USE_FOLD_CASE_SYMS
  SEXP_G_FOLD_CASE_P,
#endif
#if SEXP_USE_STABLE_ABI || SEXP_USE_WEAK_REFERENCES
  SEXP_G_WEAK_OBJECTS_PRESENT,
  SEXP_G_FILE_DESCRIPTORS,
  SEXP_G_NUM_FILE_DESCRIPTORS,
#endif
#if SEXP_USE_STABLE_ABI || ! SEXP_USE_BOEHM
  SEXP_G_PRESERVATIVES,
#endif
#if SEXP_USE_STABLE_ABI || SEXP_USE_GREEN_THREADS
  SEXP_G_IO_BLOCK_ERROR,
  SEXP_G_IO_BLOCK_ONCE_ERROR,
  SEXP_G_THREAD_TERMINATE_ERROR,
  SEXP_G_THREADS_SCHEDULER,
  SEXP_G_THREADS_FRONT,
  SEXP_G_THREADS_BACK,
  SEXP_G_THREADS_PAUSED,
  SEXP_G_THREADS_SIGNALS,
  SEXP_G_THREADS_SIGNAL_RUNNER,
  SEXP_G_THREADS_POLL_FDS,
  SEXP_G_THREADS_FD_THREADS,
  SEXP_G_THREADS_BLOCKER,
  SEXP_G_THREADS_MUTEX_ID,
  SEXP_G_THREADS_POLLFDS_ID,
  SEXP_G_ATOMIC_P,
#endif
  SEXP_G_NUM_GLOBALS
};

#define sexp_list1(x,a)        sexp_cons((x), (a), SEXP_NULL)

SEXP_API sexp sexp_push_op(sexp ctx, sexp* loc, sexp x);

#if SEXP_USE_UNSAFE_PUSH
#define sexp_push(ctx, ls, x)    ((ls) = sexp_cons((ctx), (x), (ls)))
#else
#define sexp_push(ctx, ls, x)    (sexp_push_op((ctx), &(ls), (x)))
#endif
#define sexp_insert(ctx, ls, x)  ((sexp_memq(ctx, (x), (ls)) != SEXP_FALSE) ? (ls) : sexp_push((ctx), (ls), (x)))

#define sexp_pair_source(x) (sexp_field(x, pair, SEXP_PAIR, source))

#define sexp_car(x)         (sexp_field(x, pair, SEXP_PAIR, car))
#define sexp_cdr(x)         (sexp_field(x, pair, SEXP_PAIR, cdr))

#define sexp_caar(x)      (sexp_car(sexp_car(x)))
#define sexp_cadr(x)      (sexp_car(sexp_cdr(x)))
#define sexp_cdar(x)      (sexp_cdr(sexp_car(x)))
#define sexp_cddr(x)      (sexp_cdr(sexp_cdr(x)))
#define sexp_caaar(x)     (sexp_car(sexp_caar(x)))
#define sexp_caadr(x)     (sexp_car(sexp_cadr(x)))
#define sexp_cadar(x)     (sexp_car(sexp_cdar(x)))
#define sexp_caddr(x)     (sexp_car(sexp_cddr(x)))
#define sexp_cdaar(x)     (sexp_cdr(sexp_caar(x)))
#define sexp_cdadr(x)     (sexp_cdr(sexp_cadr(x)))
#define sexp_cddar(x)     (sexp_cdr(sexp_cdar(x)))
#define sexp_cdddr(x)     (sexp_cdr(sexp_cddr(x)))
#define sexp_cadddr(x)    (sexp_cadr(sexp_cddr(x))) /* just these two */
#define sexp_cddddr(x)    (sexp_cddr(sexp_cddr(x)))

/***************************** general API ****************************/

#define sexp_read_char(x, p) (sexp_port_buf(p) ? ((sexp_port_offset(p) < sexp_port_size(p)) ? ((unsigned char*)sexp_port_buf(p))[sexp_port_offset(p)++] : sexp_buffered_read_char(x, p)) : getc(sexp_port_stream(p)))
#define sexp_push_char(x, c, p) ((c!=EOF) && (sexp_port_buf(p) ? (sexp_port_buf(p)[--sexp_port_offset(p)] = ((char)(c))) : ungetc(c, sexp_port_stream(p))))
#define sexp_write_char(x, c, p) (sexp_port_buf(p) ? ((sexp_port_offset(p) < sexp_port_size(p)) ? ((((sexp_port_buf(p))[sexp_port_offset(p)++]) = (char)(c)), 0) : sexp_buffered_write_char(x, c, p)) : putc(c, sexp_port_stream(p)))
#define sexp_write_string(x, s, p) (sexp_port_buf(p) ? sexp_buffered_write_string(x, s, p) : fputs(s, sexp_port_stream(p)))
#define sexp_write_string_n(x, s, n, p) (sexp_port_buf(p) ? sexp_buffered_write_string_n(x, s, n, p) : fwrite(s, 1, n, sexp_port_stream(p)))
#define sexp_flush(x, p) (sexp_port_buf(p) ? sexp_buffered_flush(x, p, 0) : fflush(sexp_port_stream(p)))
#define sexp_flush_forced(x, p) (sexp_port_buf(p) ? sexp_buffered_flush(x, p, 1) : fflush(sexp_port_stream(p)))

SEXP_API int sexp_buffered_read_char (sexp ctx, sexp p);
SEXP_API int sexp_buffered_write_char (sexp ctx, int c, sexp p);
SEXP_API int sexp_buffered_write_string_n (sexp ctx, const char *str, sexp_uint_t len, sexp p);
SEXP_API int sexp_buffered_write_string (sexp ctx, const char *str, sexp p);
SEXP_API int sexp_buffered_flush (sexp ctx, sexp p, int forcep);

#define sexp_newline(ctx, p) sexp_write_char((ctx), '\n', (p))
#define sexp_at_eofp(p)      (feof(sexp_port_stream(p)))
#define sexp_port_fileno(p)  (sexp_port_stream(p) ? fileno(sexp_port_stream(p)) : sexp_filenop(sexp_port_fd(p)) ? sexp_fileno_fd(sexp_port_fd(p)) : -1)

#if SEXP_USE_AUTOCLOSE_PORTS
#define SEXP_FINALIZE_PORT sexp_finalize_port
#define SEXP_FINALIZE_PORTN (sexp)"sexp_finalize_port"
#define SEXP_FINALIZE_FILENO sexp_finalize_fileno
#define SEXP_FINALIZE_FILENON (sexp)"sexp_finalize_fileno"
#else
#define SEXP_FINALIZE_PORT NULL
#define SEXP_FINALIZE_PORTN NULL
#define SEXP_FINALIZE_FILENO NULL
#define SEXP_FINALIZE_FILENON NULL
#endif

#if SEXP_USE_DL
sexp sexp_finalize_dl (sexp ctx, sexp self, sexp_sint_t n, sexp dl);
#define SEXP_FINALIZE_DL sexp_finalize_dl
#define SEXP_FINALIZE_DLN (sexp)"sexp_finalize_dl"
#else
#define SEXP_FINALIZE_DL NULL
#define SEXP_FINALIZE_DLN NULL
#endif

#if SEXP_USE_TRACK_ALLOC_SOURCE
#define sexp_current_source_param , const char* source
#else
#define sexp_current_source_param
#endif

/* To export a library from the embedding C program to Scheme, so    */
/* that it can be included into Scheme library foo/qux.sld as        */
/* (include-shared "bar"), libraries should contain the entry        */
/* {"foo/bar", init_bar}.  The signature and function of init_bar is */
/* the same as that of sexp_init_library in shared libraries.  The   */
/* array libraries must be terminated with {NULL, NULL} and must     */
/* remain valid throughout its use by Chibi.                         */

SEXP_API void sexp_add_static_libraries(struct sexp_library_entry_t* libraries);

SEXP_API sexp sexp_alloc_tagged_aux(sexp ctx, size_t size, sexp_uint_t tag sexp_current_source_param);
SEXP_API sexp sexp_make_context(sexp ctx, size_t size, size_t max_size);
SEXP_API sexp sexp_cons_op(sexp ctx, sexp self, sexp_sint_t n, sexp head, sexp tail);
SEXP_API sexp sexp_list2(sexp ctx, sexp a, sexp b);
SEXP_API sexp sexp_list3(sexp ctx, sexp a, sexp b, sexp c);
SEXP_API sexp sexp_equalp_bound (sexp ctx, sexp self, sexp_sint_t n, sexp a, sexp b, sexp depth, sexp bound);
SEXP_API sexp sexp_equalp_op (sexp ctx, sexp self, sexp_sint_t n, sexp a, sexp b);
SEXP_API sexp sexp_listp_op(sexp ctx, sexp self, sexp_sint_t n, sexp obj);
SEXP_API sexp sexp_reverse_op(sexp ctx, sexp self, sexp_sint_t n, sexp ls);
SEXP_API sexp sexp_nreverse_op(sexp ctx, sexp self, sexp_sint_t n, sexp ls);
SEXP_API sexp sexp_copy_list_op(sexp ctx, sexp self, sexp_sint_t n, sexp ls);
SEXP_API sexp sexp_append2_op(sexp ctx, sexp self, sexp_sint_t n, sexp a, sexp b);
SEXP_API sexp sexp_memq_op(sexp ctx, sexp self, sexp_sint_t n, sexp x, sexp ls);
SEXP_API sexp sexp_assq_op(sexp ctx, sexp self, sexp_sint_t n, sexp x, sexp ls);
SEXP_API sexp sexp_length_op(sexp ctx, sexp self, sexp_sint_t n, sexp ls);
SEXP_API sexp sexp_c_string(sexp ctx, const char *str, sexp_sint_t slen);
SEXP_API sexp sexp_make_ephemeron_op(sexp ctx, sexp self, sexp_sint_t n, sexp key, sexp value);
SEXP_API sexp sexp_make_bytes_op(sexp ctx, sexp self, sexp_sint_t n, sexp len, sexp i);
SEXP_API sexp sexp_make_uvector_op(sexp ctx, sexp self, sexp_sint_t n, sexp elt_type, sexp len);
SEXP_API sexp sexp_make_string_op(sexp ctx, sexp self, sexp_sint_t n, sexp len, sexp ch);
SEXP_API sexp sexp_substring_op (sexp ctx, sexp self, sexp_sint_t n, sexp str, sexp start, sexp end);
SEXP_API sexp sexp_subbytes_op (sexp ctx, sexp self, sexp_sint_t n, sexp str, sexp start, sexp end);
SEXP_API sexp sexp_string_concatenate_op (sexp ctx, sexp self, sexp_sint_t n, sexp str_ls, sexp sep);
SEXP_API sexp sexp_intern (sexp ctx, const char *str, sexp_sint_t len);
SEXP_API sexp sexp_string_to_symbol_op (sexp ctx, sexp self, sexp_sint_t n, sexp str);
SEXP_API sexp sexp_symbol_to_string_op (sexp ctx, sexp self, sexp_sint_t n, sexp sym);
SEXP_API sexp sexp_string_to_number_op (sexp ctx, sexp self, sexp_sint_t n, sexp str, sexp b);
SEXP_API sexp sexp_flonump_op (sexp ctx, sexp self, sexp_sint_t n, sexp x);
SEXP_API sexp sexp_make_vector_op (sexp ctx, sexp self, sexp_sint_t n, sexp len, sexp dflt);
SEXP_API sexp sexp_list_to_vector_op (sexp ctx, sexp self, sexp_sint_t n, sexp ls);
SEXP_API sexp sexp_list_to_uvector_op (sexp ctx, sexp self, sexp_sint_t n, sexp etype, sexp ls);
SEXP_API sexp sexp_make_cpointer (sexp ctx, sexp_uint_t type_id, void* value, sexp parent, int freep);
SEXP_API int sexp_is_separator(int c);
SEXP_API sexp sexp_write_op (sexp ctx, sexp self, sexp_sint_t n, sexp obj, sexp out);
SEXP_API sexp sexp_flush_output_op (sexp ctx, sexp self, sexp_sint_t n, sexp out);
SEXP_API sexp sexp_read_string (sexp ctx, sexp in, int sentinel);
SEXP_API sexp sexp_read_symbol (sexp ctx, sexp in, int init, int internp);
SEXP_API sexp sexp_read_number (sexp ctx, sexp in, int base, int exactp);
#if SEXP_USE_BIGNUMS
SEXP_API sexp sexp_read_bignum (sexp ctx, sexp in, sexp_uint_t init,
                                signed char sign, sexp_uint_t base);
SEXP_API sexp sexp_write_bignum (sexp ctx, sexp a, sexp out, sexp_uint_t base);
#endif
SEXP_API sexp sexp_read_float_tail(sexp ctx, sexp in, double whole, int negp);
#if SEXP_USE_COMPLEX
SEXP_API sexp sexp_read_complex_tail(sexp ctx, sexp in, sexp res);
#endif
SEXP_API sexp sexp_read_raw (sexp ctx, sexp in, sexp *shares);
SEXP_API sexp sexp_read_op (sexp ctx, sexp self, sexp_sint_t n, sexp in);
SEXP_API sexp sexp_char_ready_p (sexp ctx, sexp self, sexp_sint_t n, sexp in);
SEXP_API sexp sexp_read_from_string (sexp ctx, const char *str, sexp_sint_t len);
SEXP_API sexp sexp_read_error (sexp ctx, const char *msg, sexp ir, sexp port);
SEXP_API sexp sexp_write_to_string (sexp ctx, sexp obj);
SEXP_API sexp sexp_write_simple_object (sexp ctx, sexp self, sexp_sint_t n, sexp obj, sexp writer, sexp out);
SEXP_API sexp sexp_finalize_port (sexp ctx, sexp self, sexp_sint_t n, sexp port);
SEXP_API sexp sexp_make_fileno_op (sexp ctx, sexp self, sexp_sint_t n, sexp fd, sexp no_closep);
SEXP_API sexp sexp_make_input_port (sexp ctx, FILE* in, sexp name);
SEXP_API sexp sexp_make_output_port (sexp ctx, FILE* out, sexp name);
SEXP_API sexp sexp_open_input_file_descriptor (sexp ctx, sexp self, sexp_sint_t n, sexp fileno, sexp socketp);
SEXP_API sexp sexp_open_output_file_descriptor (sexp ctx, sexp self, sexp_sint_t n, sexp fileno, sexp socketp);
SEXP_API sexp sexp_make_non_null_input_port (sexp ctx, FILE* in, sexp name);
SEXP_API sexp sexp_make_non_null_output_port (sexp ctx, FILE* out, sexp name);
SEXP_API sexp sexp_make_non_null_input_output_port (sexp ctx, FILE* io, sexp name);
SEXP_API sexp sexp_port_outputp_op (sexp ctx, sexp self, sexp_sint_t n, sexp port);
SEXP_API sexp sexp_port_binaryp_op (sexp ctx, sexp self, sexp_sint_t n, sexp port);
SEXP_API sexp sexp_port_openp_op (sexp ctx, sexp self, sexp_sint_t n, sexp port);
#if SEXP_USE_FOLD_CASE_SYMS
SEXP_API sexp sexp_get_port_fold_case (sexp ctx, sexp self, sexp_sint_t n, sexp in);
SEXP_API sexp sexp_set_port_fold_case (sexp ctx, sexp self, sexp_sint_t n, sexp in, sexp x);
#endif
#if SEXP_USE_OBJECT_BRACE_LITERALS
SEXP_API sexp sexp_lookup_type_op (sexp ctx, sexp self, sexp_sint_t n, sexp name, sexp id);
#endif
SEXP_API sexp sexp_open_input_string_op (sexp ctx, sexp self, sexp_sint_t n, sexp str);
SEXP_API sexp sexp_open_output_string_op (sexp ctx, sexp self, sexp_sint_t n);
SEXP_API sexp sexp_get_output_string_op (sexp ctx, sexp self, sexp_sint_t n, sexp port);
SEXP_API sexp sexp_make_immutable_op (sexp ctx, sexp self, sexp_sint_t n, sexp x);
SEXP_API sexp sexp_make_exception (sexp ctx, sexp kind, sexp message, sexp irritants, sexp procedure, sexp source);
SEXP_API sexp sexp_user_exception (sexp ctx, sexp self, const char *msg, sexp x);
SEXP_API sexp sexp_user_exception_ls (sexp ctx, sexp self, const char *msg, int n, ...);
SEXP_API sexp sexp_file_exception (sexp ctx, sexp self, const char *msg, sexp x);
SEXP_API sexp sexp_type_exception (sexp ctx, sexp self, sexp_uint_t type_id, sexp x);
SEXP_API sexp sexp_xtype_exception (sexp ctx, sexp self, const char *msg, sexp x);
SEXP_API sexp sexp_range_exception (sexp ctx, sexp obj, sexp start, sexp end);
SEXP_API sexp sexp_get_stack_trace (sexp ctx);
SEXP_API sexp sexp_print_exception_op (sexp ctx, sexp self, sexp_sint_t n, sexp exn, sexp out);
SEXP_API sexp sexp_stack_trace_op (sexp ctx, sexp self, sexp_sint_t n, sexp out);
SEXP_API sexp sexp_print_exception_stack_trace_op (sexp ctx, sexp self, sexp_sint_t n, sexp exn, sexp out);
SEXP_API sexp sexp_apply (sexp context, sexp proc, sexp args);
SEXP_API sexp sexp_apply1 (sexp ctx, sexp f, sexp x);
SEXP_API sexp sexp_apply2 (sexp ctx, sexp f, sexp x, sexp y);
SEXP_API sexp sexp_apply3 (sexp ctx, sexp f, sexp x, sexp y, sexp z);
SEXP_API sexp sexp_apply_no_err_handler (sexp ctx, sexp proc, sexp args);
SEXP_API sexp sexp_make_trampoline (sexp ctx, sexp proc, sexp args);
SEXP_API sexp sexp_make_foreign (sexp ctx, const char *name, int num_args, int flags, const char *fname, sexp_proc1 f, sexp data);
SEXP_API void sexp_init(void);

#if SEXP_USE_UTF8_STRINGS
SEXP_API int sexp_utf8_initial_byte_count (int c);
SEXP_API int sexp_utf8_char_byte_count (int c);
SEXP_API sexp_uint_t sexp_string_utf8_length (unsigned char *p, long len);
SEXP_API char* sexp_string_utf8_prev (unsigned char *p);
SEXP_API sexp sexp_string_utf8_ref (sexp ctx, sexp str, sexp i);
SEXP_API sexp sexp_string_utf8_index_ref (sexp ctx, sexp self, sexp_sint_t n, sexp str, sexp i);
SEXP_API sexp sexp_string_index_to_cursor (sexp ctx, sexp self, sexp_sint_t n, sexp str, sexp index);
SEXP_API sexp sexp_string_cursor_to_index (sexp ctx, sexp self, sexp_sint_t n, sexp str, sexp offset);
SEXP_API sexp sexp_string_cursor_offset (sexp ctx, sexp self, sexp_sint_t n, sexp cur);
SEXP_API sexp sexp_utf8_substring_op (sexp ctx, sexp self, sexp_sint_t n, sexp str, sexp start, sexp end);
SEXP_API void sexp_utf8_encode_char (unsigned char* p, int len, int c);
SEXP_API int sexp_write_utf8_char (sexp ctx, int c, sexp out);
#define sexp_string_ref(ctx, s, i)    (sexp_string_utf8_index_ref(ctx, NULL, 2, s, i))
#define sexp_string_set(ctx, s, i, ch) (sexp_string_utf8_index_set(ctx, NULL, 3, s, i, ch))
#define sexp_string_cursor_ref(ctx, s, i)    (sexp_string_utf8_ref(ctx, s, i))
#define sexp_string_cursor_set(ctx, s, i, ch)    (sexp_string_utf8_set(ctx, s, i, ch))
#define sexp_string_cursor_next(s, i) sexp_make_string_cursor(sexp_unbox_string_cursor(i) + sexp_utf8_initial_byte_count(((unsigned char*)sexp_string_data(s))[sexp_unbox_string_cursor(i)]))
#define sexp_string_cursor_prev(s, i) sexp_make_string_cursor(sexp_string_utf8_prev((unsigned char*)sexp_string_data(s)+sexp_unbox_string_cursor(i)) - sexp_string_data(s))
#define sexp_string_length(s) sexp_string_utf8_length((unsigned char*)sexp_string_data(s), sexp_string_size(s))
#define sexp_substring(ctx, s, i, j) sexp_utf8_substring_op(ctx, NULL, 3, s, i, j)
#define sexp_substring_cursor(ctx, s, i, j) sexp_substring_op(ctx, NULL, 3, s, i, j)
#else  /* ASCII strings */
#define sexp_string_ref(ctx, s, i)    (sexp_make_character((unsigned char)sexp_string_data(s)[sexp_unbox_fixnum(i)]))
#define sexp_string_set(ctx, s, i, ch) (sexp_string_data(s)[sexp_unbox_fixnum(i)] = sexp_unbox_character(ch))
#define sexp_string_cursor_ref(ctx, s, i) sexp_string_ref(ctx, s, i)
#define sexp_string_cursor_set(ctx, s, i, ch) sexp_string_set(ctx, s, i, ch)
#define sexp_string_cursor_next(s, i) sexp_make_fixnum(sexp_unbox_fixnum(i) + 1)
#define sexp_string_cursor_prev(s, i) sexp_make_fixnum(sexp_unbox_fixnum(i) - 1)
#define sexp_string_length(s) sexp_string_size(s)
#define sexp_substring(ctx, s, i, j) sexp_substring_op(ctx, NULL, 3, s, i, j)
#define sexp_substring_cursor(ctx, s, i, j) sexp_substring_op(ctx, NULL, 3, s, i, j)
#endif

#if SEXP_USE_STRING_INDEX_TABLE
SEXP_API void sexp_update_string_index_lookup(sexp ctx, sexp s);
#else
#define sexp_update_string_index_lookup(ctx, s)
#endif

#if SEXP_USE_GREEN_THREADS
SEXP_API int sexp_maybe_block_port (sexp ctx, sexp in, int forcep);
SEXP_API void sexp_maybe_unblock_port (sexp ctx, sexp in);
#define sexp_check_block_port(ctx, in, forcep)          \
  if (sexp_maybe_block_port(ctx, in, forcep))           \
    return sexp_global(ctx, SEXP_G_IO_BLOCK_ERROR)
#else
#define sexp_maybe_block_port(ctx, in, forcep)
#define sexp_maybe_unblock_port(ctx, in)
#define sexp_check_block_port(ctx, in, forcep)
#endif

#define SEXP_PORT_UNKNOWN_FLAGS -1uL

#define sexp_assert_type(ctx, pred, type_id, obj) if (! pred(obj)) return sexp_type_exception(ctx, self, type_id, obj)

#define SEXP_COPY_DEFAULT SEXP_ZERO
#define SEXP_COPY_FREEP   SEXP_ONE
#define SEXP_COPY_LOADP   SEXP_TWO

#if ! SEXP_USE_BOEHM && ! SEXP_USE_MALLOC
SEXP_API void sexp_gc_init (void);
SEXP_API int sexp_grow_heap (sexp ctx, size_t size, size_t chunk_size);
SEXP_API sexp_heap sexp_make_heap (size_t size, size_t max_size, size_t chunk_size);
SEXP_API void sexp_mark (sexp ctx, sexp x);
SEXP_API sexp sexp_sweep (sexp ctx, size_t *sum_freed_ptr);
#if SEXP_USE_FINALIZERS
SEXP_API sexp sexp_finalize (sexp ctx);
#else
#define sexp_finalize(ctx) SEXP_ZERO
#endif
#endif

#if SEXP_USE_GLOBAL_HEAP
#define sexp_free_heap(heap)
#define sexp_debug_heap_stats(heap)
#define sexp_destroy_context(ctx) SEXP_TRUE
#else
SEXP_API void sexp_free_heap (sexp_heap heap);
SEXP_API void sexp_debug_heap_stats (sexp_heap heap);
SEXP_API void sexp_debug_alloc_times(sexp ctx);
SEXP_API void sexp_debug_alloc_sizes(sexp ctx);
SEXP_API sexp sexp_destroy_context (sexp ctx);
SEXP_API sexp sexp_copy_context (sexp ctx, sexp dst, sexp flags);
#endif

#if SEXP_USE_SAFE_GC_MARK
SEXP_API int sexp_in_heap_p(sexp ctx, sexp x);
#else
#define sexp_in_heap_p(ctx, x) 1
#endif

#if SEXP_DEBUG_GC > 1 || SEXP_USE_SAFE_GC_MARK || SEXP_USE_HEADER_MAGIC
SEXP_API int sexp_valid_object_p(sexp ctx, sexp x);
#else
#define sexp_valid_object_p(ctx, x) 1
#endif

#if SEXP_USE_TYPE_DEFS
SEXP_API sexp sexp_register_type_op (sexp, sexp, sexp_sint_t, sexp, sexp, sexp, sexp, sexp, sexp, sexp, sexp, sexp, sexp, sexp, sexp, sexp, sexp, sexp, sexp, sexp, const char*, sexp_proc2);
SEXP_API sexp sexp_register_simple_type_op (sexp ctx, sexp self, sexp_sint_t n, sexp name, sexp parent, sexp slots);
SEXP_API sexp sexp_finalize_c_type (sexp ctx, sexp self, sexp_sint_t n, sexp obj);
#define sexp_register_c_type(ctx, name, finalizer)                      \
  sexp_register_type(ctx, name, SEXP_FALSE, SEXP_FALSE, SEXP_ZERO, SEXP_ZERO, \
                     SEXP_ZERO, SEXP_ZERO, SEXP_ZERO,                   \
                     sexp_make_fixnum(sexp_sizeof(cpointer)),           \
                     SEXP_ZERO, SEXP_ZERO, SEXP_ZERO, SEXP_ZERO,        \
                     SEXP_ZERO, SEXP_ZERO, SEXP_ZERO, NULL,             \
                     #finalizer, (sexp_proc2)finalizer)
#endif

#define sexp_current_input_port(ctx) sexp_parameter_ref(ctx, sexp_env_ref(ctx, sexp_context_env(ctx), sexp_global(ctx,SEXP_G_CUR_IN_SYMBOL), SEXP_FALSE))
#define sexp_current_output_port(ctx) sexp_parameter_ref(ctx, sexp_env_ref(ctx, sexp_context_env(ctx), sexp_global(ctx,SEXP_G_CUR_OUT_SYMBOL), SEXP_FALSE))
#define sexp_current_error_port(ctx) sexp_parameter_ref(ctx, sexp_env_ref(ctx, sexp_context_env(ctx), sexp_global(ctx,SEXP_G_CUR_ERR_SYMBOL), SEXP_FALSE))
#define sexp_debug(ctx, msg, obj) (sexp_portp(sexp_current_error_port(ctx)) ? (sexp_write_string(ctx, msg, sexp_current_error_port(ctx)), sexp_write(ctx, obj, sexp_current_error_port(ctx)), sexp_write_char(ctx, '\n', sexp_current_error_port(ctx))) : 0)

#if SEXP_USE_POLL_PORT
SEXP_API int sexp_poll_port(sexp ctx, sexp port, int inputp);
#endif

/* simplify primitive API interface */

#define sexp_read(ctx, in) sexp_read_op(ctx, NULL, 1, in)
#define sexp_write(ctx, obj, out) sexp_write_op(ctx, NULL, 2, obj, out)
#define sexp_print_exception(ctx, e, out) sexp_print_exception_op(ctx, NULL, 2, e, out)
#define sexp_print_exception_stack_trace(ctx, e, out) sexp_print_exception_stack_trace_op(ctx, NULL, 2, e, out)
#define sexp_flush_output(ctx, out) sexp_flush_output_op(ctx, NULL, 1, out)
#define sexp_equalp(ctx, a, b) sexp_equalp_op(ctx, NULL, 2, a, b)
#define sexp_listp(ctx, x) sexp_listp_op(ctx, NULL, 1, x)
#define sexp_length(ctx, x) sexp_length_op(ctx, NULL, 1, x)
#define sexp_length_unboxed(x) sexp_unbox_fixnum(sexp_length(NULL, x))
#define sexp_reverse(ctx, x) sexp_reverse_op(ctx, NULL, 1, x)
#define sexp_nreverse(ctx, x) sexp_nreverse_op(ctx, NULL, 1, x)
#define sexp_copy_list(ctx, x) sexp_copy_list_op(ctx, NULL, 1, x)
#define sexp_cons(ctx, a, b) sexp_cons_op(ctx, NULL, 2, a, b)
#define sexp_append2(ctx, a, b) sexp_append2_op(ctx, NULL, 2, a, b)
#define sexp_make_vector(ctx, a, b) sexp_make_vector_op(ctx, NULL, 2, a, b)
#define sexp_list_to_vector(ctx, x) sexp_list_to_vector_op(ctx, NULL, 1, x)
#define sexp_list_to_uvector(ctx, etype, ls) sexp_list_to_uvector_op(ctx, NULL, 2, etype, ls)
#define sexp_exception_type(ctx, x) sexp_exception_type_op(ctx, NULL, 1, x)
#define sexp_string_to_symbol(ctx, s) sexp_string_to_symbol_op(ctx, NULL, 1, s)
#define sexp_string_to_number(ctx, s, b) sexp_string_to_number_op(ctx, NULL, 2, s, b)
#define sexp_symbol_to_string(ctx, s) sexp_symbol_to_string_op(ctx, NULL, 1, s)
#define sexp_make_ephemeron(ctx, k, v) sexp_make_ephemeron_op(ctx, NULL, 2, k, v)
#define sexp_make_bytes(ctx, l, i) sexp_make_bytes_op(ctx, NULL, 2, l, i)
#if SEXP_USE_UNIFORM_VECTOR_LITERALS
#define sexp_make_uvector(ctx, et, l) sexp_make_uvector_op(ctx, NULL, 2, et, l)
#else
#define sexp_make_uvector(ctx, et, l) sexp_make_vector(ctx, l, SEXP_ZERO)
#define sexp_write_uvector NULL
#define sexp_finalize_uvector NULL
#endif
#define sexp_make_string(ctx, l, c) sexp_make_string_op(ctx, NULL, 2, l, c)
#define sexp_subbytes(ctx, a, b, c) sexp_subbytes_op(ctx, NULL, 3, a, b, c)
#define sexp_string_concatenate(ctx, ls, s) sexp_string_concatenate_op(ctx, NULL, 2, ls, s)
#define sexp_memq(ctx, a, b) sexp_memq_op(ctx, NULL, 2, a, b)
#define sexp_assq(ctx, a, b) sexp_assq_op(ctx, NULL, 2, a, b)
#define sexp_open_output_string(ctx) sexp_open_output_string_op(ctx, NULL, 0)
#define sexp_open_input_string(ctx, s) sexp_open_input_string_op(ctx, NULL, 1, s)
#define sexp_get_output_string(ctx, out) sexp_get_output_string_op(ctx, NULL, 1, out)
#define sexp_expt(ctx, a, b) sexp_expt_op(ctx, NULL, 2, a, b)
#define sexp_register_simple_type(ctx, a, b, c) sexp_register_simple_type_op(ctx, NULL, 3, a, b, c)
#define sexp_register_type(ctx, a, b, c, d, e, f, g, h, i, j, k, l, m, o, p, q, r, sn, s) sexp_register_type_op(ctx, NULL, 18, a, b, c, d, e, f, g, h, i, j, k, l, m, o, p, q, r, sn, s)
#define sexp_make_type_predicate(ctx, a, b) sexp_make_type_predicate_op(ctx, NULL, 2, a, b)
#define sexp_make_constructor(ctx, a, b) sexp_make_constructor_op(ctx, NULL, 2, a, b)
#define sexp_make_getter(ctx, a, b, c) sexp_make_getter_op(ctx, NULL, 3, a, b, c)
#define sexp_make_setter(ctx, a, b, c) sexp_make_setter_op(ctx, NULL, 3, a, b, c)
#define sexp_lookup_type(ctx, name, id) sexp_lookup_type_op(ctx, NULL, 2, name, id)
#define sexp_make_fileno(ctx, fd, no_closep) sexp_make_fileno_op(ctx, NULL, 2, fd, no_closep)

enum sexp_opcode_names {
  /*  0 00 */ SEXP_OP_NOOP,
  /*  1 01 */ SEXP_OP_RAISE,
  /*  2 02 */ SEXP_OP_RESUMECC,
  /*  3 03 */ SEXP_OP_CALLCC,
  /*  4 04 */ SEXP_OP_APPLY1,
  /*  5 05 */ SEXP_OP_TAIL_CALL,
  /*  6 06 */ SEXP_OP_CALL,
  /*  7 07 */ SEXP_OP_FCALL0,
  /*  8 08 */ SEXP_OP_FCALL1,
  /*  9 09 */ SEXP_OP_FCALL2,
  /* 10 0A */ SEXP_OP_FCALL3,
  /* 11 0B */ SEXP_OP_FCALL4,
  /* 12 0C */ SEXP_OP_FCALLN,
  /* 13 0D */ SEXP_OP_JUMP_UNLESS,
  /* 14 0E */ SEXP_OP_JUMP,
  /* 15 0F */ SEXP_OP_PUSH,
  /* 16 10 */ SEXP_OP_RESERVE,
  /* 17 11 */ SEXP_OP_DROP,
  /* 18 12 */ SEXP_OP_GLOBAL_REF,
  /* 19 13 */ SEXP_OP_GLOBAL_KNOWN_REF,
  /* 20 14 */ SEXP_OP_PARAMETER_REF,
  /* 21 15 */ SEXP_OP_STACK_REF,
  /* 22 16 */ SEXP_OP_LOCAL_REF,
  /* 23 17 */ SEXP_OP_LOCAL_SET,
  /* 24 18 */ SEXP_OP_CLOSURE_REF,
  /* 25 19 */ SEXP_OP_CLOSURE_VARS,
  /* 26 1A */ SEXP_OP_VECTOR_REF,
  /* 27 1B */ SEXP_OP_VECTOR_SET,
  /* 28 1C */ SEXP_OP_VECTOR_LENGTH,
  /* 29 1D */ SEXP_OP_BYTES_REF,
  /* 30 1E */ SEXP_OP_BYTES_SET,
  /* 31 1F */ SEXP_OP_BYTES_LENGTH,
  /* 32 20 */ SEXP_OP_STRING_REF,
  /* 33 21 */ SEXP_OP_STRING_SET,
  /* 34 22 */ SEXP_OP_STRING_LENGTH,
  /* 35 23 */ SEXP_OP_STRING_CURSOR_NEXT,
  /* 36 24 */ SEXP_OP_STRING_CURSOR_PREV,
  /* 37 25 */ SEXP_OP_STRING_CURSOR_END,
  /* 38 26 */ SEXP_OP_MAKE_PROCEDURE,
  /* 39 27 */ SEXP_OP_MAKE_VECTOR,
  /* 40 28 */ SEXP_OP_MAKE_EXCEPTION,
  /* 41 29 */ SEXP_OP_AND,
  /* 42 2A */ SEXP_OP_NULLP,
  /* 43 2B */ SEXP_OP_FIXNUMP,
  /* 44 2C */ SEXP_OP_SYMBOLP,
  /* 45 2D */ SEXP_OP_CHARP,
  /* 46 2E */ SEXP_OP_EOFP,
  /* 47 2F */ SEXP_OP_TYPEP,
  /* 48 30 */ SEXP_OP_MAKE,
  /* 49 31 */ SEXP_OP_SLOT_REF,
  /* 50 32 */ SEXP_OP_SLOT_SET,
  /* 51 33 */ SEXP_OP_ISA,
  /* 52 34 */ SEXP_OP_SLOTN_REF,
  /* 53 35 */ SEXP_OP_SLOTN_SET,
  /* 54 36 */ SEXP_OP_CAR,
  /* 55 37 */ SEXP_OP_CDR,
  /* 56 38 */ SEXP_OP_SET_CAR,
  /* 57 39 */ SEXP_OP_SET_CDR,
  /* 58 3A */ SEXP_OP_CONS,
  /* 59 3B */ SEXP_OP_ADD,
  /* 60 3C */ SEXP_OP_SUB,
  /* 61 3D */ SEXP_OP_MUL,
  /* 62 3E */ SEXP_OP_DIV,
  /* 63 3F */ SEXP_OP_QUOTIENT,
  /* 64 40 */ SEXP_OP_REMAINDER,
  /* 65 41 */ SEXP_OP_LT,
  /* 66 42 */ SEXP_OP_LE,
  /* 67 43 */ SEXP_OP_EQN,
  /* 68 44 */ SEXP_OP_EQ,
  /* 69 45 */ SEXP_OP_CHAR2INT,
  /* 70 46 */ SEXP_OP_INT2CHAR,
  /* 71 47 */ SEXP_OP_CHAR_UPCASE,
  /* 72 48 */ SEXP_OP_CHAR_DOWNCASE,
  /* 73 49 */ SEXP_OP_WRITE_CHAR,
  /* 74 4A */ SEXP_OP_WRITE_STRING,
  /* 75 4B */ SEXP_OP_READ_CHAR,
  /* 76 4C */ SEXP_OP_PEEK_CHAR,
  /* 77 4D */ SEXP_OP_YIELD,
  /* 78 4E */ SEXP_OP_FORCE,
  /* 79 4F */ SEXP_OP_RET,
  /* 80 50 */ SEXP_OP_DONE,
  SEXP_OP_SCP,
  SEXP_OP_SC_LT,
  SEXP_OP_SC_LE,
  SEXP_OP_NUM_OPCODES
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ! SEXP_H */
