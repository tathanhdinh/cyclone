#include <setjmp.h>
/* This is a C header file to be used by the output of the Cyclone to
   C translator.  The corresponding definitions are in file
   lib/runtime_cyc.c
*/
#ifndef _CYC_INCLUDE_H_
#define _CYC_INCLUDE_H_

#ifdef NO_CYC_PREFIX
#define ADD_PREFIX(x) x
#else
#define ADD_PREFIX(x) Cyc_##x
#endif

#ifndef offsetof
/* should be size_t, but int is fine. */
#define offsetof(t,n) ((int)(&(((t *)0)->n)))
#endif

/* Tagged arrays */
struct _dyneither_ptr {
  unsigned char *curr; 
  unsigned char *base; 
  unsigned char *last_plus_one; 
};  

/* Discriminated Unions */
struct _xtunion_struct { char *tag; };

/* Need one of these per thread (we don't have threads)
   The runtime maintains a stack that contains either _handler_cons
   structs or _RegionHandle structs.  The tag is 0 for a handler_cons
   and 1 for a region handle.  */
struct _RuntimeStack {
  int tag; /* 0 for an exception handler, 1 for a region handle */
  struct _RuntimeStack *next;
};

/* Regions */
struct _RegionPage {
#ifdef CYC_REGION_PROFILE
  unsigned total_bytes;
  unsigned free_bytes;
#endif
  struct _RegionPage *next;
  char data[1];  /*FJS: used to be size 0, but that's forbidden in ansi c*/
};

struct _RegionHandle {
  struct _RuntimeStack s;
  struct _RegionPage *curr;
  char               *offset;
  char               *last_plus_one;
  struct _DynRegionHandle *sub_regions;
#ifdef CYC_REGION_PROFILE
  const char         *name;
#endif
};

struct _DynRegionFrame {
  struct _RuntimeStack s;
  struct _DynRegionHandle *x;
};

extern struct _RegionHandle _new_region(const char *);
extern void * _region_malloc(struct _RegionHandle *, unsigned);
extern void * _region_calloc(struct _RegionHandle *, unsigned t, unsigned n);
extern void   _free_region(struct _RegionHandle *);
extern void   _reset_region(struct _RegionHandle *);
extern struct _RegionHandle *_open_dynregion(struct _DynRegionFrame *f,
                                             struct _DynRegionHandle *h);
extern void   _pop_dynregion();

/* Exceptions */
struct _handler_cons {
  struct _RuntimeStack s;
  jmp_buf handler;
};
extern void _push_handler(struct _handler_cons *);
extern void _push_region(struct _RegionHandle *);
extern void _npop_handler(int);
extern void _pop_handler();
extern void _pop_region();

#ifndef _throw
extern int _throw_null_fn(const char *filename, unsigned lineno);
extern int _throw_arraybounds_fn(const char *filename, unsigned lineno);
extern int _throw_badalloc_fn(const char *filename, unsigned lineno);
extern int _throw_match_fn(const char *filename, unsigned lineno);
extern int _throw_fn(void* e, const char *filename, unsigned lineno);
#define _throw_null() (_throw_null_fn(__FILE__,__LINE__))
#define _throw_arraybounds() (_throw_arraybounds_fn(__FILE__,__LINE__))
#define _throw_badalloc() (_throw_badalloc_fn(__FILE__,__LINE__))
#define _throw_match() (_throw_match_fn(__FILE__,__LINE__))
#define _throw(e) (_throw_fn((e),__FILE__,__LINE__))
#endif

extern struct _xtunion_struct *_exn_thrown;

/* Built-in Exceptions */
struct Cyc_Null_Exception_exn_struct { char *tag; };
struct Cyc_Array_bounds_exn_struct { char *tag; };
struct Cyc_Match_Exception_exn_struct { char *tag; };
struct Cyc_Bad_alloc_exn_struct { char *tag; };
extern char Cyc_Null_Exception[];
extern char Cyc_Array_bounds[];
extern char Cyc_Match_Exception[];
extern char Cyc_Bad_alloc[];

/* Built-in Run-time Checks and company */
#ifdef __APPLE__
#define _INLINE_FUNCTIONS
#endif

#ifdef CYC_ANSI_OUTPUT
#define _INLINE  
#define _INLINE_FUNCTIONS
#else
#define _INLINE inline
#endif

#ifdef VC_C
#define _CYC_U_LONG_LONG_T __int64
#else
#ifdef GCC_C
#define _CYC_U_LONG_LONG_T unsigned long long
#else
#define _CYC_U_LONG_LONG_T unsigned long long
#endif
#endif

#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE void *
_check_null_fn(const void *ptr, const char *filename, unsigned lineno) {
  void*_check_null_temp = (void*)(ptr);
  if (!_check_null_temp) _throw_null_fn(filename,lineno);
  return _check_null_temp;
}
#define _check_null(p) (_check_null_fn((p),__FILE__,__LINE__))
#else
#define _check_null(ptr) \
  ({ void*_check_null_temp = (void*)(ptr); \
     if (!_check_null_temp) _throw_null(); \
     _check_null_temp; })
#endif
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  ((char *)ptr) + (elt_sz)*(index); })
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE char *
_check_known_subscript_null_fn(void *ptr, unsigned bound, unsigned elt_sz, unsigned index, const char *filename, unsigned lineno) {
  void*_cks_ptr = (void*)(ptr);
  unsigned _cks_bound = (bound);
  unsigned _cks_elt_sz = (elt_sz);
  unsigned _cks_index = (index);
  if (!_cks_ptr) _throw_null_fn(filename,lineno);
  if (_cks_index >= _cks_bound) _throw_arraybounds_fn(filename,lineno);
  return ((char *)_cks_ptr) + _cks_elt_sz*_cks_index;
}
#define _check_known_subscript_null(p,b,e) (_check_known_subscript_null_fn(p,b,e,__FILE__,__LINE__))
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  void*_cks_ptr = (void*)(ptr); \
  unsigned _cks_bound = (bound); \
  unsigned _cks_elt_sz = (elt_sz); \
  unsigned _cks_index = (index); \
  if (!_cks_ptr) _throw_null(); \
  if (_cks_index >= _cks_bound) _throw_arraybounds(); \
  ((char *)_cks_ptr) + _cks_elt_sz*_cks_index; })
#endif
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_notnull(bound,index) (index)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned
_check_known_subscript_notnull_fn(unsigned bound,unsigned index,const char *filename,unsigned lineno) { 
  unsigned _cksnn_bound = (bound); 
  unsigned _cksnn_index = (index); 
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds_fn(filename,lineno); 
  return _cksnn_index;
}
#define _check_known_subscript_notnull(b,i) (_check_known_subscript_notnull_fn(b,i,__FILE__,__LINE__))
#else
#define _check_known_subscript_notnull(bound,index) ({ \
  unsigned _cksnn_bound = (bound); \
  unsigned _cksnn_index = (index); \
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds(); \
  _cksnn_index; })
#endif
#endif

/* Add i to zero-terminated pointer x.  Checks for x being null and
   ensures that x[0..i-1] are not 0. */
#ifdef NO_CYC_BOUNDS_CHECK
#define _zero_arr_plus_char_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_short_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_int_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_float_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_double_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_longdouble_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_voidstar_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#else
static _INLINE char *
_zero_arr_plus_char_fn(char *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE short *
_zero_arr_plus_short_fn(short *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE int *
_zero_arr_plus_int_fn(int *orig_x, unsigned int orig_sz, int orig_i, const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE float *
_zero_arr_plus_float_fn(float *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE double *
_zero_arr_plus_double_fn(double *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE long double *
_zero_arr_plus_longdouble_fn(long double *orig_x, unsigned int orig_sz, int orig_i, const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE void *
_zero_arr_plus_voidstar_fn(void **orig_x, unsigned int orig_sz, int orig_i,const char *filename,unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0 || orig_sz == 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz-1; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
#endif

#define _zero_arr_plus_char(x,s,i) \
  (_zero_arr_plus_char_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_short(x,s,i) \
  (_zero_arr_plus_short_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_int(x,s,i) \
  (_zero_arr_plus_int_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_float(x,s,i) \
  (_zero_arr_plus_float_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_double(x,s,i) \
  (_zero_arr_plus_double_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_longdouble(x,s,i) \
  (_zero_arr_plus_longdouble_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_voidstar(x,s,i) \
  (_zero_arr_plus_voidstar_fn(x,s,i,__FILE__,__LINE__))


/* Calculates the number of elements in a zero-terminated, thin array.
   If non-null, the array is guaranteed to have orig_offset elements. */
static _INLINE int
_get_zero_arr_size_char(const char *orig_x, unsigned int orig_offset) {
  const char *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_short(const short *orig_x, unsigned int orig_offset) {
  const short *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_int(const int *orig_x, unsigned int orig_offset) {
  const int *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_float(const float *orig_x, unsigned int orig_offset) {
  const float *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_double(const double *orig_x, unsigned int orig_offset) {
  const double *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_longdouble(const long double *orig_x, unsigned int orig_offset) {
  const long double *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_voidstar(const void **orig_x, unsigned int orig_offset) {
  const void **_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}


/* Does in-place addition of a zero-terminated pointer (x += e and ++x).  
   Note that this expands to call _zero_arr_plus_<type>_fn. */
static _INLINE char *
_zero_arr_inplace_plus_char_fn(char **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_char_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_char(x,i) \
  _zero_arr_inplace_plus_char_fn((char **)(x),i,__FILE__,__LINE__)
static _INLINE short *
_zero_arr_inplace_plus_short_fn(short **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_short_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_short(x,i) \
  _zero_arr_inplace_plus_short_fn((short **)(x),i,__FILE__,__LINE__)
static _INLINE int *
_zero_arr_inplace_plus_int(int **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_int_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_int(x,i) \
  _zero_arr_inplace_plus_int_fn((int **)(x),i,__FILE__,__LINE__)
static _INLINE float *
_zero_arr_inplace_plus_float_fn(float **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_float_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_float(x,i) \
  _zero_arr_inplace_plus_float_fn((float **)(x),i,__FILE__,__LINE__)
static _INLINE double *
_zero_arr_inplace_plus_double_fn(double **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_double_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_double(x,i) \
  _zero_arr_inplace_plus_double_fn((double **)(x),i,__FILE__,__LINE__)
static _INLINE long double *
_zero_arr_inplace_plus_longdouble_fn(long double **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_longdouble_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_longdouble(x,i) \
  _zero_arr_inplace_plus_longdouble_fn((long double **)(x),i,__FILE__,__LINE__)
static _INLINE void *
_zero_arr_inplace_plus_voidstar_fn(void ***x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_voidstar_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_voidstar(x,i) \
  _zero_arr_inplace_plus_voidstar_fn((void ***)(x),i,__FILE__,__LINE__)

/* Does in-place increment of a zero-terminated pointer (e.g., x++). */
static _INLINE char *
_zero_arr_inplace_plus_post_char_fn(char **x, int orig_i,const char *filename,unsigned lineno){
  char * _zap_res = *x;
  *x = _zero_arr_plus_char_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_char(x,i) \
  _zero_arr_inplace_plus_post_char_fn((char **)(x),(i),__FILE__,__LINE__)
static _INLINE short *
_zero_arr_inplace_plus_post_short_fn(short **x, int orig_i,const char *filename,unsigned lineno){
  short * _zap_res = *x;
  *x = _zero_arr_plus_short_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_short(x,i) \
  _zero_arr_inplace_plus_post_short_fn((short **)(x),(i),__FILE__,__LINE__)
static _INLINE int *
_zero_arr_inplace_plus_post_int_fn(int **x, int orig_i,const char *filename, unsigned lineno){
  int * _zap_res = *x;
  *x = _zero_arr_plus_int_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_int(x,i) \
  _zero_arr_inplace_plus_post_int_fn((int **)(x),(i),__FILE__,__LINE__)
static _INLINE float *
_zero_arr_inplace_plus_post_float_fn(float **x, int orig_i,const char *filename, unsigned lineno){
  float * _zap_res = *x;
  *x = _zero_arr_plus_float_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_float(x,i) \
  _zero_arr_inplace_plus_post_float_fn((float **)(x),(i),__FILE__,__LINE__)
static _INLINE double *
_zero_arr_inplace_plus_post_double_fn(double **x, int orig_i,const char *filename,unsigned lineno){
  double * _zap_res = *x;
  *x = _zero_arr_plus_double_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_double(x,i) \
  _zero_arr_inplace_plus_post_double_fn((double **)(x),(i),__FILE__,__LINE__)
static _INLINE long double *
_zero_arr_inplace_plus_post_longdouble_fn(long double **x, int orig_i,const char *filename,unsigned lineno){
  long double * _zap_res = *x;
  *x = _zero_arr_plus_longdouble_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_longdouble(x,i) \
  _zero_arr_inplace_plus_post_longdouble_fn((long double **)(x),(i),__FILE__,__LINE__)
static _INLINE void **
_zero_arr_inplace_plus_post_voidstar_fn(void ***x, int orig_i,const char *filename,unsigned lineno){
  void ** _zap_res = *x;
  *x = _zero_arr_plus_voidstar_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_voidstar(x,i) \
  _zero_arr_inplace_plus_post_voidstar_fn((void***)(x),(i),__FILE__,__LINE__)

/* functions for dealing with dynamically sized pointers */
#ifdef NO_CYC_BOUNDS_CHECKS
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_check_dyneither_subscript(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  return _cus_ans;
}
#else
#define _check_dyneither_subscript(arr,elt_sz,index) ({ \
  struct _dyneither_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  _cus_ans; })
#endif
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_check_dyneither_subscript_fn(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index,const char *filename, unsigned lineno) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  /* JGM: not needed! if (!_cus_arr.base) _throw_null(); */ 
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds_fn(filename,lineno);
  return _cus_ans;
}
#define _check_dyneither_subscript(a,s,i) \
  _check_dyneither_subscript_fn(a,s,i,__FILE__,__LINE__)
#else
#define _check_dyneither_subscript(arr,elt_sz,index) ({ \
  struct _dyneither_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  /* JGM: not needed! if (!_cus_arr.base) _throw_null();*/ \
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one) \
    _throw_arraybounds(); \
  _cus_ans; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_tag_dyneither(const void *tcurr,unsigned elt_sz,unsigned num_elts) {
  struct _dyneither_ptr _tag_arr_ans;
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr);
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts);
  return _tag_arr_ans;
}
#else
#define _tag_dyneither(tcurr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _tag_arr_ans; \
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr); \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts); \
  _tag_arr_ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr *
_init_dyneither_ptr(struct _dyneither_ptr *arr_ptr,
                    void *arr, unsigned elt_sz, unsigned num_elts) {
  struct _dyneither_ptr *_itarr_ptr = (arr_ptr);
  void* _itarr = (arr);
  _itarr_ptr->base = _itarr_ptr->curr = _itarr;
  _itarr_ptr->last_plus_one = ((unsigned char *)_itarr) + (elt_sz) * (num_elts);
  return _itarr_ptr;
}
#else
#define _init_dyneither_ptr(arr_ptr,arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr *_itarr_ptr = (arr_ptr); \
  void* _itarr = (arr); \
  _itarr_ptr->base = _itarr_ptr->curr = _itarr; \
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts); \
  _itarr_ptr; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ((arr).curr)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_untag_dyneither_ptr_fn(struct _dyneither_ptr arr, 
                        unsigned elt_sz,unsigned num_elts,
                        const char *filename, unsigned lineno) {
  struct _dyneither_ptr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)
    _throw_arraybounds_fn(filename,lineno);
  return _curr;
}
#define _untag_dyneither_ptr(a,s,e) \
  _untag_dyneither_ptr_fn(a,s,e,__FILE__,__LINE__)
#else
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)\
    _throw_arraybounds(); \
  _curr; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned
_get_dyneither_size(struct _dyneither_ptr arr,unsigned elt_sz) {
  struct _dyneither_ptr _get_arr_size_temp = (arr);
  unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr;
  unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one;
  return (_get_arr_size_curr < _get_arr_size_temp.base ||
          _get_arr_size_curr >= _get_arr_size_last) ? 0 :
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));
}
#else
#define _get_dyneither_size(arr,elt_sz) \
  ({struct _dyneither_ptr _get_arr_size_temp = (arr); \
    unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr; \
    unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one; \
    (_get_arr_size_curr < _get_arr_size_temp.base || \
     _get_arr_size_curr >= _get_arr_size_last) ? 0 : \
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));})
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_plus(struct _dyneither_ptr arr,unsigned elt_sz,int change) {
  struct _dyneither_ptr _ans = (arr);
  _ans.curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _dyneither_ptr_plus(arr,elt_sz,change) ({ \
  struct _dyneither_ptr _ans = (arr); \
  _ans.curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_inplace_plus(struct _dyneither_ptr *arr_ptr,unsigned elt_sz,
                            int change) {
  struct _dyneither_ptr * _arr_ptr = (arr_ptr);
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return *_arr_ptr;
}
#else
#define _dyneither_ptr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  *_arr_ptr; })
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_inplace_plus_post(struct _dyneither_ptr *arr_ptr,unsigned elt_sz,int change) {
  struct _dyneither_ptr * _arr_ptr = (arr_ptr);
  struct _dyneither_ptr _ans = *_arr_ptr;
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _dyneither_ptr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  struct _dyneither_ptr _ans = *_arr_ptr; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

/* Decrease the upper bound on a fat pointer by numelts where sz is
   the size of the pointer's type.  Note that this can't be a macro
   if we're to get initializers right. */
static struct 
_dyneither_ptr _dyneither_ptr_decrease_size(struct _dyneither_ptr x,
                                            unsigned int sz,
                                            unsigned int numelts) {
  x.last_plus_one -= sz * numelts; 
  return x; 
}

/* Allocation */

extern void* GC_malloc(int);
extern void* GC_malloc_atomic(int);
extern void* GC_calloc(unsigned,unsigned);
extern void* GC_calloc_atomic(unsigned,unsigned);

/* FIX?  Not sure if we want to pass filename and lineno in here... */
static _INLINE void* _cycalloc(int n) {
  void * ans = (void *)GC_malloc(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static _INLINE void* _cycalloc_atomic(int n) {
  void * ans = (void *)GC_malloc_atomic(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static _INLINE void* _cyccalloc(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
static _INLINE void* _cyccalloc_atomic(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc_atomic(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
#define MAX_MALLOC_SIZE (1 << 28)
static _INLINE unsigned int _check_times(unsigned x, unsigned y) {
  _CYC_U_LONG_LONG_T whole_ans = 
    ((_CYC_U_LONG_LONG_T)x)*((_CYC_U_LONG_LONG_T)y);
  unsigned word_ans = (unsigned)whole_ans;
  if(word_ans < whole_ans || word_ans > MAX_MALLOC_SIZE)
    _throw_badalloc();
  return word_ans;
}

#if defined(CYC_REGION_PROFILE) 
extern void* _profile_GC_malloc(int,const char *file,const char *func,
                                int lineno);
extern void* _profile_GC_malloc_atomic(int,const char *file,
                                       const char *func,int lineno);
extern void* _profile_region_malloc(struct _RegionHandle *, unsigned,
                                    const char *file,
                                    const char *func,
                                    int lineno);
extern void* _profile_region_calloc(struct _RegionHandle *, unsigned,
                                    unsigned,
                                    const char *file,
                                    const char *func,
                                    int lineno);
extern struct _RegionHandle _profile_new_region(const char *rgn_name,
						const char *file,
						const char *func,
                                                int lineno);
extern void _profile_free_region(struct _RegionHandle *,
				 const char *file,
                                 const char *func,
                                 int lineno);
#  if !defined(RUNTIME_CYC)
#define _new_region(n) _profile_new_region(n,__FILE__,__FUNCTION__,__LINE__)
#define _free_region(r) _profile_free_region(r,__FILE__,__FUNCTION__,__LINE__)
#define _region_malloc(rh,n) _profile_region_malloc(rh,n,__FILE__,__FUNCTION__,__LINE__)
#define _region_calloc(rh,n,t) _profile_region_calloc(rh,n,t,__FILE__,__FUNCTION__,__LINE__)
#  endif
#define _cycalloc(n) _profile_GC_malloc(n,__FILE__,__FUNCTION__,__LINE__)
#define _cycalloc_atomic(n) _profile_GC_malloc_atomic(n,__FILE__,__FUNCTION__,__LINE__)
#endif
#endif

/* the next two routines swap [x] and [y]; not thread safe! */
static _INLINE void _swap_word(void *x, void *y) {
  unsigned long *lx = (unsigned long *)x, *ly = (unsigned long *)y, tmp;
  tmp = *lx;
  *lx = *ly;
  *ly = tmp;
}
static _INLINE void _swap_dyneither(struct _dyneither_ptr *x, 
				   struct _dyneither_ptr *y) {
  struct _dyneither_ptr tmp = *x;
  *x = *y;
  *y = tmp;
}

# 35 "core.h"
 typedef char*Cyc_Cstring;
typedef char*Cyc_CstringNN;
typedef struct _dyneither_ptr Cyc_string_t;
# 40
typedef struct _dyneither_ptr Cyc_mstring_t;
# 43
typedef struct _dyneither_ptr*Cyc_stringptr_t;
# 47
typedef struct _dyneither_ptr*Cyc_mstringptr_t;
# 50
typedef char*Cyc_Cbuffer_t;
# 52
typedef char*Cyc_CbufferNN_t;
# 54
typedef struct _dyneither_ptr Cyc_buffer_t;
# 56
typedef struct _dyneither_ptr Cyc_mbuffer_t;
# 59
typedef int Cyc_bool;struct Cyc_Core_NewRegion{struct _DynRegionHandle*dynregion;};
# 26 "cycboot.h"
typedef unsigned long Cyc_size_t;
# 33
typedef unsigned short Cyc_mode_t;struct Cyc___cycFILE;
# 49
typedef struct Cyc___cycFILE Cyc_FILE;
# 53
extern struct Cyc___cycFILE*Cyc_stderr;struct Cyc_String_pa_PrintArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Int_pa_PrintArg_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_PrintArg_struct{int tag;double f1;};struct Cyc_LongDouble_pa_PrintArg_struct{int tag;long double f1;};struct Cyc_ShortPtr_pa_PrintArg_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_PrintArg_struct{int tag;unsigned long*f1;};
# 68
typedef void*Cyc_parg_t;
# 73
struct _dyneither_ptr Cyc_aprintf(struct _dyneither_ptr,struct _dyneither_ptr);
# 100
int Cyc_fprintf(struct Cyc___cycFILE*,struct _dyneither_ptr,struct _dyneither_ptr);struct Cyc_ShortPtr_sa_ScanfArg_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_ScanfArg_struct{int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_ScanfArg_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_ScanfArg_struct{int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_DoublePtr_sa_ScanfArg_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_ScanfArg_struct{int tag;float*f1;};struct Cyc_CharPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};
# 127
typedef void*Cyc_sarg_t;
# 157 "cycboot.h"
int Cyc_printf(struct _dyneither_ptr,struct _dyneither_ptr);extern char Cyc_FileCloseError[15];struct Cyc_FileCloseError_exn_struct{char*tag;};extern char Cyc_FileOpenError[14];struct Cyc_FileOpenError_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 89 "core.h"
typedef unsigned int Cyc_Core_sizeof_t;struct Cyc_Core_Opt{void*v;};
# 93
typedef struct Cyc_Core_Opt*Cyc_Core_opt_t;extern char Cyc_Core_Invalid_argument[17];struct Cyc_Core_Invalid_argument_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Failure[8];struct Cyc_Core_Failure_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Impossible[11];struct Cyc_Core_Impossible_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Not_found[10];struct Cyc_Core_Not_found_exn_struct{char*tag;};extern char Cyc_Core_Unreachable[12];struct Cyc_Core_Unreachable_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 162 "core.h"
extern struct _RegionHandle*Cyc_Core_unique_region;extern char Cyc_Core_Open_Region[12];struct Cyc_Core_Open_Region_exn_struct{char*tag;};extern char Cyc_Core_Free_Region[12];struct Cyc_Core_Free_Region_exn_struct{char*tag;};
# 244 "core.h"
inline static void* arrcast(struct _dyneither_ptr dyn,unsigned int bd,unsigned int sz){
# 249
if(bd >> 20  || sz >> 12)
return 0;{
unsigned char*ptrbd=dyn.curr + bd * sz;
if(((ptrbd < dyn.curr  || dyn.curr == 0) || dyn.curr < dyn.base) || ptrbd > dyn.last_plus_one)
# 256
return 0;
return dyn.curr;};}struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};
# 39 "list.h"
typedef struct Cyc_List_List*Cyc_List_list_t;
# 49 "list.h"
typedef struct Cyc_List_List*Cyc_List_List_t;
# 54
struct Cyc_List_List*Cyc_List_list(struct _dyneither_ptr);
# 76
struct Cyc_List_List*Cyc_List_map(void*(*f)(void*),struct Cyc_List_List*x);
# 83
struct Cyc_List_List*Cyc_List_map_c(void*(*f)(void*,void*),void*env,struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[14];struct Cyc_List_List_mismatch_exn_struct{char*tag;};
# 123
void Cyc_List_iter_c(void(*f)(void*,void*),void*env,struct Cyc_List_List*x);
# 166
struct Cyc_List_List*Cyc_List_imp_rev(struct Cyc_List_List*x);
# 198
struct Cyc_List_List*Cyc_List_merge_sort(int(*cmp)(void*,void*),struct Cyc_List_List*x);extern char Cyc_List_Nth[4];struct Cyc_List_Nth_exn_struct{char*tag;};
# 49 "string.h"
int Cyc_strcmp(struct _dyneither_ptr s1,struct _dyneither_ptr s2);
int Cyc_strptrcmp(struct _dyneither_ptr*s1,struct _dyneither_ptr*s2);
# 64
struct _dyneither_ptr Cyc_strconcat_l(struct Cyc_List_List*);struct Cyc_Lineno_Pos{struct _dyneither_ptr logical_file;struct _dyneither_ptr line;int line_no;int col;};
# 32 "lineno.h"
typedef struct Cyc_Lineno_Pos*Cyc_Lineno_pos_t;extern char Cyc_Position_Exit[5];struct Cyc_Position_Exit_exn_struct{char*tag;};
# 37 "position.h"
typedef unsigned int Cyc_Position_seg_t;
# 42
struct Cyc_List_List*Cyc_Position_strings_of_segments(struct Cyc_List_List*);struct Cyc_Position_Lex_Position_Error_kind_struct{int tag;};struct Cyc_Position_Parse_Position_Error_kind_struct{int tag;};struct Cyc_Position_Elab_Position_Error_kind_struct{int tag;};
# 46
typedef void*Cyc_Position_error_kind_t;struct Cyc_Position_Error{struct _dyneither_ptr source;unsigned int seg;void*kind;struct _dyneither_ptr desc;};
# 53
typedef struct Cyc_Position_Error*Cyc_Position_error_t;extern char Cyc_Position_Nocontext[10];struct Cyc_Position_Nocontext_exn_struct{char*tag;};
# 61
extern int Cyc_Position_use_gcc_style_location;
# 80 "absyn.h"
typedef struct _dyneither_ptr*Cyc_Absyn_field_name_t;
typedef struct _dyneither_ptr*Cyc_Absyn_var_t;
typedef struct _dyneither_ptr*Cyc_Absyn_tvarname_t;
typedef struct _dyneither_ptr*Cyc_Absyn_var_opt_t;struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_C_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{int tag;int val;};union Cyc_Absyn_Nmspace{struct _union_Nmspace_Rel_n Rel_n;struct _union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_C_n C_n;struct _union_Nmspace_Loc_n Loc_n;};
# 92
typedef union Cyc_Absyn_Nmspace Cyc_Absyn_nmspace_t;
union Cyc_Absyn_Nmspace Cyc_Absyn_Loc_n;
union Cyc_Absyn_Nmspace Cyc_Absyn_Rel_n(struct Cyc_List_List*);
# 96
union Cyc_Absyn_Nmspace Cyc_Absyn_Abs_n(struct Cyc_List_List*ns,int C_scope);struct _tuple0{union Cyc_Absyn_Nmspace f1;struct _dyneither_ptr*f2;};
# 99
typedef struct _tuple0*Cyc_Absyn_qvar_t;typedef struct _tuple0*Cyc_Absyn_qvar_opt_t;
typedef struct _tuple0*Cyc_Absyn_typedef_name_t;
typedef struct _tuple0*Cyc_Absyn_typedef_name_opt_t;
# 104
typedef enum Cyc_Absyn_Scope Cyc_Absyn_scope_t;
typedef struct Cyc_Absyn_Tqual Cyc_Absyn_tqual_t;
typedef enum Cyc_Absyn_Size_of Cyc_Absyn_size_of_t;
typedef struct Cyc_Absyn_Kind*Cyc_Absyn_kind_t;
typedef void*Cyc_Absyn_kindbound_t;
typedef struct Cyc_Absyn_Tvar*Cyc_Absyn_tvar_t;
typedef enum Cyc_Absyn_Sign Cyc_Absyn_sign_t;
typedef enum Cyc_Absyn_AggrKind Cyc_Absyn_aggr_kind_t;
typedef void*Cyc_Absyn_bounds_t;
typedef struct Cyc_Absyn_PtrAtts Cyc_Absyn_ptr_atts_t;
typedef struct Cyc_Absyn_PtrInfo Cyc_Absyn_ptr_info_t;
typedef struct Cyc_Absyn_VarargInfo Cyc_Absyn_vararg_info_t;
typedef struct Cyc_Absyn_FnInfo Cyc_Absyn_fn_info_t;
typedef struct Cyc_Absyn_DatatypeInfo Cyc_Absyn_datatype_info_t;
typedef struct Cyc_Absyn_DatatypeFieldInfo Cyc_Absyn_datatype_field_info_t;
typedef struct Cyc_Absyn_AggrInfo Cyc_Absyn_aggr_info_t;
typedef struct Cyc_Absyn_ArrayInfo Cyc_Absyn_array_info_t;
typedef void*Cyc_Absyn_type_t;typedef void*Cyc_Absyn_rgntype_t;typedef void*Cyc_Absyn_type_opt_t;
typedef union Cyc_Absyn_Cnst Cyc_Absyn_cnst_t;
typedef enum Cyc_Absyn_Primop Cyc_Absyn_primop_t;
typedef enum Cyc_Absyn_Incrementor Cyc_Absyn_incrementor_t;
typedef struct Cyc_Absyn_VarargCallInfo Cyc_Absyn_vararg_call_info_t;
typedef void*Cyc_Absyn_raw_exp_t;
typedef struct Cyc_Absyn_Exp*Cyc_Absyn_exp_t;typedef struct Cyc_Absyn_Exp*Cyc_Absyn_exp_opt_t;
typedef void*Cyc_Absyn_raw_stmt_t;
typedef struct Cyc_Absyn_Stmt*Cyc_Absyn_stmt_t;typedef struct Cyc_Absyn_Stmt*Cyc_Absyn_stmt_opt_t;
typedef void*Cyc_Absyn_raw_pat_t;
typedef struct Cyc_Absyn_Pat*Cyc_Absyn_pat_t;
typedef void*Cyc_Absyn_binding_t;
typedef struct Cyc_Absyn_Switch_clause*Cyc_Absyn_switch_clause_t;
typedef struct Cyc_Absyn_Fndecl*Cyc_Absyn_fndecl_t;
typedef struct Cyc_Absyn_Aggrdecl*Cyc_Absyn_aggrdecl_t;
typedef struct Cyc_Absyn_Datatypefield*Cyc_Absyn_datatypefield_t;
typedef struct Cyc_Absyn_Datatypedecl*Cyc_Absyn_datatypedecl_t;
typedef struct Cyc_Absyn_Typedefdecl*Cyc_Absyn_typedefdecl_t;
typedef struct Cyc_Absyn_Enumfield*Cyc_Absyn_enumfield_t;
typedef struct Cyc_Absyn_Enumdecl*Cyc_Absyn_enumdecl_t;
typedef struct Cyc_Absyn_Vardecl*Cyc_Absyn_vardecl_t;
typedef void*Cyc_Absyn_raw_decl_t;
typedef struct Cyc_Absyn_Decl*Cyc_Absyn_decl_t;
typedef void*Cyc_Absyn_designator_t;
typedef void*Cyc_Absyn_absyn_annot_t;
typedef void*Cyc_Absyn_attribute_t;
typedef struct Cyc_List_List*Cyc_Absyn_attributes_t;
typedef struct Cyc_Absyn_Aggrfield*Cyc_Absyn_aggrfield_t;
typedef void*Cyc_Absyn_offsetof_field_t;
typedef struct Cyc_Absyn_MallocInfo Cyc_Absyn_malloc_info_t;
typedef enum Cyc_Absyn_Coercion Cyc_Absyn_coercion_t;
typedef struct Cyc_Absyn_PtrLoc*Cyc_Absyn_ptrloc_t;
# 155
enum Cyc_Absyn_Scope{Cyc_Absyn_Static  = 0,Cyc_Absyn_Abstract  = 1,Cyc_Absyn_Public  = 2,Cyc_Absyn_Extern  = 3,Cyc_Absyn_ExternC  = 4,Cyc_Absyn_Register  = 5};struct Cyc_Absyn_Tqual{int print_const;int q_volatile;int q_restrict;int real_const;unsigned int loc;};
# 176
enum Cyc_Absyn_Size_of{Cyc_Absyn_Char_sz  = 0,Cyc_Absyn_Short_sz  = 1,Cyc_Absyn_Int_sz  = 2,Cyc_Absyn_Long_sz  = 3,Cyc_Absyn_LongLong_sz  = 4};
# 181
enum Cyc_Absyn_AliasQual{Cyc_Absyn_Aliasable  = 0,Cyc_Absyn_Unique  = 1,Cyc_Absyn_Top  = 2};
# 188
enum Cyc_Absyn_KindQual{Cyc_Absyn_AnyKind  = 0,Cyc_Absyn_MemKind  = 1,Cyc_Absyn_BoxKind  = 2,Cyc_Absyn_RgnKind  = 3,Cyc_Absyn_EffKind  = 4,Cyc_Absyn_IntKind  = 5};struct Cyc_Absyn_Kind{enum Cyc_Absyn_KindQual kind;enum Cyc_Absyn_AliasQual aliasqual;};
# 208
enum Cyc_Absyn_Sign{Cyc_Absyn_Signed  = 0,Cyc_Absyn_Unsigned  = 1,Cyc_Absyn_None  = 2};
# 210
enum Cyc_Absyn_AggrKind{Cyc_Absyn_StructA  = 0,Cyc_Absyn_UnionA  = 1};struct _union_Constraint_Eq_constr{int tag;void*val;};struct _union_Constraint_Forward_constr{int tag;union Cyc_Absyn_Constraint*val;};struct _union_Constraint_No_constr{int tag;int val;};union Cyc_Absyn_Constraint{struct _union_Constraint_Eq_constr Eq_constr;struct _union_Constraint_Forward_constr Forward_constr;struct _union_Constraint_No_constr No_constr;};
# 219
typedef union Cyc_Absyn_Constraint*Cyc_Absyn_conref_t;struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct{int tag;struct Cyc_Absyn_Kind*f1;};struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_Tvar{struct _dyneither_ptr*name;int identity;void*kind;};struct Cyc_Absyn_DynEither_b_Absyn_Bounds_struct{int tag;};struct Cyc_Absyn_Upper_b_Absyn_Bounds_struct{int tag;struct Cyc_Absyn_Exp*f1;};
# 245
extern struct Cyc_Absyn_DynEither_b_Absyn_Bounds_struct Cyc_Absyn_DynEither_b_val;struct Cyc_Absyn_PtrLoc{unsigned int ptr_loc;unsigned int rgn_loc;unsigned int zt_loc;};struct Cyc_Absyn_PtrAtts{void*rgn;union Cyc_Absyn_Constraint*nullable;union Cyc_Absyn_Constraint*bounds;union Cyc_Absyn_Constraint*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;};struct Cyc_Absyn_PtrInfo{void*elt_typ;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};struct Cyc_Absyn_Numelts_ptrqual_Absyn_Pointer_qual_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Region_ptrqual_Absyn_Pointer_qual_struct{int tag;void*f1;};struct Cyc_Absyn_Thin_ptrqual_Absyn_Pointer_qual_struct{int tag;};struct Cyc_Absyn_Fat_ptrqual_Absyn_Pointer_qual_struct{int tag;};struct Cyc_Absyn_Zeroterm_ptrqual_Absyn_Pointer_qual_struct{int tag;};struct Cyc_Absyn_Nozeroterm_ptrqual_Absyn_Pointer_qual_struct{int tag;};struct Cyc_Absyn_Notnull_ptrqual_Absyn_Pointer_qual_struct{int tag;};struct Cyc_Absyn_Nullable_ptrqual_Absyn_Pointer_qual_struct{int tag;};
# 280
typedef void*Cyc_Absyn_pointer_qual_t;
typedef struct Cyc_List_List*Cyc_Absyn_pointer_quals_t;struct Cyc_Absyn_VarargInfo{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_typ;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*attributes;};struct Cyc_Absyn_UnknownDatatypeInfo{struct _tuple0*name;int is_extensible;};struct _union_DatatypeInfoU_UnknownDatatype{int tag;struct Cyc_Absyn_UnknownDatatypeInfo val;};struct _union_DatatypeInfoU_KnownDatatype{int tag;struct Cyc_Absyn_Datatypedecl**val;};union Cyc_Absyn_DatatypeInfoU{struct _union_DatatypeInfoU_UnknownDatatype UnknownDatatype;struct _union_DatatypeInfoU_KnownDatatype KnownDatatype;};struct Cyc_Absyn_DatatypeInfo{union Cyc_Absyn_DatatypeInfoU datatype_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownDatatypeFieldInfo{struct _tuple0*datatype_name;struct _tuple0*field_name;int is_extensible;};struct _union_DatatypeFieldInfoU_UnknownDatatypefield{int tag;struct Cyc_Absyn_UnknownDatatypeFieldInfo val;};struct _tuple1{struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct _union_DatatypeFieldInfoU_KnownDatatypefield{int tag;struct _tuple1 val;};union Cyc_Absyn_DatatypeFieldInfoU{struct _union_DatatypeFieldInfoU_UnknownDatatypefield UnknownDatatypefield;struct _union_DatatypeFieldInfoU_KnownDatatypefield KnownDatatypefield;};struct Cyc_Absyn_DatatypeFieldInfo{union Cyc_Absyn_DatatypeFieldInfoU field_info;struct Cyc_List_List*targs;};struct _tuple2{enum Cyc_Absyn_AggrKind f1;struct _tuple0*f2;struct Cyc_Core_Opt*f3;};struct _union_AggrInfoU_UnknownAggr{int tag;struct _tuple2 val;};struct _union_AggrInfoU_KnownAggr{int tag;struct Cyc_Absyn_Aggrdecl**val;};union Cyc_Absyn_AggrInfoU{struct _union_AggrInfoU_UnknownAggr UnknownAggr;struct _union_AggrInfoU_KnownAggr KnownAggr;};struct Cyc_Absyn_AggrInfo{union Cyc_Absyn_AggrInfoU aggr_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;union Cyc_Absyn_Constraint*zero_term;unsigned int zt_loc;};struct Cyc_Absyn_Aggr_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Enum_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Datatype_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};
# 373
typedef void*Cyc_Absyn_raw_type_decl_t;struct Cyc_Absyn_TypeDecl{void*r;unsigned int loc;};
# 378
typedef struct Cyc_Absyn_TypeDecl*Cyc_Absyn_type_decl_t;struct Cyc_Absyn_VoidType_Absyn_Type_struct{int tag;};struct Cyc_Absyn_Evar_Absyn_Type_struct{int tag;struct Cyc_Core_Opt*f1;void*f2;int f3;struct Cyc_Core_Opt*f4;};struct Cyc_Absyn_VarType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_DatatypeType_Absyn_Type_struct{int tag;struct Cyc_Absyn_DatatypeInfo f1;};struct Cyc_Absyn_DatatypeFieldType_Absyn_Type_struct{int tag;struct Cyc_Absyn_DatatypeFieldInfo f1;};struct Cyc_Absyn_PointerType_Absyn_Type_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_IntType_Absyn_Type_struct{int tag;enum Cyc_Absyn_Sign f1;enum Cyc_Absyn_Size_of f2;};struct Cyc_Absyn_FloatType_Absyn_Type_struct{int tag;int f1;};struct Cyc_Absyn_ArrayType_Absyn_Type_struct{int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_Absyn_Type_struct{int tag;struct Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_AggrType_Absyn_Type_struct{int tag;struct Cyc_Absyn_AggrInfo f1;};struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct{int tag;enum Cyc_Absyn_AggrKind f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_EnumType_Absyn_Type_struct{int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnHandleType_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_DynRgnType_Absyn_Type_struct{int tag;void*f1;void*f2;};struct Cyc_Absyn_TypedefType_Absyn_Type_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void*f4;};struct Cyc_Absyn_ValueofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_TagType_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_HeapRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_UniqueRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_RefCntRgn_Absyn_Type_struct{int tag;};struct Cyc_Absyn_AccessEff_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_JoinEff_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnsEff_Absyn_Type_struct{int tag;void*f1;};struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct{int tag;struct Cyc_Absyn_TypeDecl*f1;void**f2;};struct Cyc_Absyn_NoTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;};struct Cyc_Absyn_WithTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;void*f4;struct Cyc_List_List*f5;};
# 444 "absyn.h"
typedef void*Cyc_Absyn_funcparams_t;
# 447
enum Cyc_Absyn_Format_Type{Cyc_Absyn_Printf_ft  = 0,Cyc_Absyn_Scanf_ft  = 1};struct Cyc_Absyn_Regparm_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Stdcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Cdecl_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Fastcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Noreturn_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Const_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Aligned_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Packed_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Section_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Nocommon_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Shared_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Unused_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Weak_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllimport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllexport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_instrument_function_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Constructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Destructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_check_memory_usage_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Format_att_Absyn_Attribute_struct{int tag;enum Cyc_Absyn_Format_Type f1;int f2;int f3;};struct Cyc_Absyn_Initializes_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noliveunique_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noconsume_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Pure_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Mode_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Carray_mod_Absyn_Type_modifier_struct{int tag;union Cyc_Absyn_Constraint*f1;unsigned int f2;};struct Cyc_Absyn_ConstArray_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_Exp*f1;union Cyc_Absyn_Constraint*f2;unsigned int f3;};struct Cyc_Absyn_Pointer_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_Absyn_Type_modifier_struct{int tag;void*f1;};struct Cyc_Absyn_TypeParams_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;int f3;};struct Cyc_Absyn_Attributes_mod_Absyn_Type_modifier_struct{int tag;unsigned int f1;struct Cyc_List_List*f2;};
# 508
typedef void*Cyc_Absyn_type_modifier_t;struct _union_Cnst_Null_c{int tag;int val;};struct _tuple3{enum Cyc_Absyn_Sign f1;char f2;};struct _union_Cnst_Char_c{int tag;struct _tuple3 val;};struct _union_Cnst_Wchar_c{int tag;struct _dyneither_ptr val;};struct _tuple4{enum Cyc_Absyn_Sign f1;short f2;};struct _union_Cnst_Short_c{int tag;struct _tuple4 val;};struct _tuple5{enum Cyc_Absyn_Sign f1;int f2;};struct _union_Cnst_Int_c{int tag;struct _tuple5 val;};struct _tuple6{enum Cyc_Absyn_Sign f1;long long f2;};struct _union_Cnst_LongLong_c{int tag;struct _tuple6 val;};struct _tuple7{struct _dyneither_ptr f1;int f2;};struct _union_Cnst_Float_c{int tag;struct _tuple7 val;};struct _union_Cnst_String_c{int tag;struct _dyneither_ptr val;};struct _union_Cnst_Wstring_c{int tag;struct _dyneither_ptr val;};union Cyc_Absyn_Cnst{struct _union_Cnst_Null_c Null_c;struct _union_Cnst_Char_c Char_c;struct _union_Cnst_Wchar_c Wchar_c;struct _union_Cnst_Short_c Short_c;struct _union_Cnst_Int_c Int_c;struct _union_Cnst_LongLong_c LongLong_c;struct _union_Cnst_Float_c Float_c;struct _union_Cnst_String_c String_c;struct _union_Cnst_Wstring_c Wstring_c;};
# 534
enum Cyc_Absyn_Primop{Cyc_Absyn_Plus  = 0,Cyc_Absyn_Times  = 1,Cyc_Absyn_Minus  = 2,Cyc_Absyn_Div  = 3,Cyc_Absyn_Mod  = 4,Cyc_Absyn_Eq  = 5,Cyc_Absyn_Neq  = 6,Cyc_Absyn_Gt  = 7,Cyc_Absyn_Lt  = 8,Cyc_Absyn_Gte  = 9,Cyc_Absyn_Lte  = 10,Cyc_Absyn_Not  = 11,Cyc_Absyn_Bitnot  = 12,Cyc_Absyn_Bitand  = 13,Cyc_Absyn_Bitor  = 14,Cyc_Absyn_Bitxor  = 15,Cyc_Absyn_Bitlshift  = 16,Cyc_Absyn_Bitlrshift  = 17,Cyc_Absyn_Bitarshift  = 18,Cyc_Absyn_Numelts  = 19};
# 541
enum Cyc_Absyn_Incrementor{Cyc_Absyn_PreInc  = 0,Cyc_Absyn_PostInc  = 1,Cyc_Absyn_PreDec  = 2,Cyc_Absyn_PostDec  = 3};struct Cyc_Absyn_VarargCallInfo{int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_TupleIndex_Absyn_OffsetofField_struct{int tag;unsigned int f1;};
# 559
enum Cyc_Absyn_Coercion{Cyc_Absyn_Unknown_coercion  = 0,Cyc_Absyn_No_coercion  = 1,Cyc_Absyn_NonNull_to_Null  = 2,Cyc_Absyn_Other_coercion  = 3};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;};struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct{int tag;union Cyc_Absyn_Cnst f1;};struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct{int tag;struct _tuple0*f1;void*f2;};struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct{int tag;enum Cyc_Absyn_Primop f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;enum Cyc_Absyn_Incrementor f2;};struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;int f4;};struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;enum Cyc_Absyn_Coercion f4;};struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftyp_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct{int tag;void*f1;void*f2;};struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct _tuple8{struct _dyneither_ptr*f1;struct Cyc_Absyn_Tqual f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct{int tag;struct _tuple8*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;int f4;};struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*f4;};struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Datatypedecl*f2;struct Cyc_Absyn_Datatypefield*f3;};struct Cyc_Absyn_Enum_e_Absyn_Raw_exp_struct{int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_AnonEnum_e_Absyn_Raw_exp_struct{int tag;struct _tuple0*f1;void*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;};struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Asm_e_Absyn_Raw_exp_struct{int tag;int f1;struct _dyneither_ptr f2;};struct Cyc_Absyn_Exp{void*topt;void*r;unsigned int loc;void*annot;};struct Cyc_Absyn_Skip_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};struct _tuple9{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct{int tag;struct _tuple9 f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Continue_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Goto_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple9 f2;struct _tuple9 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple9 f2;};struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ResetRegion_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Stmt{void*r;unsigned int loc;struct Cyc_List_List*non_local_preds;int try_depth;void*annot;};struct Cyc_Absyn_Wild_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_Absyn_Raw_pat_struct{int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_AggrInfo*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Null_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct{int tag;enum Cyc_Absyn_Sign f1;int f2;};struct Cyc_Absyn_Char_p_Absyn_Raw_pat_struct{int tag;char f1;};struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct{int tag;struct _dyneither_ptr f1;int f2;};struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_UnknownId_p_Absyn_Raw_pat_struct{int tag;struct _tuple0*f1;};struct Cyc_Absyn_UnknownCall_p_Absyn_Raw_pat_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Pat{void*r;void*topt;unsigned int loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*body;unsigned int loc;};struct Cyc_Absyn_Unresolved_b_Absyn_Binding_struct{int tag;};struct Cyc_Absyn_Global_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Param_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Local_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;void*rgn;struct Cyc_List_List*attributes;int escapes;};struct Cyc_Absyn_Fndecl{enum Cyc_Absyn_Scope sc;int is_inline;struct _tuple0*name;struct Cyc_List_List*tvs;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;void*cached_typ;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Aggrfield{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct Cyc_List_List*rgn_po;struct Cyc_List_List*fields;int tagged;};struct Cyc_Absyn_Aggrdecl{enum Cyc_Absyn_AggrKind kind;enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*impl;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Datatypefield{struct _tuple0*name;struct Cyc_List_List*typs;unsigned int loc;enum Cyc_Absyn_Scope sc;};struct Cyc_Absyn_Datatypedecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int is_extensible;};struct Cyc_Absyn_Enumfield{struct _tuple0*name;struct Cyc_Absyn_Exp*tag;unsigned int loc;};struct Cyc_Absyn_Enumdecl{enum Cyc_Absyn_Scope sc;struct _tuple0*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct _tuple0*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*kind;void*defn;struct Cyc_List_List*atts;};struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Letv_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;int f3;struct Cyc_Absyn_Exp*f4;};struct Cyc_Absyn_Alias_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Datatype_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_ExternCinclude_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Porton_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Portoff_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Decl{void*r;unsigned int loc;};struct Cyc_Absyn_ArrayElement_Absyn_Designator_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_Absyn_Designator_struct{int tag;struct _dyneither_ptr*f1;};extern char Cyc_Absyn_EmptyAnnot[11];struct Cyc_Absyn_EmptyAnnot_Absyn_AbsynAnnot_struct{char*tag;};
# 905 "absyn.h"
int Cyc_Absyn_qvar_cmp(struct _tuple0*,struct _tuple0*);
# 915
struct Cyc_Absyn_Tqual Cyc_Absyn_empty_tqual(unsigned int);
# 919
union Cyc_Absyn_Constraint*Cyc_Absyn_empty_conref();
# 922
void*Cyc_Absyn_conref_val(union Cyc_Absyn_Constraint*x);
# 940
void*Cyc_Absyn_wildtyp(struct Cyc_Core_Opt*);
# 945
extern void*Cyc_Absyn_sint_typ;
# 962
void*Cyc_Absyn_string_typ(void*rgn);
# 984
void*Cyc_Absyn_dyneither_typ(void*t,void*rgn,struct Cyc_Absyn_Tqual tq,union Cyc_Absyn_Constraint*zero_term);struct Cyc_PP_Ppstate;
# 41 "pp.h"
typedef struct Cyc_PP_Ppstate*Cyc_PP_ppstate_t;struct Cyc_PP_Out;
# 43
typedef struct Cyc_PP_Out*Cyc_PP_out_t;struct Cyc_PP_Doc;
# 45
typedef struct Cyc_PP_Doc*Cyc_PP_doc_t;struct Cyc_Absynpp_Params{int expand_typedefs;int qvar_to_Cids;int add_cyc_prefix;int to_VC;int decls_first;int rewrite_temp_tvars;int print_all_tvars;int print_all_kinds;int print_all_effects;int print_using_stmts;int print_externC_stmts;int print_full_evars;int print_zeroterm;int generate_line_directives;int use_curr_namespace;struct Cyc_List_List*curr_namespace;};
# 68 "absynpp.h"
struct _dyneither_ptr Cyc_Absynpp_exp2string(struct Cyc_Absyn_Exp*);
# 70
struct _dyneither_ptr Cyc_Absynpp_qvar2string(struct _tuple0*);struct Cyc_Iter_Iter{void*env;int(*next)(void*env,void*dest);};
# 34 "iter.h"
typedef struct Cyc_Iter_Iter Cyc_Iter_iter_t;
# 37
int Cyc_Iter_next(struct Cyc_Iter_Iter,void*);struct Cyc_Set_Set;
# 40 "set.h"
typedef struct Cyc_Set_Set*Cyc_Set_set_t;extern char Cyc_Set_Absent[7];struct Cyc_Set_Absent_exn_struct{char*tag;};struct Cyc_Dict_T;
# 46 "dict.h"
typedef const struct Cyc_Dict_T*Cyc_Dict_tree;struct Cyc_Dict_Dict{int(*rel)(void*,void*);struct _RegionHandle*r;const struct Cyc_Dict_T*t;};
# 52
typedef struct Cyc_Dict_Dict Cyc_Dict_dict_t;extern char Cyc_Dict_Present[8];struct Cyc_Dict_Present_exn_struct{char*tag;};extern char Cyc_Dict_Absent[7];struct Cyc_Dict_Absent_exn_struct{char*tag;};
# 62
struct Cyc_Dict_Dict Cyc_Dict_empty(int(*cmp)(void*,void*));
# 83
int Cyc_Dict_member(struct Cyc_Dict_Dict d,void*k);
# 87
struct Cyc_Dict_Dict Cyc_Dict_insert(struct Cyc_Dict_Dict d,void*k,void*v);
# 110
void*Cyc_Dict_lookup(struct Cyc_Dict_Dict d,void*k);
# 122 "dict.h"
void**Cyc_Dict_lookup_opt(struct Cyc_Dict_Dict d,void*k);struct Cyc_RgnOrder_RgnPO;
# 33 "rgnorder.h"
typedef struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_rgn_po_t;
# 35
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_initial_fn_po(struct _RegionHandle*,struct Cyc_List_List*tvs,struct Cyc_List_List*po,void*effect,struct Cyc_Absyn_Tvar*fst_rgn,unsigned int);
# 42
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_outlives_constraint(struct _RegionHandle*,struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn,unsigned int loc);
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_youngest(struct _RegionHandle*,struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*rgn,int resetable,int opened);
int Cyc_RgnOrder_is_region_resetable(struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*r);
int Cyc_RgnOrder_effect_outlives(struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn);
int Cyc_RgnOrder_satisfies_constraints(struct Cyc_RgnOrder_RgnPO*po,struct Cyc_List_List*constraints,void*default_bound,int do_pin);
# 48
int Cyc_RgnOrder_eff_outlives_eff(struct Cyc_RgnOrder_RgnPO*po,void*eff1,void*eff2);
# 51
void Cyc_RgnOrder_print_region_po(struct Cyc_RgnOrder_RgnPO*po);struct Cyc_Tcenv_CList{void*hd;const struct Cyc_Tcenv_CList*tl;};
# 40 "tcenv.h"
typedef const struct Cyc_Tcenv_CList*Cyc_Tcenv_mclist_t;
typedef const struct Cyc_Tcenv_CList*const Cyc_Tcenv_clist_t;struct Cyc_Tcenv_VarRes_Tcenv_Resolved_struct{int tag;void*f1;};struct Cyc_Tcenv_AggrRes_Tcenv_Resolved_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Tcenv_DatatypeRes_Tcenv_Resolved_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct Cyc_Tcenv_EnumRes_Tcenv_Resolved_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_AnonEnumRes_Tcenv_Resolved_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};
# 51
typedef void*Cyc_Tcenv_resolved_t;struct Cyc_Tcenv_Genv{struct _RegionHandle*grgn;struct Cyc_Set_Set*namespaces;struct Cyc_Dict_Dict aggrdecls;struct Cyc_Dict_Dict datatypedecls;struct Cyc_Dict_Dict enumdecls;struct Cyc_Dict_Dict typedefs;struct Cyc_Dict_Dict ordinaries;struct Cyc_List_List*availables;};
# 70
typedef struct Cyc_Tcenv_Genv*Cyc_Tcenv_genv_t;struct Cyc_Tcenv_Fenv;
# 74
typedef struct Cyc_Tcenv_Fenv*Cyc_Tcenv_fenv_t;struct Cyc_Tcenv_NotLoop_j_Tcenv_Jumpee_struct{int tag;};struct Cyc_Tcenv_CaseEnd_j_Tcenv_Jumpee_struct{int tag;};struct Cyc_Tcenv_FnEnd_j_Tcenv_Jumpee_struct{int tag;};struct Cyc_Tcenv_Stmt_j_Tcenv_Jumpee_struct{int tag;struct Cyc_Absyn_Stmt*f1;};
# 85
typedef void*Cyc_Tcenv_jumpee_t;struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Dict_Dict ae;struct Cyc_Tcenv_Fenv*le;int allow_valueof;};
# 96
typedef struct Cyc_Tcenv_Tenv*Cyc_Tcenv_tenv_t;
# 130 "tcenv.h"
enum Cyc_Tcenv_NewStatus{Cyc_Tcenv_NoneNew  = 0,Cyc_Tcenv_InNew  = 1,Cyc_Tcenv_InNewAggr  = 2};
# 38 "tcutil.h"
void*Cyc_Tcutil_impos(struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 342 "tcutil.h"
void Cyc_Tcutil_check_no_qual(unsigned int loc,void*t);struct _tuple10{void*f1;void*f2;};struct Cyc_Tcexp_TestEnv{struct _tuple10*eq;int isTrue;};
# 39 "tcexp.h"
typedef struct Cyc_Tcexp_TestEnv Cyc_Tcexp_testenv_t;
struct Cyc_Tcexp_TestEnv Cyc_Tcexp_tcTest(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e,struct _dyneither_ptr msg_part);extern char Cyc_Tcdecl_Incompatible[13];struct Cyc_Tcdecl_Incompatible_exn_struct{char*tag;};struct Cyc_Tcdecl_Xdatatypefielddecl{struct Cyc_Absyn_Datatypedecl*base;struct Cyc_Absyn_Datatypefield*field;};
# 41 "tcdecl.h"
typedef struct Cyc_Tcdecl_Xdatatypefielddecl*Cyc_Tcdecl_xdatatypefielddecl_t;struct Cyc_Position_Segment{int start;int end;};struct Cyc_Port_Edit{unsigned int loc;struct _dyneither_ptr old_string;struct _dyneither_ptr new_string;};
# 92 "port.cyc"
typedef struct Cyc_Port_Edit*Cyc_Port_edit_t;
# 97
typedef struct Cyc_List_List*Cyc_Port_edits_t;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;
# 99
int Cyc_Port_cmp_edit(struct Cyc_Port_Edit*e1,struct Cyc_Port_Edit*e2){
return(int)e1 - (int)e2;}
# 107
typedef void*Cyc_Port_ctype_t;struct Cyc_Port_Cvar{int id;void**eq;struct Cyc_List_List*uppers;struct Cyc_List_List*lowers;};
# 109
typedef struct Cyc_Port_Cvar*Cyc_Port_cvar_t;struct Cyc_Port_Cfield{void*qual;struct _dyneither_ptr*f;void*type;};
# 116
typedef struct Cyc_Port_Cfield*Cyc_Port_cfield_t;
# 121
typedef struct Cyc_List_List*Cyc_Port_cfields_t;struct Cyc_Port_Const_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Notconst_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Thin_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Fat_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Void_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Zero_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Arith_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Heap_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Zterm_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_Nozterm_ct_Port_Ctype_struct{int tag;};struct Cyc_Port_RgnVar_ct_Port_Ctype_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Port_Ptr_ct_Port_Ctype_struct{int tag;void*f1;void*f2;void*f3;void*f4;void*f5;};struct Cyc_Port_Array_ct_Port_Ctype_struct{int tag;void*f1;void*f2;void*f3;};struct _tuple11{struct Cyc_Absyn_Aggrdecl*f1;struct Cyc_List_List*f2;};struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct{int tag;struct _tuple11*f1;};struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct{int tag;struct Cyc_List_List*f1;void**f2;};struct Cyc_Port_Fn_ct_Port_Ctype_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Port_Cvar;struct Cyc_Port_Var_ct_Port_Ctype_struct{int tag;struct Cyc_Port_Cvar*f1;};
# 151
struct Cyc_Port_Const_ct_Port_Ctype_struct Cyc_Port_Const_ct_val={0};
struct Cyc_Port_Notconst_ct_Port_Ctype_struct Cyc_Port_Notconst_ct_val={1};
struct Cyc_Port_Thin_ct_Port_Ctype_struct Cyc_Port_Thin_ct_val={2};
struct Cyc_Port_Fat_ct_Port_Ctype_struct Cyc_Port_Fat_ct_val={3};
struct Cyc_Port_Void_ct_Port_Ctype_struct Cyc_Port_Void_ct_val={4};
struct Cyc_Port_Zero_ct_Port_Ctype_struct Cyc_Port_Zero_ct_val={5};
struct Cyc_Port_Arith_ct_Port_Ctype_struct Cyc_Port_Arith_ct_val={6};
struct Cyc_Port_Heap_ct_Port_Ctype_struct Cyc_Port_Heap_ct_val={7};
struct Cyc_Port_Zterm_ct_Port_Ctype_struct Cyc_Port_Zterm_ct_val={8};
struct Cyc_Port_Nozterm_ct_Port_Ctype_struct Cyc_Port_Nozterm_ct_val={9};
# 164
static struct _dyneither_ptr Cyc_Port_ctypes2string(int deep,struct Cyc_List_List*ts);
static struct _dyneither_ptr Cyc_Port_cfields2string(int deep,struct Cyc_List_List*ts);struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;
static struct _dyneither_ptr Cyc_Port_ctype2string(int deep,void*t){
void*_tmpA=t;struct _dyneither_ptr*_tmp16;void*_tmp18;void*_tmp19;void*_tmp1A;void*_tmp1B;void*_tmp1C;void*_tmp1E;void*_tmp1F;void*_tmp20;struct Cyc_Absyn_Aggrdecl*_tmp22;struct Cyc_List_List*_tmp23;struct Cyc_List_List*_tmp25;void*_tmp26;struct Cyc_List_List*_tmp28;void*_tmp2A;struct Cyc_List_List*_tmp2B;struct Cyc_Port_Cvar*_tmp2D;_LL1: {struct Cyc_Port_Const_ct_Port_Ctype_struct*_tmpB=(struct Cyc_Port_Const_ct_Port_Ctype_struct*)_tmpA;if(_tmpB->tag != 0)goto _LL3;}_LL2: {
const char*_tmp493;return(_tmp493="const",_tag_dyneither(_tmp493,sizeof(char),6));}_LL3: {struct Cyc_Port_Notconst_ct_Port_Ctype_struct*_tmpC=(struct Cyc_Port_Notconst_ct_Port_Ctype_struct*)_tmpA;if(_tmpC->tag != 1)goto _LL5;}_LL4: {
const char*_tmp494;return(_tmp494="notconst",_tag_dyneither(_tmp494,sizeof(char),9));}_LL5: {struct Cyc_Port_Thin_ct_Port_Ctype_struct*_tmpD=(struct Cyc_Port_Thin_ct_Port_Ctype_struct*)_tmpA;if(_tmpD->tag != 2)goto _LL7;}_LL6: {
const char*_tmp495;return(_tmp495="thin",_tag_dyneither(_tmp495,sizeof(char),5));}_LL7: {struct Cyc_Port_Fat_ct_Port_Ctype_struct*_tmpE=(struct Cyc_Port_Fat_ct_Port_Ctype_struct*)_tmpA;if(_tmpE->tag != 3)goto _LL9;}_LL8: {
const char*_tmp496;return(_tmp496="fat",_tag_dyneither(_tmp496,sizeof(char),4));}_LL9: {struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmpF=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmpA;if(_tmpF->tag != 4)goto _LLB;}_LLA: {
const char*_tmp497;return(_tmp497="void",_tag_dyneither(_tmp497,sizeof(char),5));}_LLB: {struct Cyc_Port_Zero_ct_Port_Ctype_struct*_tmp10=(struct Cyc_Port_Zero_ct_Port_Ctype_struct*)_tmpA;if(_tmp10->tag != 5)goto _LLD;}_LLC: {
const char*_tmp498;return(_tmp498="zero",_tag_dyneither(_tmp498,sizeof(char),5));}_LLD: {struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmp11=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmpA;if(_tmp11->tag != 6)goto _LLF;}_LLE: {
const char*_tmp499;return(_tmp499="arith",_tag_dyneither(_tmp499,sizeof(char),6));}_LLF: {struct Cyc_Port_Heap_ct_Port_Ctype_struct*_tmp12=(struct Cyc_Port_Heap_ct_Port_Ctype_struct*)_tmpA;if(_tmp12->tag != 7)goto _LL11;}_LL10: {
const char*_tmp49A;return(_tmp49A="heap",_tag_dyneither(_tmp49A,sizeof(char),5));}_LL11: {struct Cyc_Port_Zterm_ct_Port_Ctype_struct*_tmp13=(struct Cyc_Port_Zterm_ct_Port_Ctype_struct*)_tmpA;if(_tmp13->tag != 8)goto _LL13;}_LL12: {
const char*_tmp49B;return(_tmp49B="ZT",_tag_dyneither(_tmp49B,sizeof(char),3));}_LL13: {struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*_tmp14=(struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*)_tmpA;if(_tmp14->tag != 9)goto _LL15;}_LL14: {
const char*_tmp49C;return(_tmp49C="NZT",_tag_dyneither(_tmp49C,sizeof(char),4));}_LL15: {struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmp15=(struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmpA;if(_tmp15->tag != 10)goto _LL17;else{_tmp16=_tmp15->f1;}}_LL16: {
const char*_tmp4A0;void*_tmp49F[1];struct Cyc_String_pa_PrintArg_struct _tmp49E;return(struct _dyneither_ptr)((_tmp49E.tag=0,((_tmp49E.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp16),((_tmp49F[0]=& _tmp49E,Cyc_aprintf(((_tmp4A0="%s",_tag_dyneither(_tmp4A0,sizeof(char),3))),_tag_dyneither(_tmp49F,sizeof(void*),1))))))));}_LL17: {struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp17=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpA;if(_tmp17->tag != 11)goto _LL19;else{_tmp18=(void*)_tmp17->f1;_tmp19=(void*)_tmp17->f2;_tmp1A=(void*)_tmp17->f3;_tmp1B=(void*)_tmp17->f4;_tmp1C=(void*)_tmp17->f5;}}_LL18: {
# 180
const char*_tmp4A8;void*_tmp4A7[5];struct Cyc_String_pa_PrintArg_struct _tmp4A6;struct Cyc_String_pa_PrintArg_struct _tmp4A5;struct Cyc_String_pa_PrintArg_struct _tmp4A4;struct Cyc_String_pa_PrintArg_struct _tmp4A3;struct Cyc_String_pa_PrintArg_struct _tmp4A2;return(struct _dyneither_ptr)((_tmp4A2.tag=0,((_tmp4A2.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 182
Cyc_Port_ctype2string(deep,_tmp1C)),((_tmp4A3.tag=0,((_tmp4A3.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp1B)),((_tmp4A4.tag=0,((_tmp4A4.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 181
Cyc_Port_ctype2string(deep,_tmp1A)),((_tmp4A5.tag=0,((_tmp4A5.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp19)),((_tmp4A6.tag=0,((_tmp4A6.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 180
Cyc_Port_ctype2string(deep,_tmp18)),((_tmp4A7[0]=& _tmp4A6,((_tmp4A7[1]=& _tmp4A5,((_tmp4A7[2]=& _tmp4A4,((_tmp4A7[3]=& _tmp4A3,((_tmp4A7[4]=& _tmp4A2,Cyc_aprintf(((_tmp4A8="ptr(%s,%s,%s,%s,%s)",_tag_dyneither(_tmp4A8,sizeof(char),20))),_tag_dyneither(_tmp4A7,sizeof(void*),5))))))))))))))))))))))))))))))));}_LL19: {struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp1D=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpA;if(_tmp1D->tag != 12)goto _LL1B;else{_tmp1E=(void*)_tmp1D->f1;_tmp1F=(void*)_tmp1D->f2;_tmp20=(void*)_tmp1D->f3;}}_LL1A: {
# 184
const char*_tmp4AE;void*_tmp4AD[3];struct Cyc_String_pa_PrintArg_struct _tmp4AC;struct Cyc_String_pa_PrintArg_struct _tmp4AB;struct Cyc_String_pa_PrintArg_struct _tmp4AA;return(struct _dyneither_ptr)((_tmp4AA.tag=0,((_tmp4AA.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Port_ctype2string(deep,_tmp20)),((_tmp4AB.tag=0,((_tmp4AB.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp1F)),((_tmp4AC.tag=0,((_tmp4AC.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 184
Cyc_Port_ctype2string(deep,_tmp1E)),((_tmp4AD[0]=& _tmp4AC,((_tmp4AD[1]=& _tmp4AB,((_tmp4AD[2]=& _tmp4AA,Cyc_aprintf(((_tmp4AE="array(%s,%s,%s)",_tag_dyneither(_tmp4AE,sizeof(char),16))),_tag_dyneither(_tmp4AD,sizeof(void*),3))))))))))))))))))));}_LL1B: {struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmp21=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpA;if(_tmp21->tag != 13)goto _LL1D;else{_tmp22=(_tmp21->f1)->f1;_tmp23=(_tmp21->f1)->f2;}}_LL1C: {
# 187
const char*_tmp4B0;const char*_tmp4AF;struct _dyneither_ptr s=_tmp22->kind == Cyc_Absyn_StructA?(_tmp4B0="struct",_tag_dyneither(_tmp4B0,sizeof(char),7)):((_tmp4AF="union",_tag_dyneither(_tmp4AF,sizeof(char),6)));
if(!deep){
const char*_tmp4B5;void*_tmp4B4[2];struct Cyc_String_pa_PrintArg_struct _tmp4B3;struct Cyc_String_pa_PrintArg_struct _tmp4B2;return(struct _dyneither_ptr)((_tmp4B2.tag=0,((_tmp4B2.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp22->name)),((_tmp4B3.tag=0,((_tmp4B3.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s),((_tmp4B4[0]=& _tmp4B3,((_tmp4B4[1]=& _tmp4B2,Cyc_aprintf(((_tmp4B5="%s %s",_tag_dyneither(_tmp4B5,sizeof(char),6))),_tag_dyneither(_tmp4B4,sizeof(void*),2))))))))))))));}else{
# 191
const char*_tmp4BB;void*_tmp4BA[3];struct Cyc_String_pa_PrintArg_struct _tmp4B9;struct Cyc_String_pa_PrintArg_struct _tmp4B8;struct Cyc_String_pa_PrintArg_struct _tmp4B7;return(struct _dyneither_ptr)((_tmp4B7.tag=0,((_tmp4B7.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Port_cfields2string(0,_tmp23)),((_tmp4B8.tag=0,((_tmp4B8.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 191
Cyc_Absynpp_qvar2string(_tmp22->name)),((_tmp4B9.tag=0,((_tmp4B9.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s),((_tmp4BA[0]=& _tmp4B9,((_tmp4BA[1]=& _tmp4B8,((_tmp4BA[2]=& _tmp4B7,Cyc_aprintf(((_tmp4BB="%s %s {%s}",_tag_dyneither(_tmp4BB,sizeof(char),11))),_tag_dyneither(_tmp4BA,sizeof(void*),3))))))))))))))))))));}}_LL1D: {struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmp24=(struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpA;if(_tmp24->tag != 14)goto _LL1F;else{_tmp25=_tmp24->f1;if(_tmp24->f2 == 0)goto _LL1F;_tmp26=*_tmp24->f2;}}_LL1E:
# 193
 return Cyc_Port_ctype2string(deep,_tmp26);_LL1F: {struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmp27=(struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpA;if(_tmp27->tag != 14)goto _LL21;else{_tmp28=_tmp27->f1;}}_LL20: {
# 195
const char*_tmp4BF;void*_tmp4BE[1];struct Cyc_String_pa_PrintArg_struct _tmp4BD;return(struct _dyneither_ptr)((_tmp4BD.tag=0,((_tmp4BD.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_cfields2string(deep,_tmp28)),((_tmp4BE[0]=& _tmp4BD,Cyc_aprintf(((_tmp4BF="aggr {%s}",_tag_dyneither(_tmp4BF,sizeof(char),10))),_tag_dyneither(_tmp4BE,sizeof(void*),1))))))));}_LL21: {struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp29=(struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpA;if(_tmp29->tag != 15)goto _LL23;else{_tmp2A=(void*)_tmp29->f1;_tmp2B=_tmp29->f2;}}_LL22: {
# 197
const char*_tmp4C4;void*_tmp4C3[2];struct Cyc_String_pa_PrintArg_struct _tmp4C2;struct Cyc_String_pa_PrintArg_struct _tmp4C1;return(struct _dyneither_ptr)((_tmp4C1.tag=0,((_tmp4C1.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,_tmp2A)),((_tmp4C2.tag=0,((_tmp4C2.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctypes2string(deep,_tmp2B)),((_tmp4C3[0]=& _tmp4C2,((_tmp4C3[1]=& _tmp4C1,Cyc_aprintf(((_tmp4C4="fn(%s)->%s",_tag_dyneither(_tmp4C4,sizeof(char),11))),_tag_dyneither(_tmp4C3,sizeof(void*),2))))))))))))));}_LL23: {struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmp2C=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmpA;if(_tmp2C->tag != 16)goto _LL0;else{_tmp2D=_tmp2C->f1;}}_LL24:
# 199
 if((unsigned int)_tmp2D->eq)
return Cyc_Port_ctype2string(deep,*((void**)_check_null(_tmp2D->eq)));else{
if(!deep  || _tmp2D->uppers == 0  && _tmp2D->lowers == 0){
const char*_tmp4C8;void*_tmp4C7[1];struct Cyc_Int_pa_PrintArg_struct _tmp4C6;return(struct _dyneither_ptr)((_tmp4C6.tag=1,((_tmp4C6.f1=(unsigned long)_tmp2D->id,((_tmp4C7[0]=& _tmp4C6,Cyc_aprintf(((_tmp4C8="var(%d)",_tag_dyneither(_tmp4C8,sizeof(char),8))),_tag_dyneither(_tmp4C7,sizeof(void*),1))))))));}else{
const char*_tmp4CE;void*_tmp4CD[3];struct Cyc_String_pa_PrintArg_struct _tmp4CC;struct Cyc_Int_pa_PrintArg_struct _tmp4CB;struct Cyc_String_pa_PrintArg_struct _tmp4CA;return(struct _dyneither_ptr)((_tmp4CA.tag=0,((_tmp4CA.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 206
Cyc_Port_ctypes2string(0,_tmp2D->uppers)),((_tmp4CB.tag=1,((_tmp4CB.f1=(unsigned long)_tmp2D->id,((_tmp4CC.tag=0,((_tmp4CC.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
# 204
Cyc_Port_ctypes2string(0,_tmp2D->lowers)),((_tmp4CD[0]=& _tmp4CC,((_tmp4CD[1]=& _tmp4CB,((_tmp4CD[2]=& _tmp4CA,Cyc_aprintf(((_tmp4CE="var([%s]<=%d<=[%s])",_tag_dyneither(_tmp4CE,sizeof(char),20))),_tag_dyneither(_tmp4CD,sizeof(void*),3))))))))))))))))))));}}_LL0:;}
# 209
static struct _dyneither_ptr*Cyc_Port_ctype2stringptr(int deep,void*t){struct _dyneither_ptr*_tmp4CF;return(_tmp4CF=_cycalloc(sizeof(*_tmp4CF)),((_tmp4CF[0]=Cyc_Port_ctype2string(deep,t),_tmp4CF)));}
struct Cyc_List_List*Cyc_Port_sep(struct _dyneither_ptr s,struct Cyc_List_List*xs){
struct _dyneither_ptr*_tmp4D0;struct _dyneither_ptr*_tmp62=(_tmp4D0=_cycalloc(sizeof(*_tmp4D0)),((_tmp4D0[0]=s,_tmp4D0)));
if(xs == 0)return xs;{
struct Cyc_List_List*_tmp63=xs;
struct Cyc_List_List*_tmp64=xs->tl;
for(0;_tmp64 != 0;(_tmp63=_tmp64,_tmp64=_tmp64->tl)){
struct Cyc_List_List*_tmp4D1;((struct Cyc_List_List*)_check_null(_tmp63))->tl=((_tmp4D1=_cycalloc(sizeof(*_tmp4D1)),((_tmp4D1->hd=_tmp62,((_tmp4D1->tl=_tmp64,_tmp4D1))))));}
# 218
return xs;};}struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;
# 220
static struct _dyneither_ptr*Cyc_Port_cfield2stringptr(int deep,struct Cyc_Port_Cfield*f){
const char*_tmp4D7;void*_tmp4D6[3];struct Cyc_String_pa_PrintArg_struct _tmp4D5;struct Cyc_String_pa_PrintArg_struct _tmp4D4;struct Cyc_String_pa_PrintArg_struct _tmp4D3;struct _dyneither_ptr s=(struct _dyneither_ptr)(
(_tmp4D3.tag=0,((_tmp4D3.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,f->type)),((_tmp4D4.tag=0,((_tmp4D4.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f->f),((_tmp4D5.tag=0,((_tmp4D5.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Port_ctype2string(deep,f->qual)),((_tmp4D6[0]=& _tmp4D5,((_tmp4D6[1]=& _tmp4D4,((_tmp4D6[2]=& _tmp4D3,Cyc_aprintf(((_tmp4D7="%s %s: %s",_tag_dyneither(_tmp4D7,sizeof(char),10))),_tag_dyneither(_tmp4D6,sizeof(void*),3))))))))))))))))))));
struct _dyneither_ptr*_tmp4D8;return(_tmp4D8=_cycalloc(sizeof(*_tmp4D8)),((_tmp4D8[0]=s,_tmp4D8)));}
# 225
static struct _dyneither_ptr Cyc_Port_ctypes2string(int deep,struct Cyc_List_List*ts){
const char*_tmp4D9;return(struct _dyneither_ptr)Cyc_strconcat_l(Cyc_Port_sep(((_tmp4D9=",",_tag_dyneither(_tmp4D9,sizeof(char),2))),((struct Cyc_List_List*(*)(struct _dyneither_ptr*(*f)(int,void*),int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_ctype2stringptr,deep,ts)));}struct Cyc_Port_Cfield;
# 228
static struct _dyneither_ptr Cyc_Port_cfields2string(int deep,struct Cyc_List_List*fs){
const char*_tmp4DA;return(struct _dyneither_ptr)Cyc_strconcat_l(Cyc_Port_sep(((_tmp4DA=";",_tag_dyneither(_tmp4DA,sizeof(char),2))),((struct Cyc_List_List*(*)(struct _dyneither_ptr*(*f)(int,struct Cyc_Port_Cfield*),int env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_cfield2stringptr,deep,fs)));}
# 234
static void*Cyc_Port_notconst_ct(){return(void*)& Cyc_Port_Notconst_ct_val;}
static void*Cyc_Port_const_ct(){return(void*)& Cyc_Port_Const_ct_val;}
static void*Cyc_Port_thin_ct(){return(void*)& Cyc_Port_Thin_ct_val;}
static void*Cyc_Port_fat_ct(){return(void*)& Cyc_Port_Fat_ct_val;}
static void*Cyc_Port_void_ct(){return(void*)& Cyc_Port_Void_ct_val;}
static void*Cyc_Port_zero_ct(){return(void*)& Cyc_Port_Zero_ct_val;}
static void*Cyc_Port_arith_ct(){return(void*)& Cyc_Port_Arith_ct_val;}
static void*Cyc_Port_heap_ct(){return(void*)& Cyc_Port_Heap_ct_val;}
static void*Cyc_Port_zterm_ct(){return(void*)& Cyc_Port_Zterm_ct_val;}
static void*Cyc_Port_nozterm_ct(){return(void*)& Cyc_Port_Nozterm_ct_val;}
static void*Cyc_Port_rgnvar_ct(struct _dyneither_ptr*n){struct Cyc_Port_RgnVar_ct_Port_Ctype_struct _tmp4DD;struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmp4DC;return(void*)((_tmp4DC=_cycalloc(sizeof(*_tmp4DC)),((_tmp4DC[0]=((_tmp4DD.tag=10,((_tmp4DD.f1=n,_tmp4DD)))),_tmp4DC))));}
static void*Cyc_Port_unknown_aggr_ct(struct Cyc_List_List*fs){
struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct _tmp4E0;struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmp4DF;return(void*)((_tmp4DF=_cycalloc(sizeof(*_tmp4DF)),((_tmp4DF[0]=((_tmp4E0.tag=14,((_tmp4E0.f1=fs,((_tmp4E0.f2=0,_tmp4E0)))))),_tmp4DF))));}
# 248
static void*Cyc_Port_known_aggr_ct(struct _tuple11*p){
struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct _tmp4E3;struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmp4E2;return(void*)((_tmp4E2=_cycalloc(sizeof(*_tmp4E2)),((_tmp4E2[0]=((_tmp4E3.tag=13,((_tmp4E3.f1=p,_tmp4E3)))),_tmp4E2))));}
# 251
static void*Cyc_Port_ptr_ct(void*elt,void*qual,void*ptr_kind,void*r,void*zt){
# 253
struct Cyc_Port_Ptr_ct_Port_Ctype_struct _tmp4E6;struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp4E5;return(void*)((_tmp4E5=_cycalloc(sizeof(*_tmp4E5)),((_tmp4E5[0]=((_tmp4E6.tag=11,((_tmp4E6.f1=elt,((_tmp4E6.f2=qual,((_tmp4E6.f3=ptr_kind,((_tmp4E6.f4=r,((_tmp4E6.f5=zt,_tmp4E6)))))))))))),_tmp4E5))));}
# 255
static void*Cyc_Port_array_ct(void*elt,void*qual,void*zt){
struct Cyc_Port_Array_ct_Port_Ctype_struct _tmp4E9;struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp4E8;return(void*)((_tmp4E8=_cycalloc(sizeof(*_tmp4E8)),((_tmp4E8[0]=((_tmp4E9.tag=12,((_tmp4E9.f1=elt,((_tmp4E9.f2=qual,((_tmp4E9.f3=zt,_tmp4E9)))))))),_tmp4E8))));}
# 258
static void*Cyc_Port_fn_ct(void*return_type,struct Cyc_List_List*args){
struct Cyc_Port_Fn_ct_Port_Ctype_struct _tmp4EC;struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp4EB;return(void*)((_tmp4EB=_cycalloc(sizeof(*_tmp4EB)),((_tmp4EB[0]=((_tmp4EC.tag=15,((_tmp4EC.f1=return_type,((_tmp4EC.f2=args,_tmp4EC)))))),_tmp4EB))));}struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;
# 261
static void*Cyc_Port_var(){
static int counter=0;
struct Cyc_Port_Var_ct_Port_Ctype_struct _tmp4F2;struct Cyc_Port_Cvar*_tmp4F1;struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmp4F0;return(void*)((_tmp4F0=_cycalloc(sizeof(*_tmp4F0)),((_tmp4F0[0]=((_tmp4F2.tag=16,((_tmp4F2.f1=((_tmp4F1=_cycalloc(sizeof(*_tmp4F1)),((_tmp4F1->id=counter ++,((_tmp4F1->eq=0,((_tmp4F1->uppers=0,((_tmp4F1->lowers=0,_tmp4F1)))))))))),_tmp4F2)))),_tmp4F0))));}
# 265
static void*Cyc_Port_new_var(void*x){
return Cyc_Port_var();}
# 268
static struct _dyneither_ptr*Cyc_Port_new_region_var(){
static int counter=0;
const char*_tmp4F6;void*_tmp4F5[1];struct Cyc_Int_pa_PrintArg_struct _tmp4F4;struct _dyneither_ptr s=(struct _dyneither_ptr)((_tmp4F4.tag=1,((_tmp4F4.f1=(unsigned long)counter ++,((_tmp4F5[0]=& _tmp4F4,Cyc_aprintf(((_tmp4F6="`R%d",_tag_dyneither(_tmp4F6,sizeof(char),5))),_tag_dyneither(_tmp4F5,sizeof(void*),1))))))));
struct _dyneither_ptr*_tmp4F7;return(_tmp4F7=_cycalloc(sizeof(*_tmp4F7)),((_tmp4F7[0]=s,_tmp4F7)));}
# 276
static int Cyc_Port_unifies(void*t1,void*t2);struct Cyc_Port_Cvar;
# 278
static void*Cyc_Port_compress_ct(void*t){
void*_tmp82=t;void***_tmp84;struct Cyc_List_List*_tmp85;struct Cyc_List_List*_tmp86;void**_tmp88;_LL26: {struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmp83=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp82;if(_tmp83->tag != 16)goto _LL28;else{_tmp84=(void***)&(_tmp83->f1)->eq;_tmp85=(_tmp83->f1)->uppers;_tmp86=(_tmp83->f1)->lowers;}}_LL27: {
# 281
void**_tmp89=*_tmp84;
if((unsigned int)_tmp89){
# 285
void*r=Cyc_Port_compress_ct(*_tmp89);
if(*_tmp89 != r){void**_tmp4F8;*_tmp84=((_tmp4F8=_cycalloc(sizeof(*_tmp4F8)),((_tmp4F8[0]=r,_tmp4F8))));}
return r;}
# 292
for(0;_tmp86 != 0;_tmp86=_tmp86->tl){
void*_tmp8B=(void*)_tmp86->hd;_LL2D: {struct Cyc_Port_Const_ct_Port_Ctype_struct*_tmp8C=(struct Cyc_Port_Const_ct_Port_Ctype_struct*)_tmp8B;if(_tmp8C->tag != 0)goto _LL2F;}_LL2E:
 goto _LL30;_LL2F: {struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*_tmp8D=(struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*)_tmp8B;if(_tmp8D->tag != 9)goto _LL31;}_LL30:
 goto _LL32;_LL31: {struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmp8E=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmp8B;if(_tmp8E->tag != 4)goto _LL33;}_LL32:
 goto _LL34;_LL33: {struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmp8F=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmp8B;if(_tmp8F->tag != 13)goto _LL35;}_LL34:
 goto _LL36;_LL35: {struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp90=(struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp8B;if(_tmp90->tag != 15)goto _LL37;}_LL36:
# 299
{void**_tmp4F9;*_tmp84=((_tmp4F9=_cycalloc(sizeof(*_tmp4F9)),((_tmp4F9[0]=(void*)_tmp86->hd,_tmp4F9))));}
return(void*)_tmp86->hd;_LL37:;_LL38:
# 302
 goto _LL2C;_LL2C:;}
# 305
for(0;_tmp85 != 0;_tmp85=_tmp85->tl){
void*_tmp92=(void*)_tmp85->hd;_LL3A: {struct Cyc_Port_Notconst_ct_Port_Ctype_struct*_tmp93=(struct Cyc_Port_Notconst_ct_Port_Ctype_struct*)_tmp92;if(_tmp93->tag != 1)goto _LL3C;}_LL3B:
 goto _LL3D;_LL3C: {struct Cyc_Port_Zterm_ct_Port_Ctype_struct*_tmp94=(struct Cyc_Port_Zterm_ct_Port_Ctype_struct*)_tmp92;if(_tmp94->tag != 8)goto _LL3E;}_LL3D:
 goto _LL3F;_LL3E: {struct Cyc_Port_Zero_ct_Port_Ctype_struct*_tmp95=(struct Cyc_Port_Zero_ct_Port_Ctype_struct*)_tmp92;if(_tmp95->tag != 5)goto _LL40;}_LL3F:
 goto _LL41;_LL40: {struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmp96=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmp92;if(_tmp96->tag != 13)goto _LL42;}_LL41:
 goto _LL43;_LL42: {struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp97=(struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp92;if(_tmp97->tag != 15)goto _LL44;}_LL43:
# 312
{void**_tmp4FA;*_tmp84=((_tmp4FA=_cycalloc(sizeof(*_tmp4FA)),((_tmp4FA[0]=(void*)_tmp85->hd,_tmp4FA))));}
return(void*)_tmp85->hd;_LL44:;_LL45:
# 315
 goto _LL39;_LL39:;}
# 318
return t;}_LL28: {struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmp87=(struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmp82;if(_tmp87->tag != 14)goto _LL2A;else{_tmp88=_tmp87->f2;}}_LL29:
# 321
 if((unsigned int)_tmp88)return Cyc_Port_compress_ct(*_tmp88);else{
return t;}_LL2A:;_LL2B:
# 325
 return t;_LL25:;}
# 331
static void*Cyc_Port_inst(struct Cyc_Dict_Dict*instenv,void*t){
void*_tmp99=Cyc_Port_compress_ct(t);struct _dyneither_ptr*_tmpA8;void*_tmpAA;void*_tmpAB;void*_tmpAC;void*_tmpAD;void*_tmpAE;void*_tmpB0;void*_tmpB1;void*_tmpB2;void*_tmpB4;struct Cyc_List_List*_tmpB5;_LL47: {struct Cyc_Port_Const_ct_Port_Ctype_struct*_tmp9A=(struct Cyc_Port_Const_ct_Port_Ctype_struct*)_tmp99;if(_tmp9A->tag != 0)goto _LL49;}_LL48:
 goto _LL4A;_LL49: {struct Cyc_Port_Notconst_ct_Port_Ctype_struct*_tmp9B=(struct Cyc_Port_Notconst_ct_Port_Ctype_struct*)_tmp99;if(_tmp9B->tag != 1)goto _LL4B;}_LL4A:
 goto _LL4C;_LL4B: {struct Cyc_Port_Thin_ct_Port_Ctype_struct*_tmp9C=(struct Cyc_Port_Thin_ct_Port_Ctype_struct*)_tmp99;if(_tmp9C->tag != 2)goto _LL4D;}_LL4C:
 goto _LL4E;_LL4D: {struct Cyc_Port_Fat_ct_Port_Ctype_struct*_tmp9D=(struct Cyc_Port_Fat_ct_Port_Ctype_struct*)_tmp99;if(_tmp9D->tag != 3)goto _LL4F;}_LL4E:
 goto _LL50;_LL4F: {struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmp9E=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmp99;if(_tmp9E->tag != 4)goto _LL51;}_LL50:
 goto _LL52;_LL51: {struct Cyc_Port_Zero_ct_Port_Ctype_struct*_tmp9F=(struct Cyc_Port_Zero_ct_Port_Ctype_struct*)_tmp99;if(_tmp9F->tag != 5)goto _LL53;}_LL52:
 goto _LL54;_LL53: {struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmpA0=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmp99;if(_tmpA0->tag != 6)goto _LL55;}_LL54:
 goto _LL56;_LL55: {struct Cyc_Port_Zterm_ct_Port_Ctype_struct*_tmpA1=(struct Cyc_Port_Zterm_ct_Port_Ctype_struct*)_tmp99;if(_tmpA1->tag != 8)goto _LL57;}_LL56:
 goto _LL58;_LL57: {struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*_tmpA2=(struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*)_tmp99;if(_tmpA2->tag != 9)goto _LL59;}_LL58:
 goto _LL5A;_LL59: {struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmpA3=(struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmp99;if(_tmpA3->tag != 14)goto _LL5B;}_LL5A:
 goto _LL5C;_LL5B: {struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmpA4=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmp99;if(_tmpA4->tag != 13)goto _LL5D;}_LL5C:
 goto _LL5E;_LL5D: {struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmpA5=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp99;if(_tmpA5->tag != 16)goto _LL5F;}_LL5E:
 goto _LL60;_LL5F: {struct Cyc_Port_Heap_ct_Port_Ctype_struct*_tmpA6=(struct Cyc_Port_Heap_ct_Port_Ctype_struct*)_tmp99;if(_tmpA6->tag != 7)goto _LL61;}_LL60:
 return t;_LL61: {struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmpA7=(struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp99;if(_tmpA7->tag != 10)goto _LL63;else{_tmpA8=_tmpA7->f1;}}_LL62:
# 347
 if(!((int(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_member)(*instenv,_tmpA8))
*instenv=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k,void*v))Cyc_Dict_insert)(*instenv,_tmpA8,Cyc_Port_var());
return((void*(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_lookup)(*instenv,_tmpA8);_LL63: {struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmpA9=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp99;if(_tmpA9->tag != 11)goto _LL65;else{_tmpAA=(void*)_tmpA9->f1;_tmpAB=(void*)_tmpA9->f2;_tmpAC=(void*)_tmpA9->f3;_tmpAD=(void*)_tmpA9->f4;_tmpAE=(void*)_tmpA9->f5;}}_LL64: {
# 351
void*_tmpB8;void*_tmpB9;struct _tuple10 _tmp4FB;struct _tuple10 _tmpB7=(_tmp4FB.f1=Cyc_Port_inst(instenv,_tmpAA),((_tmp4FB.f2=Cyc_Port_inst(instenv,_tmpAD),_tmp4FB)));_tmpB8=_tmpB7.f1;_tmpB9=_tmpB7.f2;
if(_tmpB8 == _tmpAA  && _tmpB9 == _tmpAD)return t;{
struct Cyc_Port_Ptr_ct_Port_Ctype_struct _tmp4FE;struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp4FD;return(void*)((_tmp4FD=_cycalloc(sizeof(*_tmp4FD)),((_tmp4FD[0]=((_tmp4FE.tag=11,((_tmp4FE.f1=_tmpB8,((_tmp4FE.f2=_tmpAB,((_tmp4FE.f3=_tmpAC,((_tmp4FE.f4=_tmpB9,((_tmp4FE.f5=_tmpAE,_tmp4FE)))))))))))),_tmp4FD))));};}_LL65: {struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmpAF=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp99;if(_tmpAF->tag != 12)goto _LL67;else{_tmpB0=(void*)_tmpAF->f1;_tmpB1=(void*)_tmpAF->f2;_tmpB2=(void*)_tmpAF->f3;}}_LL66: {
# 355
void*_tmpBC=Cyc_Port_inst(instenv,_tmpB0);
if(_tmpBC == _tmpB0)return t;{
struct Cyc_Port_Array_ct_Port_Ctype_struct _tmp501;struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp500;return(void*)((_tmp500=_cycalloc(sizeof(*_tmp500)),((_tmp500[0]=((_tmp501.tag=12,((_tmp501.f1=_tmpBC,((_tmp501.f2=_tmpB1,((_tmp501.f3=_tmpB2,_tmp501)))))))),_tmp500))));};}_LL67: {struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmpB3=(struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp99;if(_tmpB3->tag != 15)goto _LL46;else{_tmpB4=(void*)_tmpB3->f1;_tmpB5=_tmpB3->f2;}}_LL68: {
# 359
struct Cyc_Port_Fn_ct_Port_Ctype_struct _tmp504;struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp503;return(void*)((_tmp503=_cycalloc(sizeof(*_tmp503)),((_tmp503[0]=((_tmp504.tag=15,((_tmp504.f1=Cyc_Port_inst(instenv,_tmpB4),((_tmp504.f2=((struct Cyc_List_List*(*)(void*(*f)(struct Cyc_Dict_Dict*,void*),struct Cyc_Dict_Dict*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_inst,instenv,_tmpB5),_tmp504)))))),_tmp503))));}_LL46:;}
# 363
void*Cyc_Port_instantiate(void*t){
struct Cyc_Dict_Dict*_tmp505;return Cyc_Port_inst(((_tmp505=_cycalloc(sizeof(*_tmp505)),((_tmp505[0]=((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Dict_empty)(Cyc_strptrcmp),_tmp505)))),t);}
# 370
struct Cyc_List_List*Cyc_Port_filter_self(void*t,struct Cyc_List_List*ts){
int found=0;
{struct Cyc_List_List*_tmpC2=ts;for(0;(unsigned int)_tmpC2;_tmpC2=_tmpC2->tl){
void*_tmpC3=Cyc_Port_compress_ct((void*)_tmpC2->hd);
if(t == _tmpC3)found=1;}}
# 376
if(!found)return ts;{
struct Cyc_List_List*res=0;
for(0;ts != 0;ts=ts->tl){
if(t == Cyc_Port_compress_ct((void*)ts->hd))continue;{
struct Cyc_List_List*_tmp506;res=((_tmp506=_cycalloc(sizeof(*_tmp506)),((_tmp506->hd=(void*)ts->hd,((_tmp506->tl=res,_tmp506))))));};}
# 382
return res;};}struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;
# 387
void Cyc_Port_generalize(int is_rgn,void*t){
t=Cyc_Port_compress_ct(t);{
void*_tmpC5=t;struct Cyc_Port_Cvar*_tmpC7;void*_tmpD6;void*_tmpD7;void*_tmpD8;void*_tmpD9;void*_tmpDA;void*_tmpDC;void*_tmpDD;void*_tmpDE;void*_tmpE0;struct Cyc_List_List*_tmpE1;_LL6A: {struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmpC6=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmpC5;if(_tmpC6->tag != 16)goto _LL6C;else{_tmpC7=_tmpC6->f1;}}_LL6B:
# 392
 _tmpC7->uppers=Cyc_Port_filter_self(t,_tmpC7->uppers);
_tmpC7->lowers=Cyc_Port_filter_self(t,_tmpC7->lowers);
if(is_rgn){
# 397
if(_tmpC7->uppers == 0  && _tmpC7->lowers == 0){
Cyc_Port_unifies(t,Cyc_Port_rgnvar_ct(Cyc_Port_new_region_var()));
return;}
# 402
if((unsigned int)_tmpC7->uppers){
Cyc_Port_unifies(t,(void*)((struct Cyc_List_List*)_check_null(_tmpC7->uppers))->hd);
Cyc_Port_generalize(1,t);}else{
# 406
Cyc_Port_unifies(t,(void*)((struct Cyc_List_List*)_check_null(_tmpC7->lowers))->hd);
Cyc_Port_generalize(1,t);}}
# 410
return;_LL6C: {struct Cyc_Port_Const_ct_Port_Ctype_struct*_tmpC8=(struct Cyc_Port_Const_ct_Port_Ctype_struct*)_tmpC5;if(_tmpC8->tag != 0)goto _LL6E;}_LL6D:
 goto _LL6F;_LL6E: {struct Cyc_Port_Notconst_ct_Port_Ctype_struct*_tmpC9=(struct Cyc_Port_Notconst_ct_Port_Ctype_struct*)_tmpC5;if(_tmpC9->tag != 1)goto _LL70;}_LL6F:
 goto _LL71;_LL70: {struct Cyc_Port_Thin_ct_Port_Ctype_struct*_tmpCA=(struct Cyc_Port_Thin_ct_Port_Ctype_struct*)_tmpC5;if(_tmpCA->tag != 2)goto _LL72;}_LL71:
 goto _LL73;_LL72: {struct Cyc_Port_Fat_ct_Port_Ctype_struct*_tmpCB=(struct Cyc_Port_Fat_ct_Port_Ctype_struct*)_tmpC5;if(_tmpCB->tag != 3)goto _LL74;}_LL73:
 goto _LL75;_LL74: {struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmpCC=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmpC5;if(_tmpCC->tag != 4)goto _LL76;}_LL75:
 goto _LL77;_LL76: {struct Cyc_Port_Zero_ct_Port_Ctype_struct*_tmpCD=(struct Cyc_Port_Zero_ct_Port_Ctype_struct*)_tmpC5;if(_tmpCD->tag != 5)goto _LL78;}_LL77:
 goto _LL79;_LL78: {struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmpCE=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmpC5;if(_tmpCE->tag != 6)goto _LL7A;}_LL79:
 goto _LL7B;_LL7A: {struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmpCF=(struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpC5;if(_tmpCF->tag != 14)goto _LL7C;}_LL7B:
 goto _LL7D;_LL7C: {struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmpD0=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpC5;if(_tmpD0->tag != 13)goto _LL7E;}_LL7D:
 goto _LL7F;_LL7E: {struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmpD1=(struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmpC5;if(_tmpD1->tag != 10)goto _LL80;}_LL7F:
 goto _LL81;_LL80: {struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*_tmpD2=(struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*)_tmpC5;if(_tmpD2->tag != 9)goto _LL82;}_LL81:
 goto _LL83;_LL82: {struct Cyc_Port_Zterm_ct_Port_Ctype_struct*_tmpD3=(struct Cyc_Port_Zterm_ct_Port_Ctype_struct*)_tmpC5;if(_tmpD3->tag != 8)goto _LL84;}_LL83:
 goto _LL85;_LL84: {struct Cyc_Port_Heap_ct_Port_Ctype_struct*_tmpD4=(struct Cyc_Port_Heap_ct_Port_Ctype_struct*)_tmpC5;if(_tmpD4->tag != 7)goto _LL86;}_LL85:
 return;_LL86: {struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmpD5=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpC5;if(_tmpD5->tag != 11)goto _LL88;else{_tmpD6=(void*)_tmpD5->f1;_tmpD7=(void*)_tmpD5->f2;_tmpD8=(void*)_tmpD5->f3;_tmpD9=(void*)_tmpD5->f4;_tmpDA=(void*)_tmpD5->f5;}}_LL87:
# 427
 Cyc_Port_generalize(0,_tmpD6);Cyc_Port_generalize(1,_tmpD9);goto _LL69;_LL88: {struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmpDB=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpC5;if(_tmpDB->tag != 12)goto _LL8A;else{_tmpDC=(void*)_tmpDB->f1;_tmpDD=(void*)_tmpDB->f2;_tmpDE=(void*)_tmpDB->f3;}}_LL89:
# 429
 Cyc_Port_generalize(0,_tmpDC);Cyc_Port_generalize(0,_tmpDD);goto _LL69;_LL8A: {struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmpDF=(struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpC5;if(_tmpDF->tag != 15)goto _LL69;else{_tmpE0=(void*)_tmpDF->f1;_tmpE1=_tmpDF->f2;}}_LL8B:
# 431
 Cyc_Port_generalize(0,_tmpE0);((void(*)(void(*f)(int,void*),int env,struct Cyc_List_List*x))Cyc_List_iter_c)(Cyc_Port_generalize,0,_tmpE1);goto _LL69;_LL69:;};}struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;
# 437
static int Cyc_Port_occurs(void*v,void*t){
t=Cyc_Port_compress_ct(t);
if(t == v)return 1;{
void*_tmpE2=t;void*_tmpE4;void*_tmpE5;void*_tmpE6;void*_tmpE7;void*_tmpE8;void*_tmpEA;void*_tmpEB;void*_tmpEC;void*_tmpEE;struct Cyc_List_List*_tmpEF;struct Cyc_List_List*_tmpF1;struct Cyc_List_List*_tmpF3;_LL8D: {struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmpE3=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpE2;if(_tmpE3->tag != 11)goto _LL8F;else{_tmpE4=(void*)_tmpE3->f1;_tmpE5=(void*)_tmpE3->f2;_tmpE6=(void*)_tmpE3->f3;_tmpE7=(void*)_tmpE3->f4;_tmpE8=(void*)_tmpE3->f5;}}_LL8E:
# 442
 return(((Cyc_Port_occurs(v,_tmpE4) || Cyc_Port_occurs(v,_tmpE5)) || Cyc_Port_occurs(v,_tmpE6)) || Cyc_Port_occurs(v,_tmpE7)) || 
Cyc_Port_occurs(v,_tmpE8);_LL8F: {struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmpE9=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpE2;if(_tmpE9->tag != 12)goto _LL91;else{_tmpEA=(void*)_tmpE9->f1;_tmpEB=(void*)_tmpE9->f2;_tmpEC=(void*)_tmpE9->f3;}}_LL90:
# 445
 return(Cyc_Port_occurs(v,_tmpEA) || Cyc_Port_occurs(v,_tmpEB)) || Cyc_Port_occurs(v,_tmpEC);_LL91: {struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmpED=(struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpE2;if(_tmpED->tag != 15)goto _LL93;else{_tmpEE=(void*)_tmpED->f1;_tmpEF=_tmpED->f2;}}_LL92:
# 447
 if(Cyc_Port_occurs(v,_tmpEE))return 1;
for(0;(unsigned int)_tmpEF;_tmpEF=_tmpEF->tl){
if(Cyc_Port_occurs(v,(void*)_tmpEF->hd))return 1;}
return 0;_LL93: {struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmpF0=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpE2;if(_tmpF0->tag != 13)goto _LL95;else{_tmpF1=(_tmpF0->f1)->f2;}}_LL94:
 return 0;_LL95: {struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmpF2=(struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpE2;if(_tmpF2->tag != 14)goto _LL97;else{_tmpF3=_tmpF2->f1;}}_LL96:
# 453
 for(0;(unsigned int)_tmpF3;_tmpF3=_tmpF3->tl){
if(Cyc_Port_occurs(v,((struct Cyc_Port_Cfield*)_tmpF3->hd)->qual) || Cyc_Port_occurs(v,((struct Cyc_Port_Cfield*)_tmpF3->hd)->type))return 1;}
return 0;_LL97:;_LL98:
 return 0;_LL8C:;};}char Cyc_Port_Unify_ct[9]="Unify_ct";struct Cyc_Port_Unify_ct_exn_struct{char*tag;};
# 465
struct Cyc_Port_Unify_ct_exn_struct Cyc_Port_Unify_ct_val={Cyc_Port_Unify_ct};
# 467
static int Cyc_Port_leq(void*t1,void*t2);
static void Cyc_Port_unify_cts(struct Cyc_List_List*t1,struct Cyc_List_List*t2);
static struct Cyc_List_List*Cyc_Port_merge_fields(struct Cyc_List_List*fs1,struct Cyc_List_List*fs2,int allow_subset);struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;
# 471
static void Cyc_Port_unify_ct(void*t1,void*t2){
t1=Cyc_Port_compress_ct(t1);
t2=Cyc_Port_compress_ct(t2);
if(t1 == t2)return;{
struct _tuple10 _tmp507;struct _tuple10 _tmpF6=(_tmp507.f1=t1,((_tmp507.f2=t2,_tmp507)));struct Cyc_Port_Cvar*_tmpF8;struct Cyc_Port_Cvar*_tmpFA;void*_tmpFC;void*_tmpFD;void*_tmpFE;void*_tmpFF;void*_tmp100;void*_tmp102;void*_tmp103;void*_tmp104;void*_tmp105;void*_tmp106;struct _dyneither_ptr _tmp108;struct _dyneither_ptr _tmp10A;void*_tmp10C;void*_tmp10D;void*_tmp10E;void*_tmp110;void*_tmp111;void*_tmp112;void*_tmp114;struct Cyc_List_List*_tmp115;void*_tmp117;struct Cyc_List_List*_tmp118;struct _tuple11*_tmp11A;struct _tuple11*_tmp11C;struct Cyc_List_List*_tmp11E;void***_tmp11F;struct Cyc_List_List*_tmp121;void***_tmp122;struct Cyc_List_List*_tmp124;void***_tmp125;struct Cyc_Absyn_Aggrdecl*_tmp127;struct Cyc_List_List*_tmp128;struct Cyc_Absyn_Aggrdecl*_tmp12A;struct Cyc_List_List*_tmp12B;struct Cyc_List_List*_tmp12D;void***_tmp12E;_LL9A: {struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmpF7=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmpF6.f1;if(_tmpF7->tag != 16)goto _LL9C;else{_tmpF8=_tmpF7->f1;}}_LL9B:
# 477
 if(!Cyc_Port_occurs(t1,t2)){
# 480
{struct Cyc_List_List*_tmp12F=Cyc_Port_filter_self(t1,_tmpF8->uppers);for(0;(unsigned int)_tmp12F;_tmp12F=_tmp12F->tl){
if(!Cyc_Port_leq(t2,(void*)_tmp12F->hd))(int)_throw((void*)& Cyc_Port_Unify_ct_val);}}
{struct Cyc_List_List*_tmp130=Cyc_Port_filter_self(t1,_tmpF8->lowers);for(0;(unsigned int)_tmp130;_tmp130=_tmp130->tl){
if(!Cyc_Port_leq((void*)_tmp130->hd,t2))(int)_throw((void*)& Cyc_Port_Unify_ct_val);}}
{void**_tmp508;_tmpF8->eq=((_tmp508=_cycalloc(sizeof(*_tmp508)),((_tmp508[0]=t2,_tmp508))));}
return;}else{
(int)_throw((void*)& Cyc_Port_Unify_ct_val);}_LL9C: {struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmpF9=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmpF6.f2;if(_tmpF9->tag != 16)goto _LL9E;else{_tmpFA=_tmpF9->f1;}}_LL9D:
 Cyc_Port_unify_ct(t2,t1);return;_LL9E:{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmpFB=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF6.f1;if(_tmpFB->tag != 11)goto _LLA0;else{_tmpFC=(void*)_tmpFB->f1;_tmpFD=(void*)_tmpFB->f2;_tmpFE=(void*)_tmpFB->f3;_tmpFF=(void*)_tmpFB->f4;_tmp100=(void*)_tmpFB->f5;}}{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp101=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmpF6.f2;if(_tmp101->tag != 11)goto _LLA0;else{_tmp102=(void*)_tmp101->f1;_tmp103=(void*)_tmp101->f2;_tmp104=(void*)_tmp101->f3;_tmp105=(void*)_tmp101->f4;_tmp106=(void*)_tmp101->f5;}};_LL9F:
# 489
 Cyc_Port_unify_ct(_tmpFC,_tmp102);Cyc_Port_unify_ct(_tmpFD,_tmp103);Cyc_Port_unify_ct(_tmpFE,_tmp104);Cyc_Port_unify_ct(_tmpFF,_tmp105);
Cyc_Port_unify_ct(_tmp100,_tmp106);
return;_LLA0:{struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmp107=(struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmpF6.f1;if(_tmp107->tag != 10)goto _LLA2;else{_tmp108=*_tmp107->f1;}}{struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmp109=(struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmpF6.f2;if(_tmp109->tag != 10)goto _LLA2;else{_tmp10A=*_tmp109->f1;}};_LLA1:
# 493
 if(Cyc_strcmp((struct _dyneither_ptr)_tmp108,(struct _dyneither_ptr)_tmp10A)!= 0)(int)_throw((void*)& Cyc_Port_Unify_ct_val);
return;_LLA2:{struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp10B=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpF6.f1;if(_tmp10B->tag != 12)goto _LLA4;else{_tmp10C=(void*)_tmp10B->f1;_tmp10D=(void*)_tmp10B->f2;_tmp10E=(void*)_tmp10B->f3;}}{struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp10F=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmpF6.f2;if(_tmp10F->tag != 12)goto _LLA4;else{_tmp110=(void*)_tmp10F->f1;_tmp111=(void*)_tmp10F->f2;_tmp112=(void*)_tmp10F->f3;}};_LLA3:
# 496
 Cyc_Port_unify_ct(_tmp10C,_tmp110);Cyc_Port_unify_ct(_tmp10D,_tmp111);Cyc_Port_unify_ct(_tmp10E,_tmp112);return;_LLA4:{struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp113=(struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpF6.f1;if(_tmp113->tag != 15)goto _LLA6;else{_tmp114=(void*)_tmp113->f1;_tmp115=_tmp113->f2;}}{struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp116=(struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmpF6.f2;if(_tmp116->tag != 15)goto _LLA6;else{_tmp117=(void*)_tmp116->f1;_tmp118=_tmp116->f2;}};_LLA5:
# 498
 Cyc_Port_unify_ct(_tmp114,_tmp117);Cyc_Port_unify_cts(_tmp115,_tmp118);return;_LLA6:{struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmp119=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpF6.f1;if(_tmp119->tag != 13)goto _LLA8;else{_tmp11A=_tmp119->f1;}}{struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmp11B=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpF6.f2;if(_tmp11B->tag != 13)goto _LLA8;else{_tmp11C=_tmp11B->f1;}};_LLA7:
# 500
 if(_tmp11A == _tmp11C)return;else{(int)_throw((void*)& Cyc_Port_Unify_ct_val);}_LLA8:{struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmp11D=(struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpF6.f1;if(_tmp11D->tag != 14)goto _LLAA;else{_tmp11E=_tmp11D->f1;_tmp11F=(void***)& _tmp11D->f2;}}{struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmp120=(struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpF6.f2;if(_tmp120->tag != 14)goto _LLAA;else{_tmp121=_tmp120->f1;_tmp122=(void***)& _tmp120->f2;}};_LLA9: {
# 502
void*_tmp132=Cyc_Port_unknown_aggr_ct(Cyc_Port_merge_fields(_tmp11E,_tmp121,1));
{void**_tmp509;*_tmp11F=(*_tmp122=((_tmp509=_cycalloc(sizeof(*_tmp509)),((_tmp509[0]=_tmp132,_tmp509)))));}
return;}_LLAA:{struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmp123=(struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpF6.f1;if(_tmp123->tag != 14)goto _LLAC;else{_tmp124=_tmp123->f1;_tmp125=(void***)& _tmp123->f2;}}{struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmp126=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpF6.f2;if(_tmp126->tag != 13)goto _LLAC;else{_tmp127=(_tmp126->f1)->f1;_tmp128=(_tmp126->f1)->f2;}};_LLAB:
# 506
 Cyc_Port_merge_fields(_tmp128,_tmp124,0);
{void**_tmp50A;*_tmp125=((_tmp50A=_cycalloc(sizeof(*_tmp50A)),((_tmp50A[0]=t2,_tmp50A))));}
return;_LLAC:{struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmp129=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmpF6.f1;if(_tmp129->tag != 13)goto _LLAE;else{_tmp12A=(_tmp129->f1)->f1;_tmp12B=(_tmp129->f1)->f2;}}{struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmp12C=(struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmpF6.f2;if(_tmp12C->tag != 14)goto _LLAE;else{_tmp12D=_tmp12C->f1;_tmp12E=(void***)& _tmp12C->f2;}};_LLAD:
# 510
 Cyc_Port_merge_fields(_tmp12B,_tmp12D,0);
{void**_tmp50B;*_tmp12E=((_tmp50B=_cycalloc(sizeof(*_tmp50B)),((_tmp50B[0]=t1,_tmp50B))));}
return;_LLAE:;_LLAF:
(int)_throw((void*)& Cyc_Port_Unify_ct_val);_LL99:;};}
# 517
static void Cyc_Port_unify_cts(struct Cyc_List_List*t1,struct Cyc_List_List*t2){
for(0;t1 != 0  && t2 != 0;(t1=t1->tl,t2=t2->tl)){
Cyc_Port_unify_ct((void*)t1->hd,(void*)t2->hd);}
# 521
if(t1 != 0  || t2 != 0)
(int)_throw((void*)& Cyc_Port_Unify_ct_val);}struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;
# 527
static struct Cyc_List_List*Cyc_Port_merge_fields(struct Cyc_List_List*fs1,struct Cyc_List_List*fs2,int allow_f1_subset_f2){
# 529
struct Cyc_List_List*common=0;
{struct Cyc_List_List*_tmp136=fs2;for(0;(unsigned int)_tmp136;_tmp136=_tmp136->tl){
struct Cyc_Port_Cfield*_tmp137=(struct Cyc_Port_Cfield*)_tmp136->hd;
int found=0;
{struct Cyc_List_List*_tmp138=fs1;for(0;(unsigned int)_tmp138;_tmp138=_tmp138->tl){
struct Cyc_Port_Cfield*_tmp139=(struct Cyc_Port_Cfield*)_tmp138->hd;
if(Cyc_strptrcmp(_tmp139->f,_tmp137->f)== 0){
{struct Cyc_List_List*_tmp50C;common=((_tmp50C=_cycalloc(sizeof(*_tmp50C)),((_tmp50C->hd=_tmp139,((_tmp50C->tl=common,_tmp50C))))));}
Cyc_Port_unify_ct(_tmp139->qual,_tmp137->qual);
Cyc_Port_unify_ct(_tmp139->type,_tmp137->type);
found=1;
break;}}}
# 543
if(!found){
if(allow_f1_subset_f2){
struct Cyc_List_List*_tmp50D;common=((_tmp50D=_cycalloc(sizeof(*_tmp50D)),((_tmp50D->hd=_tmp137,((_tmp50D->tl=common,_tmp50D))))));}else{
(int)_throw((void*)& Cyc_Port_Unify_ct_val);}}}}
# 549
{struct Cyc_List_List*_tmp13C=fs1;for(0;(unsigned int)_tmp13C;_tmp13C=_tmp13C->tl){
struct Cyc_Port_Cfield*_tmp13D=(struct Cyc_Port_Cfield*)_tmp13C->hd;
int found=0;
{struct Cyc_List_List*_tmp13E=fs2;for(0;(unsigned int)_tmp13E;_tmp13E=_tmp13E->tl){
struct Cyc_Port_Cfield*_tmp13F=(struct Cyc_Port_Cfield*)_tmp13E->hd;
if(Cyc_strptrcmp(_tmp13D->f,_tmp13F->f))found=1;}}
# 556
if(!found){
struct Cyc_List_List*_tmp50E;common=((_tmp50E=_cycalloc(sizeof(*_tmp50E)),((_tmp50E->hd=_tmp13D,((_tmp50E->tl=common,_tmp50E))))));}}}
# 559
return common;}
# 562
static int Cyc_Port_unifies(void*t1,void*t2){
{struct _handler_cons _tmp141;_push_handler(& _tmp141);{int _tmp143=0;if(setjmp(_tmp141.handler))_tmp143=1;if(!_tmp143){
# 569
Cyc_Port_unify_ct(t1,t2);;_pop_handler();}else{void*_tmp142=(void*)_exn_thrown;void*_tmp145=_tmp142;void*_tmp147;_LLB1: {struct Cyc_Port_Unify_ct_exn_struct*_tmp146=(struct Cyc_Port_Unify_ct_exn_struct*)_tmp145;if(_tmp146->tag != Cyc_Port_Unify_ct)goto _LLB3;}_LLB2:
# 576
 return 0;_LLB3: _tmp147=_tmp145;_LLB4:(void)_throw(_tmp147);_LLB0:;}};}
# 578
return 1;}struct _tuple12{void*f1;void*f2;void*f3;void*f4;void*f5;};
# 584
static struct Cyc_List_List*Cyc_Port_insert_upper(void*v,void*t,struct Cyc_List_List**uppers){
# 586
t=Cyc_Port_compress_ct(t);
{void*_tmp148=t;_LLB6: {struct Cyc_Port_Notconst_ct_Port_Ctype_struct*_tmp149=(struct Cyc_Port_Notconst_ct_Port_Ctype_struct*)_tmp148;if(_tmp149->tag != 1)goto _LLB8;}_LLB7:
# 590
 goto _LLB9;_LLB8: {struct Cyc_Port_Zterm_ct_Port_Ctype_struct*_tmp14A=(struct Cyc_Port_Zterm_ct_Port_Ctype_struct*)_tmp148;if(_tmp14A->tag != 8)goto _LLBA;}_LLB9:
 goto _LLBB;_LLBA: {struct Cyc_Port_Zero_ct_Port_Ctype_struct*_tmp14B=(struct Cyc_Port_Zero_ct_Port_Ctype_struct*)_tmp148;if(_tmp14B->tag != 5)goto _LLBC;}_LLBB:
 goto _LLBD;_LLBC: {struct Cyc_Port_Thin_ct_Port_Ctype_struct*_tmp14C=(struct Cyc_Port_Thin_ct_Port_Ctype_struct*)_tmp148;if(_tmp14C->tag != 2)goto _LLBE;}_LLBD:
 goto _LLBF;_LLBE: {struct Cyc_Port_Fat_ct_Port_Ctype_struct*_tmp14D=(struct Cyc_Port_Fat_ct_Port_Ctype_struct*)_tmp148;if(_tmp14D->tag != 3)goto _LLC0;}_LLBF:
 goto _LLC1;_LLC0: {struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp14E=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp148;if(_tmp14E->tag != 12)goto _LLC2;}_LLC1:
 goto _LLC3;_LLC2: {struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmp14F=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmp148;if(_tmp14F->tag != 13)goto _LLC4;}_LLC3:
 goto _LLC5;_LLC4: {struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp150=(struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp148;if(_tmp150->tag != 15)goto _LLC6;}_LLC5:
 goto _LLC7;_LLC6: {struct Cyc_Port_Heap_ct_Port_Ctype_struct*_tmp151=(struct Cyc_Port_Heap_ct_Port_Ctype_struct*)_tmp148;if(_tmp151->tag != 7)goto _LLC8;}_LLC7:
# 601
*uppers=0;
Cyc_Port_unifies(v,t);
return*uppers;_LLC8: {struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmp152=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmp148;if(_tmp152->tag != 4)goto _LLCA;}_LLC9:
# 606
 goto _LLCB;_LLCA: {struct Cyc_Port_Const_ct_Port_Ctype_struct*_tmp153=(struct Cyc_Port_Const_ct_Port_Ctype_struct*)_tmp148;if(_tmp153->tag != 0)goto _LLCC;}_LLCB:
 goto _LLCD;_LLCC: {struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*_tmp154=(struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*)_tmp148;if(_tmp154->tag != 9)goto _LLCE;}_LLCD:
# 609
 return*uppers;_LLCE:;_LLCF:
 goto _LLB5;_LLB5:;}
# 613
{struct Cyc_List_List*_tmp155=*uppers;for(0;(unsigned int)_tmp155;_tmp155=_tmp155->tl){
void*_tmp156=Cyc_Port_compress_ct((void*)_tmp155->hd);
# 616
if(t == _tmp156)return*uppers;{
struct _tuple10 _tmp50F;struct _tuple10 _tmp158=(_tmp50F.f1=t,((_tmp50F.f2=_tmp156,_tmp50F)));void*_tmp160;void*_tmp161;void*_tmp162;void*_tmp163;void*_tmp164;void*_tmp166;void*_tmp167;void*_tmp168;void*_tmp169;void*_tmp16A;_LLD1:{struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmp159=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmp158.f1;if(_tmp159->tag != 6)goto _LLD3;}{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp15A=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp158.f2;if(_tmp15A->tag != 11)goto _LLD3;};_LLD2:
# 621
 goto _LLD4;_LLD3:{struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmp15B=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmp158.f1;if(_tmp15B->tag != 6)goto _LLD5;}{struct Cyc_Port_Zero_ct_Port_Ctype_struct*_tmp15C=(struct Cyc_Port_Zero_ct_Port_Ctype_struct*)_tmp158.f2;if(_tmp15C->tag != 5)goto _LLD5;};_LLD4:
 goto _LLD6;_LLD5:{struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmp15D=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmp158.f1;if(_tmp15D->tag != 6)goto _LLD7;}{struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp15E=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp158.f2;if(_tmp15E->tag != 12)goto _LLD7;};_LLD6:
# 624
 return*uppers;_LLD7:{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp15F=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp158.f1;if(_tmp15F->tag != 11)goto _LLD9;else{_tmp160=(void*)_tmp15F->f1;_tmp161=(void*)_tmp15F->f2;_tmp162=(void*)_tmp15F->f3;_tmp163=(void*)_tmp15F->f4;_tmp164=(void*)_tmp15F->f5;}}{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp165=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp158.f2;if(_tmp165->tag != 11)goto _LLD9;else{_tmp166=(void*)_tmp165->f1;_tmp167=(void*)_tmp165->f2;_tmp168=(void*)_tmp165->f3;_tmp169=(void*)_tmp165->f4;_tmp16A=(void*)_tmp165->f5;}};_LLD8: {
# 629
void*_tmp16D;void*_tmp16E;void*_tmp16F;void*_tmp170;void*_tmp171;struct _tuple12 _tmp510;struct _tuple12 _tmp16C=(_tmp510.f1=Cyc_Port_var(),((_tmp510.f2=Cyc_Port_var(),((_tmp510.f3=Cyc_Port_var(),((_tmp510.f4=Cyc_Port_var(),((_tmp510.f5=Cyc_Port_var(),_tmp510)))))))));_tmp16D=_tmp16C.f1;_tmp16E=_tmp16C.f2;_tmp16F=_tmp16C.f3;_tmp170=_tmp16C.f4;_tmp171=_tmp16C.f5;{
struct Cyc_Port_Ptr_ct_Port_Ctype_struct _tmp513;struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp512;struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp172=(_tmp512=_cycalloc(sizeof(*_tmp512)),((_tmp512[0]=((_tmp513.tag=11,((_tmp513.f1=_tmp16D,((_tmp513.f2=_tmp16E,((_tmp513.f3=_tmp16F,((_tmp513.f4=_tmp170,((_tmp513.f5=_tmp171,_tmp513)))))))))))),_tmp512)));
Cyc_Port_leq(_tmp16D,_tmp160);Cyc_Port_leq(_tmp16D,_tmp166);
Cyc_Port_leq(_tmp16E,_tmp161);Cyc_Port_leq(_tmp16E,_tmp167);
Cyc_Port_leq(_tmp16F,_tmp162);Cyc_Port_leq(_tmp16F,_tmp167);
Cyc_Port_leq(_tmp170,_tmp163);Cyc_Port_leq(_tmp170,_tmp169);
Cyc_Port_leq(_tmp171,_tmp164);Cyc_Port_leq(_tmp171,_tmp16A);
_tmp155->hd=(void*)((void*)_tmp172);
return*uppers;};}_LLD9:;_LLDA:
 goto _LLD0;_LLD0:;};}}{
# 641
struct Cyc_List_List*_tmp514;return(_tmp514=_cycalloc(sizeof(*_tmp514)),((_tmp514->hd=t,((_tmp514->tl=*uppers,_tmp514)))));};}
# 646
static struct Cyc_List_List*Cyc_Port_insert_lower(void*v,void*t,struct Cyc_List_List**lowers){
# 648
t=Cyc_Port_compress_ct(t);
{void*_tmp176=t;_LLDC: {struct Cyc_Port_Const_ct_Port_Ctype_struct*_tmp177=(struct Cyc_Port_Const_ct_Port_Ctype_struct*)_tmp176;if(_tmp177->tag != 0)goto _LLDE;}_LLDD:
 goto _LLDF;_LLDE: {struct Cyc_Port_Zterm_ct_Port_Ctype_struct*_tmp178=(struct Cyc_Port_Zterm_ct_Port_Ctype_struct*)_tmp176;if(_tmp178->tag != 8)goto _LLE0;}_LLDF:
 goto _LLE1;_LLE0: {struct Cyc_Port_Thin_ct_Port_Ctype_struct*_tmp179=(struct Cyc_Port_Thin_ct_Port_Ctype_struct*)_tmp176;if(_tmp179->tag != 2)goto _LLE2;}_LLE1:
 goto _LLE3;_LLE2: {struct Cyc_Port_Fat_ct_Port_Ctype_struct*_tmp17A=(struct Cyc_Port_Fat_ct_Port_Ctype_struct*)_tmp176;if(_tmp17A->tag != 3)goto _LLE4;}_LLE3:
 goto _LLE5;_LLE4: {struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmp17B=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmp176;if(_tmp17B->tag != 4)goto _LLE6;}_LLE5:
 goto _LLE7;_LLE6: {struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmp17C=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmp176;if(_tmp17C->tag != 13)goto _LLE8;}_LLE7:
 goto _LLE9;_LLE8: {struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp17D=(struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp176;if(_tmp17D->tag != 15)goto _LLEA;}_LLE9:
 goto _LLEB;_LLEA: {struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmp17E=(struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp176;if(_tmp17E->tag != 10)goto _LLEC;}_LLEB:
# 658
*lowers=0;
Cyc_Port_unifies(v,t);
return*lowers;_LLEC: {struct Cyc_Port_Heap_ct_Port_Ctype_struct*_tmp17F=(struct Cyc_Port_Heap_ct_Port_Ctype_struct*)_tmp176;if(_tmp17F->tag != 7)goto _LLEE;}_LLED:
 goto _LLEF;_LLEE: {struct Cyc_Port_Notconst_ct_Port_Ctype_struct*_tmp180=(struct Cyc_Port_Notconst_ct_Port_Ctype_struct*)_tmp176;if(_tmp180->tag != 1)goto _LLF0;}_LLEF:
 goto _LLF1;_LLF0: {struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*_tmp181=(struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*)_tmp176;if(_tmp181->tag != 9)goto _LLF2;}_LLF1:
# 664
 return*lowers;_LLF2:;_LLF3:
# 666
 goto _LLDB;_LLDB:;}
# 668
{struct Cyc_List_List*_tmp182=*lowers;for(0;(unsigned int)_tmp182;_tmp182=_tmp182->tl){
void*_tmp183=Cyc_Port_compress_ct((void*)_tmp182->hd);
if(t == _tmp183)return*lowers;{
struct _tuple10 _tmp515;struct _tuple10 _tmp185=(_tmp515.f1=t,((_tmp515.f2=_tmp183,_tmp515)));void*_tmp190;void*_tmp191;void*_tmp192;void*_tmp193;void*_tmp194;void*_tmp196;void*_tmp197;void*_tmp198;void*_tmp199;void*_tmp19A;_LLF5: {struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmp186=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmp185.f2;if(_tmp186->tag != 4)goto _LLF7;}_LLF6:
 goto _LLF8;_LLF7:{struct Cyc_Port_Zero_ct_Port_Ctype_struct*_tmp187=(struct Cyc_Port_Zero_ct_Port_Ctype_struct*)_tmp185.f1;if(_tmp187->tag != 5)goto _LLF9;}{struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmp188=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmp185.f2;if(_tmp188->tag != 6)goto _LLF9;};_LLF8:
 goto _LLFA;_LLF9:{struct Cyc_Port_Zero_ct_Port_Ctype_struct*_tmp189=(struct Cyc_Port_Zero_ct_Port_Ctype_struct*)_tmp185.f1;if(_tmp189->tag != 5)goto _LLFB;}{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp18A=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp185.f2;if(_tmp18A->tag != 11)goto _LLFB;};_LLFA:
 goto _LLFC;_LLFB:{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp18B=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp185.f1;if(_tmp18B->tag != 11)goto _LLFD;}{struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmp18C=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmp185.f2;if(_tmp18C->tag != 6)goto _LLFD;};_LLFC:
 goto _LLFE;_LLFD:{struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp18D=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp185.f1;if(_tmp18D->tag != 12)goto _LLFF;}{struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmp18E=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmp185.f2;if(_tmp18E->tag != 6)goto _LLFF;};_LLFE:
# 677
 return*lowers;_LLFF:{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp18F=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp185.f1;if(_tmp18F->tag != 11)goto _LL101;else{_tmp190=(void*)_tmp18F->f1;_tmp191=(void*)_tmp18F->f2;_tmp192=(void*)_tmp18F->f3;_tmp193=(void*)_tmp18F->f4;_tmp194=(void*)_tmp18F->f5;}}{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp195=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp185.f2;if(_tmp195->tag != 11)goto _LL101;else{_tmp196=(void*)_tmp195->f1;_tmp197=(void*)_tmp195->f2;_tmp198=(void*)_tmp195->f3;_tmp199=(void*)_tmp195->f4;_tmp19A=(void*)_tmp195->f5;}};_LL100: {
# 682
void*_tmp19D;void*_tmp19E;void*_tmp19F;void*_tmp1A0;void*_tmp1A1;struct _tuple12 _tmp516;struct _tuple12 _tmp19C=(_tmp516.f1=Cyc_Port_var(),((_tmp516.f2=Cyc_Port_var(),((_tmp516.f3=Cyc_Port_var(),((_tmp516.f4=Cyc_Port_var(),((_tmp516.f5=Cyc_Port_var(),_tmp516)))))))));_tmp19D=_tmp19C.f1;_tmp19E=_tmp19C.f2;_tmp19F=_tmp19C.f3;_tmp1A0=_tmp19C.f4;_tmp1A1=_tmp19C.f5;{
struct Cyc_Port_Ptr_ct_Port_Ctype_struct _tmp519;struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp518;struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp1A2=(_tmp518=_cycalloc(sizeof(*_tmp518)),((_tmp518[0]=((_tmp519.tag=11,((_tmp519.f1=_tmp19D,((_tmp519.f2=_tmp19E,((_tmp519.f3=_tmp19F,((_tmp519.f4=_tmp1A0,((_tmp519.f5=_tmp1A1,_tmp519)))))))))))),_tmp518)));
Cyc_Port_leq(_tmp190,_tmp19D);Cyc_Port_leq(_tmp196,_tmp19D);
Cyc_Port_leq(_tmp191,_tmp19E);Cyc_Port_leq(_tmp197,_tmp19E);
Cyc_Port_leq(_tmp192,_tmp19F);Cyc_Port_leq(_tmp197,_tmp19F);
Cyc_Port_leq(_tmp193,_tmp1A0);Cyc_Port_leq(_tmp199,_tmp1A0);
Cyc_Port_leq(_tmp194,_tmp1A1);Cyc_Port_leq(_tmp19A,_tmp1A1);
_tmp182->hd=(void*)((void*)_tmp1A2);
return*lowers;};}_LL101:;_LL102:
 goto _LLF4;_LLF4:;};}}{
# 694
struct Cyc_List_List*_tmp51A;return(_tmp51A=_cycalloc(sizeof(*_tmp51A)),((_tmp51A->hd=t,((_tmp51A->tl=*lowers,_tmp51A)))));};}struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;struct Cyc_Port_Cvar;
# 701
static int Cyc_Port_leq(void*t1,void*t2){
# 707
if(t1 == t2)return 1;
t1=Cyc_Port_compress_ct(t1);
t2=Cyc_Port_compress_ct(t2);{
struct _tuple10 _tmp51B;struct _tuple10 _tmp1A7=(_tmp51B.f1=t1,((_tmp51B.f2=t2,_tmp51B)));struct _dyneither_ptr _tmp1AA;struct _dyneither_ptr _tmp1AC;struct _dyneither_ptr _tmp1AE;void*_tmp1CC;void*_tmp1CD;void*_tmp1CE;void*_tmp1CF;void*_tmp1D0;void*_tmp1D2;void*_tmp1D3;void*_tmp1D4;void*_tmp1D5;void*_tmp1D6;void*_tmp1D8;void*_tmp1D9;void*_tmp1DA;void*_tmp1DC;void*_tmp1DD;void*_tmp1DE;void*_tmp1E0;void*_tmp1E1;void*_tmp1E2;void*_tmp1E4;void*_tmp1E5;void*_tmp1E6;void*_tmp1E7;struct Cyc_Port_Cvar*_tmp1E9;struct Cyc_Port_Cvar*_tmp1EB;struct Cyc_Port_Cvar*_tmp1ED;struct Cyc_Port_Cvar*_tmp1EF;_LL104: {struct Cyc_Port_Heap_ct_Port_Ctype_struct*_tmp1A8=(struct Cyc_Port_Heap_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1A8->tag != 7)goto _LL106;}_LL105:
 return 1;_LL106:{struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmp1A9=(struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1A9->tag != 10)goto _LL108;else{_tmp1AA=*_tmp1A9->f1;}}{struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmp1AB=(struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1AB->tag != 10)goto _LL108;else{_tmp1AC=*_tmp1AB->f1;}};_LL107:
 return Cyc_strcmp((struct _dyneither_ptr)_tmp1AA,(struct _dyneither_ptr)_tmp1AC)== 0;_LL108:{struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmp1AD=(struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1AD->tag != 10)goto _LL10A;else{_tmp1AE=*_tmp1AD->f1;}}{struct Cyc_Port_Heap_ct_Port_Ctype_struct*_tmp1AF=(struct Cyc_Port_Heap_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1AF->tag != 7)goto _LL10A;};_LL109:
 return 0;_LL10A:{struct Cyc_Port_Notconst_ct_Port_Ctype_struct*_tmp1B0=(struct Cyc_Port_Notconst_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1B0->tag != 1)goto _LL10C;}{struct Cyc_Port_Const_ct_Port_Ctype_struct*_tmp1B1=(struct Cyc_Port_Const_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1B1->tag != 0)goto _LL10C;};_LL10B:
 return 1;_LL10C:{struct Cyc_Port_Const_ct_Port_Ctype_struct*_tmp1B2=(struct Cyc_Port_Const_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1B2->tag != 0)goto _LL10E;}{struct Cyc_Port_Notconst_ct_Port_Ctype_struct*_tmp1B3=(struct Cyc_Port_Notconst_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1B3->tag != 1)goto _LL10E;};_LL10D:
 return 0;_LL10E:{struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*_tmp1B4=(struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1B4->tag != 9)goto _LL110;}{struct Cyc_Port_Zterm_ct_Port_Ctype_struct*_tmp1B5=(struct Cyc_Port_Zterm_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1B5->tag != 8)goto _LL110;};_LL10F:
 return 0;_LL110:{struct Cyc_Port_Zterm_ct_Port_Ctype_struct*_tmp1B6=(struct Cyc_Port_Zterm_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1B6->tag != 8)goto _LL112;}{struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*_tmp1B7=(struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1B7->tag != 9)goto _LL112;};_LL111:
 return 1;_LL112:{struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmp1B8=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1B8->tag != 16)goto _LL114;}{struct Cyc_Port_Const_ct_Port_Ctype_struct*_tmp1B9=(struct Cyc_Port_Const_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1B9->tag != 0)goto _LL114;};_LL113:
 return 1;_LL114:{struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmp1BA=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1BA->tag != 16)goto _LL116;}{struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmp1BB=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1BB->tag != 4)goto _LL116;};_LL115:
 return 1;_LL116: {struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmp1BC=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1BC->tag != 4)goto _LL118;}_LL117:
 return 0;_LL118:{struct Cyc_Port_Zero_ct_Port_Ctype_struct*_tmp1BD=(struct Cyc_Port_Zero_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1BD->tag != 5)goto _LL11A;}{struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmp1BE=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1BE->tag != 6)goto _LL11A;};_LL119:
 return 1;_LL11A:{struct Cyc_Port_Zero_ct_Port_Ctype_struct*_tmp1BF=(struct Cyc_Port_Zero_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1BF->tag != 5)goto _LL11C;}{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp1C0=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1C0->tag != 11)goto _LL11C;};_LL11B:
 return 1;_LL11C:{struct Cyc_Port_Zero_ct_Port_Ctype_struct*_tmp1C1=(struct Cyc_Port_Zero_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1C1->tag != 5)goto _LL11E;}{struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmp1C2=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1C2->tag != 4)goto _LL11E;};_LL11D:
 return 1;_LL11E:{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp1C3=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1C3->tag != 11)goto _LL120;}{struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmp1C4=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1C4->tag != 6)goto _LL120;};_LL11F:
 return 1;_LL120:{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp1C5=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1C5->tag != 11)goto _LL122;}{struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmp1C6=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1C6->tag != 4)goto _LL122;};_LL121:
 return 1;_LL122:{struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp1C7=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1C7->tag != 12)goto _LL124;}{struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmp1C8=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1C8->tag != 6)goto _LL124;};_LL123:
 return 1;_LL124:{struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp1C9=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1C9->tag != 12)goto _LL126;}{struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmp1CA=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1CA->tag != 4)goto _LL126;};_LL125:
 return 1;_LL126:{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp1CB=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1CB->tag != 11)goto _LL128;else{_tmp1CC=(void*)_tmp1CB->f1;_tmp1CD=(void*)_tmp1CB->f2;_tmp1CE=(void*)_tmp1CB->f3;_tmp1CF=(void*)_tmp1CB->f4;_tmp1D0=(void*)_tmp1CB->f5;}}{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp1D1=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1D1->tag != 11)goto _LL128;else{_tmp1D2=(void*)_tmp1D1->f1;_tmp1D3=(void*)_tmp1D1->f2;_tmp1D4=(void*)_tmp1D1->f3;_tmp1D5=(void*)_tmp1D1->f4;_tmp1D6=(void*)_tmp1D1->f5;}};_LL127:
# 729
 return(((Cyc_Port_leq(_tmp1CC,_tmp1D2) && Cyc_Port_leq(_tmp1CD,_tmp1D3)) && Cyc_Port_unifies(_tmp1CE,_tmp1D4)) && Cyc_Port_leq(_tmp1CF,_tmp1D5)) && 
Cyc_Port_leq(_tmp1D0,_tmp1D6);_LL128:{struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp1D7=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1D7->tag != 12)goto _LL12A;else{_tmp1D8=(void*)_tmp1D7->f1;_tmp1D9=(void*)_tmp1D7->f2;_tmp1DA=(void*)_tmp1D7->f3;}}{struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp1DB=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1DB->tag != 12)goto _LL12A;else{_tmp1DC=(void*)_tmp1DB->f1;_tmp1DD=(void*)_tmp1DB->f2;_tmp1DE=(void*)_tmp1DB->f3;}};_LL129:
# 732
 return(Cyc_Port_leq(_tmp1D8,_tmp1DC) && Cyc_Port_leq(_tmp1D9,_tmp1DD)) && Cyc_Port_leq(_tmp1DA,_tmp1DE);_LL12A:{struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp1DF=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1DF->tag != 12)goto _LL12C;else{_tmp1E0=(void*)_tmp1DF->f1;_tmp1E1=(void*)_tmp1DF->f2;_tmp1E2=(void*)_tmp1DF->f3;}}{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp1E3=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1E3->tag != 11)goto _LL12C;else{_tmp1E4=(void*)_tmp1E3->f1;_tmp1E5=(void*)_tmp1E3->f2;_tmp1E6=(void*)_tmp1E3->f3;_tmp1E7=(void*)_tmp1E3->f5;}};_LL12B:
# 734
 return((Cyc_Port_leq(_tmp1E0,_tmp1E4) && Cyc_Port_leq(_tmp1E1,_tmp1E5)) && Cyc_Port_unifies((void*)& Cyc_Port_Fat_ct_val,_tmp1E6)) && 
Cyc_Port_leq(_tmp1E2,_tmp1E7);_LL12C:{struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmp1E8=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1E8->tag != 16)goto _LL12E;else{_tmp1E9=_tmp1E8->f1;}}{struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmp1EA=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1EA->tag != 16)goto _LL12E;else{_tmp1EB=_tmp1EA->f1;}};_LL12D:
# 737
 _tmp1E9->uppers=Cyc_Port_filter_self(t1,_tmp1E9->uppers);
_tmp1EB->lowers=Cyc_Port_filter_self(t2,_tmp1EB->lowers);
_tmp1E9->uppers=Cyc_Port_insert_upper(t1,t2,& _tmp1E9->uppers);
_tmp1EB->lowers=Cyc_Port_insert_lower(t2,t1,& _tmp1EB->lowers);
return 1;_LL12E: {struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmp1EC=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp1A7.f1;if(_tmp1EC->tag != 16)goto _LL130;else{_tmp1ED=_tmp1EC->f1;}}_LL12F:
# 743
 _tmp1ED->uppers=Cyc_Port_filter_self(t1,_tmp1ED->uppers);
_tmp1ED->uppers=Cyc_Port_insert_upper(t1,t2,& _tmp1ED->uppers);
return 1;_LL130: {struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmp1EE=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp1A7.f2;if(_tmp1EE->tag != 16)goto _LL132;else{_tmp1EF=_tmp1EE->f1;}}_LL131:
# 747
 _tmp1EF->lowers=Cyc_Port_filter_self(t2,_tmp1EF->lowers);
_tmp1EF->lowers=Cyc_Port_insert_lower(t2,t1,& _tmp1EF->lowers);
return 1;_LL132:;_LL133:
 return Cyc_Port_unifies(t1,t2);_LL103:;};}struct Cyc_Port_GlobalCenv{struct Cyc_Dict_Dict typedef_dict;struct Cyc_Dict_Dict struct_dict;struct Cyc_Dict_Dict union_dict;void*return_type;struct Cyc_List_List*qualifier_edits;struct Cyc_List_List*pointer_edits;struct Cyc_List_List*zeroterm_edits;struct Cyc_List_List*edits;int porting;};
# 756
typedef struct Cyc_Port_GlobalCenv*Cyc_Port_gcenv_t;
# 779
enum Cyc_Port_CPos{Cyc_Port_FnRes_pos  = 0,Cyc_Port_FnArg_pos  = 1,Cyc_Port_FnBody_pos  = 2,Cyc_Port_Toplevel_pos  = 3};
typedef enum Cyc_Port_CPos Cyc_Port_cpos_t;struct Cyc_Port_GlobalCenv;struct Cyc_Port_Cenv{struct Cyc_Port_GlobalCenv*gcenv;struct Cyc_Dict_Dict var_dict;enum Cyc_Port_CPos cpos;};
# 782
typedef struct Cyc_Port_Cenv*Cyc_Port_cenv_t;struct Cyc_Port_Cenv;struct Cyc_Port_GlobalCenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_GlobalCenv;struct Cyc_Port_GlobalCenv;
# 792
static struct Cyc_Port_Cenv*Cyc_Port_empty_cenv(){
struct Cyc_Port_GlobalCenv*_tmp51C;struct Cyc_Port_GlobalCenv*g=(_tmp51C=_cycalloc(sizeof(*_tmp51C)),((_tmp51C->typedef_dict=((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _tuple0*,struct _tuple0*)))Cyc_Dict_empty)(Cyc_Absyn_qvar_cmp),((_tmp51C->struct_dict=
((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _tuple0*,struct _tuple0*)))Cyc_Dict_empty)(Cyc_Absyn_qvar_cmp),((_tmp51C->union_dict=
((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _tuple0*,struct _tuple0*)))Cyc_Dict_empty)(Cyc_Absyn_qvar_cmp),((_tmp51C->qualifier_edits=0,((_tmp51C->pointer_edits=0,((_tmp51C->zeroterm_edits=0,((_tmp51C->edits=0,((_tmp51C->porting=0,((_tmp51C->return_type=
# 801
Cyc_Port_void_ct(),_tmp51C)))))))))))))))))));
struct Cyc_Port_Cenv*_tmp51D;return(_tmp51D=_cycalloc(sizeof(*_tmp51D)),((_tmp51D->gcenv=g,((_tmp51D->cpos=Cyc_Port_Toplevel_pos,((_tmp51D->var_dict=
# 804
((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _tuple0*,struct _tuple0*)))Cyc_Dict_empty)(Cyc_Absyn_qvar_cmp),_tmp51D)))))));}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 810
static int Cyc_Port_in_fn_arg(struct Cyc_Port_Cenv*env){
return env->cpos == Cyc_Port_FnArg_pos;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 813
static int Cyc_Port_in_fn_res(struct Cyc_Port_Cenv*env){
return env->cpos == Cyc_Port_FnRes_pos;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 816
static int Cyc_Port_in_toplevel(struct Cyc_Port_Cenv*env){
return env->cpos == Cyc_Port_Toplevel_pos;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 819
static void*Cyc_Port_lookup_return_type(struct Cyc_Port_Cenv*env){
return(env->gcenv)->return_type;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 822
static void*Cyc_Port_lookup_typedef(struct Cyc_Port_Cenv*env,struct _tuple0*n){
struct _handler_cons _tmp1F2;_push_handler(& _tmp1F2);{int _tmp1F4=0;if(setjmp(_tmp1F2.handler))_tmp1F4=1;if(!_tmp1F4){
{void*_tmp1F6;struct _tuple10 _tmp1F5=*((struct _tuple10*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)((env->gcenv)->typedef_dict,n);_tmp1F6=_tmp1F5.f1;{
void*_tmp1F7=_tmp1F6;_npop_handler(0);return _tmp1F7;};}
# 824
;_pop_handler();}else{void*_tmp1F3=(void*)_exn_thrown;void*_tmp1F9=_tmp1F3;void*_tmp1FB;_LL135: {struct Cyc_Dict_Absent_exn_struct*_tmp1FA=(struct Cyc_Dict_Absent_exn_struct*)_tmp1F9;if(_tmp1FA->tag != Cyc_Dict_Absent)goto _LL137;}_LL136:
# 831
 return Cyc_Absyn_sint_typ;_LL137: _tmp1FB=_tmp1F9;_LL138:(void)_throw(_tmp1FB);_LL134:;}};}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 836
static void*Cyc_Port_lookup_typedef_ctype(struct Cyc_Port_Cenv*env,struct _tuple0*n){
struct _handler_cons _tmp1FC;_push_handler(& _tmp1FC);{int _tmp1FE=0;if(setjmp(_tmp1FC.handler))_tmp1FE=1;if(!_tmp1FE){
{void*_tmp200;struct _tuple10 _tmp1FF=*((struct _tuple10*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)((env->gcenv)->typedef_dict,n);_tmp200=_tmp1FF.f2;{
void*_tmp201=_tmp200;_npop_handler(0);return _tmp201;};}
# 838
;_pop_handler();}else{void*_tmp1FD=(void*)_exn_thrown;void*_tmp203=_tmp1FD;void*_tmp205;_LL13A: {struct Cyc_Dict_Absent_exn_struct*_tmp204=(struct Cyc_Dict_Absent_exn_struct*)_tmp203;if(_tmp204->tag != Cyc_Dict_Absent)goto _LL13C;}_LL13B:
# 845
 return Cyc_Port_var();_LL13C: _tmp205=_tmp203;_LL13D:(void)_throw(_tmp205);_LL139:;}};}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 851
static struct _tuple11*Cyc_Port_lookup_struct_decl(struct Cyc_Port_Cenv*env,struct _tuple0*n){
# 853
struct _tuple11**_tmp206=((struct _tuple11**(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup_opt)((env->gcenv)->struct_dict,n);
if((unsigned int)_tmp206)
return*_tmp206;else{
# 857
struct Cyc_Absyn_Aggrdecl*_tmp51E;struct Cyc_Absyn_Aggrdecl*_tmp207=(_tmp51E=_cycalloc(sizeof(*_tmp51E)),((_tmp51E->kind=Cyc_Absyn_StructA,((_tmp51E->sc=Cyc_Absyn_Public,((_tmp51E->name=n,((_tmp51E->tvs=0,((_tmp51E->impl=0,((_tmp51E->attributes=0,_tmp51E)))))))))))));
# 859
struct _tuple11*_tmp51F;struct _tuple11*p=(_tmp51F=_cycalloc(sizeof(*_tmp51F)),((_tmp51F->f1=_tmp207,((_tmp51F->f2=0,_tmp51F)))));
(env->gcenv)->struct_dict=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _tuple0*k,struct _tuple11*v))Cyc_Dict_insert)((env->gcenv)->struct_dict,n,p);
return p;}}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 867
static struct _tuple11*Cyc_Port_lookup_union_decl(struct Cyc_Port_Cenv*env,struct _tuple0*n){
# 869
struct _tuple11**_tmp20A=((struct _tuple11**(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup_opt)((env->gcenv)->union_dict,n);
if((unsigned int)_tmp20A)
return*_tmp20A;else{
# 873
struct Cyc_Absyn_Aggrdecl*_tmp520;struct Cyc_Absyn_Aggrdecl*_tmp20B=(_tmp520=_cycalloc(sizeof(*_tmp520)),((_tmp520->kind=Cyc_Absyn_UnionA,((_tmp520->sc=Cyc_Absyn_Public,((_tmp520->name=n,((_tmp520->tvs=0,((_tmp520->impl=0,((_tmp520->attributes=0,_tmp520)))))))))))));
# 875
struct _tuple11*_tmp521;struct _tuple11*p=(_tmp521=_cycalloc(sizeof(*_tmp521)),((_tmp521->f1=_tmp20B,((_tmp521->f2=0,_tmp521)))));
(env->gcenv)->union_dict=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _tuple0*k,struct _tuple11*v))Cyc_Dict_insert)((env->gcenv)->union_dict,n,p);
return p;}}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct _tuple13{void*f1;struct _tuple10*f2;};
# 882
static struct _tuple10 Cyc_Port_lookup_var(struct Cyc_Port_Cenv*env,struct _tuple0*x){
struct _handler_cons _tmp20E;_push_handler(& _tmp20E);{int _tmp210=0;if(setjmp(_tmp20E.handler))_tmp210=1;if(!_tmp210){
{struct _tuple10*_tmp212;struct _tuple13 _tmp211=*((struct _tuple13*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)(env->var_dict,x);_tmp212=_tmp211.f2;{
struct _tuple10 _tmp213=*_tmp212;_npop_handler(0);return _tmp213;};}
# 884
;_pop_handler();}else{void*_tmp20F=(void*)_exn_thrown;void*_tmp215=_tmp20F;void*_tmp217;_LL13F: {struct Cyc_Dict_Absent_exn_struct*_tmp216=(struct Cyc_Dict_Absent_exn_struct*)_tmp215;if(_tmp216->tag != Cyc_Dict_Absent)goto _LL141;}_LL140: {
# 891
struct _tuple10 _tmp522;return(_tmp522.f1=Cyc_Port_var(),((_tmp522.f2=Cyc_Port_var(),_tmp522)));}_LL141: _tmp217=_tmp215;_LL142:(void)_throw(_tmp217);_LL13E:;}};}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 894
static struct _tuple13*Cyc_Port_lookup_full_var(struct Cyc_Port_Cenv*env,struct _tuple0*x){
return((struct _tuple13*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)(env->var_dict,x);}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 898
static int Cyc_Port_declared_var(struct Cyc_Port_Cenv*env,struct _tuple0*x){
return((int(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_member)(env->var_dict,x);}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 902
static int Cyc_Port_isfn(struct Cyc_Port_Cenv*env,struct _tuple0*x){
struct _handler_cons _tmp219;_push_handler(& _tmp219);{int _tmp21B=0;if(setjmp(_tmp219.handler))_tmp21B=1;if(!_tmp21B){
{void*_tmp21D;struct _tuple13 _tmp21C=*((struct _tuple13*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)(env->var_dict,x);_tmp21D=_tmp21C.f1;
LOOP: {void*_tmp21E=_tmp21D;struct _tuple0*_tmp220;_LL144: {struct Cyc_Absyn_TypedefType_Absyn_Type_struct*_tmp21F=(struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp21E;if(_tmp21F->tag != 17)goto _LL146;else{_tmp220=_tmp21F->f1;}}_LL145:
 _tmp21D=Cyc_Port_lookup_typedef(env,_tmp220);goto LOOP;_LL146: {struct Cyc_Absyn_FnType_Absyn_Type_struct*_tmp221=(struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp21E;if(_tmp221->tag != 9)goto _LL148;}_LL147: {
int _tmp222=1;_npop_handler(0);return _tmp222;}_LL148:;_LL149: {
int _tmp223=0;_npop_handler(0);return _tmp223;}_LL143:;}}
# 904
;_pop_handler();}else{void*_tmp21A=(void*)_exn_thrown;void*_tmp225=_tmp21A;void*_tmp227;_LL14B: {struct Cyc_Dict_Absent_exn_struct*_tmp226=(struct Cyc_Dict_Absent_exn_struct*)_tmp225;if(_tmp226->tag != Cyc_Dict_Absent)goto _LL14D;}_LL14C:
# 915
 return 0;_LL14D: _tmp227=_tmp225;_LL14E:(void)_throw(_tmp227);_LL14A:;}};}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 919
static int Cyc_Port_isarray(struct Cyc_Port_Cenv*env,struct _tuple0*x){
struct _handler_cons _tmp228;_push_handler(& _tmp228);{int _tmp22A=0;if(setjmp(_tmp228.handler))_tmp22A=1;if(!_tmp22A){
{void*_tmp22C;struct _tuple13 _tmp22B=*((struct _tuple13*(*)(struct Cyc_Dict_Dict d,struct _tuple0*k))Cyc_Dict_lookup)(env->var_dict,x);_tmp22C=_tmp22B.f1;
LOOP: {void*_tmp22D=_tmp22C;struct _tuple0*_tmp22F;_LL150: {struct Cyc_Absyn_TypedefType_Absyn_Type_struct*_tmp22E=(struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp22D;if(_tmp22E->tag != 17)goto _LL152;else{_tmp22F=_tmp22E->f1;}}_LL151:
 _tmp22C=Cyc_Port_lookup_typedef(env,_tmp22F);goto LOOP;_LL152: {struct Cyc_Absyn_ArrayType_Absyn_Type_struct*_tmp230=(struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp22D;if(_tmp230->tag != 8)goto _LL154;}_LL153: {
int _tmp231=1;_npop_handler(0);return _tmp231;}_LL154:;_LL155: {
int _tmp232=0;_npop_handler(0);return _tmp232;}_LL14F:;}}
# 921
;_pop_handler();}else{void*_tmp229=(void*)_exn_thrown;void*_tmp234=_tmp229;void*_tmp236;_LL157: {struct Cyc_Dict_Absent_exn_struct*_tmp235=(struct Cyc_Dict_Absent_exn_struct*)_tmp234;if(_tmp235->tag != Cyc_Dict_Absent)goto _LL159;}_LL158:
# 932
 return 0;_LL159: _tmp236=_tmp234;_LL15A:(void)_throw(_tmp236);_LL156:;}};}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 938
static struct Cyc_Port_Cenv*Cyc_Port_set_cpos(struct Cyc_Port_Cenv*env,enum Cyc_Port_CPos cpos){
struct Cyc_Port_Cenv*_tmp523;return(_tmp523=_cycalloc(sizeof(*_tmp523)),((_tmp523->gcenv=env->gcenv,((_tmp523->var_dict=env->var_dict,((_tmp523->cpos=cpos,_tmp523)))))));}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 943
static void Cyc_Port_add_return_type(struct Cyc_Port_Cenv*env,void*t){
(env->gcenv)->return_type=t;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 948
static struct Cyc_Port_Cenv*Cyc_Port_add_var(struct Cyc_Port_Cenv*env,struct _tuple0*x,void*t,void*qual,void*ctype){
# 950
struct _tuple13*_tmp529;struct _tuple10*_tmp528;struct Cyc_Port_Cenv*_tmp527;return(_tmp527=_cycalloc(sizeof(*_tmp527)),((_tmp527->gcenv=env->gcenv,((_tmp527->var_dict=
((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _tuple0*k,struct _tuple13*v))Cyc_Dict_insert)(env->var_dict,x,((_tmp529=_cycalloc(sizeof(*_tmp529)),((_tmp529->f1=t,((_tmp529->f2=((_tmp528=_cycalloc(sizeof(*_tmp528)),((_tmp528->f1=qual,((_tmp528->f2=ctype,_tmp528)))))),_tmp529))))))),((_tmp527->cpos=env->cpos,_tmp527)))))));}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 956
static void Cyc_Port_add_typedef(struct Cyc_Port_Cenv*env,struct _tuple0*n,void*t,void*ct){
struct _tuple10*_tmp52A;(env->gcenv)->typedef_dict=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _tuple0*k,struct _tuple10*v))Cyc_Dict_insert)((env->gcenv)->typedef_dict,n,(
(_tmp52A=_cycalloc(sizeof(*_tmp52A)),((_tmp52A->f1=t,((_tmp52A->f2=ct,_tmp52A)))))));}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct _tuple14{void*f1;void*f2;unsigned int f3;};
# 963
static void Cyc_Port_register_const_cvar(struct Cyc_Port_Cenv*env,void*new_qual,void*orig_qual,unsigned int loc){
# 965
struct _tuple14*_tmp52D;struct Cyc_List_List*_tmp52C;(env->gcenv)->qualifier_edits=((_tmp52C=_cycalloc(sizeof(*_tmp52C)),((_tmp52C->hd=((_tmp52D=_cycalloc(sizeof(*_tmp52D)),((_tmp52D->f1=new_qual,((_tmp52D->f2=orig_qual,((_tmp52D->f3=loc,_tmp52D)))))))),((_tmp52C->tl=(env->gcenv)->qualifier_edits,_tmp52C))))));}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 971
static void Cyc_Port_register_ptr_cvars(struct Cyc_Port_Cenv*env,void*new_ptr,void*orig_ptr,unsigned int loc){
# 973
struct _tuple14*_tmp530;struct Cyc_List_List*_tmp52F;(env->gcenv)->pointer_edits=((_tmp52F=_cycalloc(sizeof(*_tmp52F)),((_tmp52F->hd=((_tmp530=_cycalloc(sizeof(*_tmp530)),((_tmp530->f1=new_ptr,((_tmp530->f2=orig_ptr,((_tmp530->f3=loc,_tmp530)))))))),((_tmp52F->tl=(env->gcenv)->pointer_edits,_tmp52F))))));}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 977
static void Cyc_Port_register_zeroterm_cvars(struct Cyc_Port_Cenv*env,void*new_zt,void*orig_zt,unsigned int loc){
# 979
struct _tuple14*_tmp533;struct Cyc_List_List*_tmp532;(env->gcenv)->zeroterm_edits=((_tmp532=_cycalloc(sizeof(*_tmp532)),((_tmp532->hd=((_tmp533=_cycalloc(sizeof(*_tmp533)),((_tmp533->f1=new_zt,((_tmp533->f2=orig_zt,((_tmp533->f3=loc,_tmp533)))))))),((_tmp532->tl=(env->gcenv)->zeroterm_edits,_tmp532))))));}
# 985
static void*Cyc_Port_main_type(){
struct _tuple8*_tmp534;struct _tuple8*arg1=
(_tmp534=_cycalloc(sizeof(*_tmp534)),((_tmp534->f1=0,((_tmp534->f2=Cyc_Absyn_empty_tqual(0),((_tmp534->f3=Cyc_Absyn_sint_typ,_tmp534)))))));
struct _tuple8*_tmp535;struct _tuple8*arg2=
(_tmp535=_cycalloc(sizeof(*_tmp535)),((_tmp535->f1=0,((_tmp535->f2=Cyc_Absyn_empty_tqual(0),((_tmp535->f3=
Cyc_Absyn_dyneither_typ(Cyc_Absyn_string_typ(Cyc_Absyn_wildtyp(0)),Cyc_Absyn_wildtyp(0),
Cyc_Absyn_empty_tqual(0),((union Cyc_Absyn_Constraint*(*)())Cyc_Absyn_empty_conref)()),_tmp535)))))));
struct Cyc_Absyn_FnType_Absyn_Type_struct _tmp53F;struct _tuple8*_tmp53E[2];struct Cyc_Absyn_FnInfo _tmp53D;struct Cyc_Absyn_FnType_Absyn_Type_struct*_tmp53C;return(void*)((_tmp53C=_cycalloc(sizeof(*_tmp53C)),((_tmp53C[0]=((_tmp53F.tag=9,((_tmp53F.f1=((_tmp53D.tvars=0,((_tmp53D.effect=0,((_tmp53D.ret_tqual=
Cyc_Absyn_empty_tqual(0),((_tmp53D.ret_typ=Cyc_Absyn_sint_typ,((_tmp53D.args=(
# 995
(_tmp53E[1]=arg2,((_tmp53E[0]=arg1,((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp53E,sizeof(struct _tuple8*),2)))))),((_tmp53D.c_varargs=0,((_tmp53D.cyc_varargs=0,((_tmp53D.rgn_po=0,((_tmp53D.attributes=0,_tmp53D)))))))))))))))))),_tmp53F)))),_tmp53C))));}struct Cyc_Port_Cenv;
# 1001
static void*Cyc_Port_type_to_ctype(struct Cyc_Port_Cenv*env,void*t);struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1004
static struct Cyc_Port_Cenv*Cyc_Port_initial_cenv(){
struct Cyc_Port_Cenv*_tmp248=Cyc_Port_empty_cenv();
# 1007
{struct _tuple0*_tmp540;_tmp248=Cyc_Port_add_var(_tmp248,((_tmp540=_cycalloc(sizeof(*_tmp540)),((_tmp540->f1=Cyc_Absyn_Loc_n,((_tmp540->f2=_init_dyneither_ptr(_cycalloc(sizeof(struct _dyneither_ptr)),"main",sizeof(char),5),_tmp540)))))),Cyc_Port_main_type(),Cyc_Port_const_ct(),
Cyc_Port_type_to_ctype(_tmp248,Cyc_Port_main_type()));}
return _tmp248;}
# 1015
static void Cyc_Port_region_counts(struct Cyc_Dict_Dict*cts,void*t){
void*_tmp24B=Cyc_Port_compress_ct(t);struct _dyneither_ptr*_tmp25A;void*_tmp25C;void*_tmp25D;void*_tmp25E;void*_tmp25F;void*_tmp260;void*_tmp262;void*_tmp263;void*_tmp264;void*_tmp266;struct Cyc_List_List*_tmp267;_LL15C: {struct Cyc_Port_Const_ct_Port_Ctype_struct*_tmp24C=(struct Cyc_Port_Const_ct_Port_Ctype_struct*)_tmp24B;if(_tmp24C->tag != 0)goto _LL15E;}_LL15D:
 goto _LL15F;_LL15E: {struct Cyc_Port_Notconst_ct_Port_Ctype_struct*_tmp24D=(struct Cyc_Port_Notconst_ct_Port_Ctype_struct*)_tmp24B;if(_tmp24D->tag != 1)goto _LL160;}_LL15F:
 goto _LL161;_LL160: {struct Cyc_Port_Thin_ct_Port_Ctype_struct*_tmp24E=(struct Cyc_Port_Thin_ct_Port_Ctype_struct*)_tmp24B;if(_tmp24E->tag != 2)goto _LL162;}_LL161:
 goto _LL163;_LL162: {struct Cyc_Port_Fat_ct_Port_Ctype_struct*_tmp24F=(struct Cyc_Port_Fat_ct_Port_Ctype_struct*)_tmp24B;if(_tmp24F->tag != 3)goto _LL164;}_LL163:
 goto _LL165;_LL164: {struct Cyc_Port_Void_ct_Port_Ctype_struct*_tmp250=(struct Cyc_Port_Void_ct_Port_Ctype_struct*)_tmp24B;if(_tmp250->tag != 4)goto _LL166;}_LL165:
 goto _LL167;_LL166: {struct Cyc_Port_Zero_ct_Port_Ctype_struct*_tmp251=(struct Cyc_Port_Zero_ct_Port_Ctype_struct*)_tmp24B;if(_tmp251->tag != 5)goto _LL168;}_LL167:
 goto _LL169;_LL168: {struct Cyc_Port_Arith_ct_Port_Ctype_struct*_tmp252=(struct Cyc_Port_Arith_ct_Port_Ctype_struct*)_tmp24B;if(_tmp252->tag != 6)goto _LL16A;}_LL169:
 goto _LL16B;_LL16A: {struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*_tmp253=(struct Cyc_Port_UnknownAggr_ct_Port_Ctype_struct*)_tmp24B;if(_tmp253->tag != 14)goto _LL16C;}_LL16B:
 goto _LL16D;_LL16C: {struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*_tmp254=(struct Cyc_Port_KnownAggr_ct_Port_Ctype_struct*)_tmp24B;if(_tmp254->tag != 13)goto _LL16E;}_LL16D:
 goto _LL16F;_LL16E: {struct Cyc_Port_Var_ct_Port_Ctype_struct*_tmp255=(struct Cyc_Port_Var_ct_Port_Ctype_struct*)_tmp24B;if(_tmp255->tag != 16)goto _LL170;}_LL16F:
 goto _LL171;_LL170: {struct Cyc_Port_Zterm_ct_Port_Ctype_struct*_tmp256=(struct Cyc_Port_Zterm_ct_Port_Ctype_struct*)_tmp24B;if(_tmp256->tag != 8)goto _LL172;}_LL171:
 goto _LL173;_LL172: {struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*_tmp257=(struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*)_tmp24B;if(_tmp257->tag != 9)goto _LL174;}_LL173:
 goto _LL175;_LL174: {struct Cyc_Port_Heap_ct_Port_Ctype_struct*_tmp258=(struct Cyc_Port_Heap_ct_Port_Ctype_struct*)_tmp24B;if(_tmp258->tag != 7)goto _LL176;}_LL175:
 return;_LL176: {struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmp259=(struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp24B;if(_tmp259->tag != 10)goto _LL178;else{_tmp25A=_tmp259->f1;}}_LL177:
# 1031
 if(!((int(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_member)(*cts,_tmp25A)){
int*_tmp541;*cts=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k,int*v))Cyc_Dict_insert)(*cts,_tmp25A,((_tmp541=_cycalloc_atomic(sizeof(*_tmp541)),((_tmp541[0]=0,_tmp541)))));}{
int*_tmp269=((int*(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_lookup)(*cts,_tmp25A);
*_tmp269=*_tmp269 + 1;
return;};_LL178: {struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp25B=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp24B;if(_tmp25B->tag != 11)goto _LL17A;else{_tmp25C=(void*)_tmp25B->f1;_tmp25D=(void*)_tmp25B->f2;_tmp25E=(void*)_tmp25B->f3;_tmp25F=(void*)_tmp25B->f4;_tmp260=(void*)_tmp25B->f5;}}_LL179:
# 1037
 Cyc_Port_region_counts(cts,_tmp25C);
Cyc_Port_region_counts(cts,_tmp25F);
return;_LL17A: {struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp261=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp24B;if(_tmp261->tag != 12)goto _LL17C;else{_tmp262=(void*)_tmp261->f1;_tmp263=(void*)_tmp261->f2;_tmp264=(void*)_tmp261->f3;}}_LL17B:
# 1041
 Cyc_Port_region_counts(cts,_tmp262);
return;_LL17C: {struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp265=(struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp24B;if(_tmp265->tag != 15)goto _LL15B;else{_tmp266=(void*)_tmp265->f1;_tmp267=_tmp265->f2;}}_LL17D:
# 1044
 Cyc_Port_region_counts(cts,_tmp266);
for(0;(unsigned int)_tmp267;_tmp267=_tmp267->tl){Cyc_Port_region_counts(cts,(void*)_tmp267->hd);}
return;_LL15B:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;
# 1055
static void Cyc_Port_register_rgns(struct Cyc_Port_Cenv*env,struct Cyc_Dict_Dict counts,int fn_res,void*t,void*c){
# 1057
c=Cyc_Port_compress_ct(c);{
struct _tuple10 _tmp542;struct _tuple10 _tmp26B=(_tmp542.f1=t,((_tmp542.f2=c,_tmp542)));void*_tmp26D;void*_tmp26E;struct Cyc_Absyn_PtrLoc*_tmp26F;void*_tmp271;void*_tmp272;void*_tmp274;void*_tmp276;void*_tmp278;struct Cyc_List_List*_tmp279;void*_tmp27B;struct Cyc_List_List*_tmp27C;_LL17F:{struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp26C=(struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp26B.f1;if(_tmp26C->tag != 5)goto _LL181;else{_tmp26D=(_tmp26C->f1).elt_typ;_tmp26E=((_tmp26C->f1).ptr_atts).rgn;_tmp26F=((_tmp26C->f1).ptr_atts).ptrloc;}}{struct Cyc_Port_Ptr_ct_Port_Ctype_struct*_tmp270=(struct Cyc_Port_Ptr_ct_Port_Ctype_struct*)_tmp26B.f2;if(_tmp270->tag != 11)goto _LL181;else{_tmp271=(void*)_tmp270->f1;_tmp272=(void*)_tmp270->f4;}};_LL180:
# 1060
 Cyc_Port_register_rgns(env,counts,fn_res,_tmp26D,_tmp271);{
unsigned int _tmp27D=(unsigned int)_tmp26F?_tmp26F->rgn_loc:(unsigned int)0;
_tmp272=Cyc_Port_compress_ct(_tmp272);
{struct _tuple10 _tmp543;struct _tuple10 _tmp27F=(_tmp543.f1=_tmp26E,((_tmp543.f2=_tmp272,_tmp543)));struct _dyneither_ptr*_tmp284;_LL188:{struct Cyc_Absyn_Evar_Absyn_Type_struct*_tmp280=(struct Cyc_Absyn_Evar_Absyn_Type_struct*)_tmp27F.f1;if(_tmp280->tag != 1)goto _LL18A;}{struct Cyc_Port_Heap_ct_Port_Ctype_struct*_tmp281=(struct Cyc_Port_Heap_ct_Port_Ctype_struct*)_tmp27F.f2;if(_tmp281->tag != 7)goto _LL18A;};if(!(!fn_res))goto _LL18A;_LL189:
# 1065
{struct Cyc_Port_Edit*_tmp54C;const char*_tmp54B;const char*_tmp54A;struct Cyc_List_List*_tmp549;(env->gcenv)->edits=(
(_tmp549=_cycalloc(sizeof(*_tmp549)),((_tmp549->hd=((_tmp54C=_cycalloc(sizeof(*_tmp54C)),((_tmp54C->loc=_tmp27D,((_tmp54C->old_string=((_tmp54B="`H ",_tag_dyneither(_tmp54B,sizeof(char),4))),((_tmp54C->new_string=((_tmp54A="",_tag_dyneither(_tmp54A,sizeof(char),1))),_tmp54C)))))))),((_tmp549->tl=(env->gcenv)->edits,_tmp549))))));}
goto _LL187;_LL18A:{struct Cyc_Absyn_Evar_Absyn_Type_struct*_tmp282=(struct Cyc_Absyn_Evar_Absyn_Type_struct*)_tmp27F.f1;if(_tmp282->tag != 1)goto _LL18C;}{struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*_tmp283=(struct Cyc_Port_RgnVar_ct_Port_Ctype_struct*)_tmp27F.f2;if(_tmp283->tag != 10)goto _LL18C;else{_tmp284=_tmp283->f1;}};_LL18B: {
# 1069
int _tmp289=*((int*(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_lookup)(counts,_tmp284);
if(_tmp289 > 1){
struct Cyc_Port_Edit*_tmp55C;const char*_tmp55B;void*_tmp55A[1];struct Cyc_String_pa_PrintArg_struct _tmp559;const char*_tmp558;struct Cyc_List_List*_tmp557;(env->gcenv)->edits=(
(_tmp557=_cycalloc(sizeof(*_tmp557)),((_tmp557->hd=((_tmp55C=_cycalloc(sizeof(*_tmp55C)),((_tmp55C->loc=_tmp27D,((_tmp55C->old_string=(struct _dyneither_ptr)((_tmp559.tag=0,((_tmp559.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp284),((_tmp55A[0]=& _tmp559,Cyc_aprintf(((_tmp55B="%s ",_tag_dyneither(_tmp55B,sizeof(char),4))),_tag_dyneither(_tmp55A,sizeof(void*),1)))))))),((_tmp55C->new_string=((_tmp558="",_tag_dyneither(_tmp558,sizeof(char),1))),_tmp55C)))))))),((_tmp557->tl=(env->gcenv)->edits,_tmp557))))));}
goto _LL187;}_LL18C:;_LL18D:
 goto _LL187;_LL187:;}
# 1076
goto _LL17E;};_LL181:{struct Cyc_Absyn_ArrayType_Absyn_Type_struct*_tmp273=(struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp26B.f1;if(_tmp273->tag != 8)goto _LL183;else{_tmp274=(_tmp273->f1).elt_type;}}{struct Cyc_Port_Array_ct_Port_Ctype_struct*_tmp275=(struct Cyc_Port_Array_ct_Port_Ctype_struct*)_tmp26B.f2;if(_tmp275->tag != 12)goto _LL183;else{_tmp276=(void*)_tmp275->f1;}};_LL182:
# 1078
 Cyc_Port_register_rgns(env,counts,fn_res,_tmp274,_tmp276);
goto _LL17E;_LL183:{struct Cyc_Absyn_FnType_Absyn_Type_struct*_tmp277=(struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp26B.f1;if(_tmp277->tag != 9)goto _LL185;else{_tmp278=(_tmp277->f1).ret_typ;_tmp279=(_tmp277->f1).args;}}{struct Cyc_Port_Fn_ct_Port_Ctype_struct*_tmp27A=(struct Cyc_Port_Fn_ct_Port_Ctype_struct*)_tmp26B.f2;if(_tmp27A->tag != 15)goto _LL185;else{_tmp27B=(void*)_tmp27A->f1;_tmp27C=_tmp27A->f2;}};_LL184:
# 1081
 Cyc_Port_register_rgns(env,counts,1,_tmp278,_tmp27B);
for(0;_tmp279 != 0  && _tmp27C != 0;(_tmp279=_tmp279->tl,_tmp27C=_tmp27C->tl)){
void*_tmp291;struct _tuple8 _tmp290=*((struct _tuple8*)_tmp279->hd);_tmp291=_tmp290.f3;{
void*_tmp292=(void*)_tmp27C->hd;
Cyc_Port_register_rgns(env,counts,0,_tmp291,_tmp292);};}
# 1087
goto _LL17E;_LL185:;_LL186:
 goto _LL17E;_LL17E:;};}struct Cyc_Port_Cenv;
# 1094
static void*Cyc_Port_gen_exp(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Exp*e);struct Cyc_Port_Cenv;
static void Cyc_Port_gen_stmt(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Stmt*s);struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
static struct Cyc_Port_Cenv*Cyc_Port_gen_localdecl(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Decl*d);struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1099
static int Cyc_Port_is_char(struct Cyc_Port_Cenv*env,void*t){
void*_tmp293=t;struct _tuple0*_tmp295;enum Cyc_Absyn_Size_of _tmp297;_LL18F: {struct Cyc_Absyn_TypedefType_Absyn_Type_struct*_tmp294=(struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp293;if(_tmp294->tag != 17)goto _LL191;else{_tmp295=_tmp294->f1;}}_LL190:
# 1102
(*_tmp295).f1=Cyc_Absyn_Loc_n;
return Cyc_Port_is_char(env,Cyc_Port_lookup_typedef(env,_tmp295));_LL191: {struct Cyc_Absyn_IntType_Absyn_Type_struct*_tmp296=(struct Cyc_Absyn_IntType_Absyn_Type_struct*)_tmp293;if(_tmp296->tag != 6)goto _LL193;else{_tmp297=_tmp296->f2;}}_LL192:
 return 1;_LL193:;_LL194:
 return 0;_LL18E:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1110
static void*Cyc_Port_type_to_ctype(struct Cyc_Port_Cenv*env,void*t){
void*_tmp298=t;struct _tuple0*_tmp29A;void*_tmp29D;struct Cyc_Absyn_Tqual _tmp29E;void*_tmp29F;union Cyc_Absyn_Constraint*_tmp2A0;union Cyc_Absyn_Constraint*_tmp2A1;union Cyc_Absyn_Constraint*_tmp2A2;struct Cyc_Absyn_PtrLoc*_tmp2A3;void*_tmp2A7;struct Cyc_Absyn_Tqual _tmp2A8;union Cyc_Absyn_Constraint*_tmp2A9;unsigned int _tmp2AA;void*_tmp2AC;struct Cyc_List_List*_tmp2AD;union Cyc_Absyn_AggrInfoU _tmp2AF;struct Cyc_List_List*_tmp2B2;_LL196: {struct Cyc_Absyn_TypedefType_Absyn_Type_struct*_tmp299=(struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp298;if(_tmp299->tag != 17)goto _LL198;else{_tmp29A=_tmp299->f1;}}_LL197:
# 1113
(*_tmp29A).f1=Cyc_Absyn_Loc_n;
return Cyc_Port_lookup_typedef_ctype(env,_tmp29A);_LL198: {struct Cyc_Absyn_VoidType_Absyn_Type_struct*_tmp29B=(struct Cyc_Absyn_VoidType_Absyn_Type_struct*)_tmp298;if(_tmp29B->tag != 0)goto _LL19A;}_LL199:
 return Cyc_Port_void_ct();_LL19A: {struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp29C=(struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp298;if(_tmp29C->tag != 5)goto _LL19C;else{_tmp29D=(_tmp29C->f1).elt_typ;_tmp29E=(_tmp29C->f1).elt_tq;_tmp29F=((_tmp29C->f1).ptr_atts).rgn;_tmp2A0=((_tmp29C->f1).ptr_atts).nullable;_tmp2A1=((_tmp29C->f1).ptr_atts).bounds;_tmp2A2=((_tmp29C->f1).ptr_atts).zero_term;_tmp2A3=((_tmp29C->f1).ptr_atts).ptrloc;}}_LL19B: {
# 1117
unsigned int _tmp2B3=(unsigned int)_tmp2A3?_tmp2A3->ptr_loc:(unsigned int)0;
unsigned int _tmp2B4=(unsigned int)_tmp2A3?_tmp2A3->rgn_loc:(unsigned int)0;
unsigned int _tmp2B5=(unsigned int)_tmp2A3?_tmp2A3->zt_loc:(unsigned int)0;
# 1123
void*_tmp2B6=Cyc_Port_type_to_ctype(env,_tmp29D);
void*new_rgn;
# 1126
{void*_tmp2B7=_tmp29F;struct _dyneither_ptr*_tmp2BA;_LL1AD: {struct Cyc_Absyn_HeapRgn_Absyn_Type_struct*_tmp2B8=(struct Cyc_Absyn_HeapRgn_Absyn_Type_struct*)_tmp2B7;if(_tmp2B8->tag != 20)goto _LL1AF;}_LL1AE:
# 1128
 new_rgn=Cyc_Port_heap_ct();goto _LL1AC;_LL1AF: {struct Cyc_Absyn_VarType_Absyn_Type_struct*_tmp2B9=(struct Cyc_Absyn_VarType_Absyn_Type_struct*)_tmp2B7;if(_tmp2B9->tag != 2)goto _LL1B1;else{_tmp2BA=(_tmp2B9->f1)->name;}}_LL1B0:
# 1130
 new_rgn=Cyc_Port_rgnvar_ct(_tmp2BA);goto _LL1AC;_LL1B1: {struct Cyc_Absyn_Evar_Absyn_Type_struct*_tmp2BB=(struct Cyc_Absyn_Evar_Absyn_Type_struct*)_tmp2B7;if(_tmp2BB->tag != 1)goto _LL1B3;}_LL1B2:
# 1134
 if(Cyc_Port_in_fn_arg(env))
new_rgn=Cyc_Port_rgnvar_ct(Cyc_Port_new_region_var());else{
# 1137
if(Cyc_Port_in_fn_res(env) || Cyc_Port_in_toplevel(env))
new_rgn=Cyc_Port_heap_ct();else{
new_rgn=Cyc_Port_var();}}
goto _LL1AC;_LL1B3:;_LL1B4:
# 1142
 new_rgn=Cyc_Port_heap_ct();goto _LL1AC;_LL1AC:;}{
# 1144
int _tmp2BC=((void*(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp2A1)== (void*)& Cyc_Absyn_DynEither_b_val;
int orig_zt;
{union Cyc_Absyn_Constraint _tmp2BD=*_tmp2A2;_LL1B6: if((_tmp2BD.No_constr).tag != 3)goto _LL1B8;_LL1B7:
 orig_zt=Cyc_Port_is_char(env,_tmp29D);goto _LL1B5;_LL1B8:;_LL1B9:
 orig_zt=((int(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp2A2);goto _LL1B5;_LL1B5:;}
# 1150
if((env->gcenv)->porting){
void*_tmp2BE=Cyc_Port_var();
# 1155
Cyc_Port_register_const_cvar(env,_tmp2BE,_tmp29E.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),_tmp29E.loc);
# 1158
if(_tmp29E.print_const)Cyc_Port_unify_ct(_tmp2BE,Cyc_Port_const_ct());{
# 1164
void*_tmp2BF=Cyc_Port_var();
# 1167
Cyc_Port_register_ptr_cvars(env,_tmp2BF,_tmp2BC?Cyc_Port_fat_ct(): Cyc_Port_thin_ct(),_tmp2B3);
# 1169
if(_tmp2BC)Cyc_Port_unify_ct(_tmp2BF,Cyc_Port_fat_ct());{
void*_tmp2C0=Cyc_Port_var();
# 1173
Cyc_Port_register_zeroterm_cvars(env,_tmp2C0,orig_zt?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct(),_tmp2B5);
# 1175
{union Cyc_Absyn_Constraint _tmp2C1=*_tmp2A2;_LL1BB: if((_tmp2C1.No_constr).tag != 3)goto _LL1BD;_LL1BC:
# 1177
 goto _LL1BA;_LL1BD:;_LL1BE:
# 1179
 Cyc_Port_unify_ct(_tmp2C0,((int(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp2A2)?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct());
goto _LL1BA;_LL1BA:;}
# 1185
return Cyc_Port_ptr_ct(_tmp2B6,_tmp2BE,_tmp2BF,new_rgn,_tmp2C0);};};}else{
# 1189
return Cyc_Port_ptr_ct(_tmp2B6,_tmp29E.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),
_tmp2BC?Cyc_Port_fat_ct(): Cyc_Port_thin_ct(),new_rgn,
orig_zt?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct());}};}_LL19C: {struct Cyc_Absyn_IntType_Absyn_Type_struct*_tmp2A4=(struct Cyc_Absyn_IntType_Absyn_Type_struct*)_tmp298;if(_tmp2A4->tag != 6)goto _LL19E;}_LL19D:
# 1193
 goto _LL19F;_LL19E: {struct Cyc_Absyn_FloatType_Absyn_Type_struct*_tmp2A5=(struct Cyc_Absyn_FloatType_Absyn_Type_struct*)_tmp298;if(_tmp2A5->tag != 7)goto _LL1A0;}_LL19F:
 return Cyc_Port_arith_ct();_LL1A0: {struct Cyc_Absyn_ArrayType_Absyn_Type_struct*_tmp2A6=(struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp298;if(_tmp2A6->tag != 8)goto _LL1A2;else{_tmp2A7=(_tmp2A6->f1).elt_type;_tmp2A8=(_tmp2A6->f1).tq;_tmp2A9=(_tmp2A6->f1).zero_term;_tmp2AA=(_tmp2A6->f1).zt_loc;}}_LL1A1: {
# 1197
void*_tmp2C2=Cyc_Port_type_to_ctype(env,_tmp2A7);
int orig_zt;
{union Cyc_Absyn_Constraint _tmp2C3=*_tmp2A9;_LL1C0: if((_tmp2C3.No_constr).tag != 3)goto _LL1C2;_LL1C1:
 orig_zt=0;goto _LL1BF;_LL1C2:;_LL1C3:
 orig_zt=((int(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp2A9);goto _LL1BF;_LL1BF:;}
# 1203
if((env->gcenv)->porting){
void*_tmp2C4=Cyc_Port_var();
Cyc_Port_register_const_cvar(env,_tmp2C4,_tmp2A8.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),_tmp2A8.loc);{
# 1212
void*_tmp2C5=Cyc_Port_var();
Cyc_Port_register_zeroterm_cvars(env,_tmp2C5,orig_zt?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct(),_tmp2AA);
# 1215
{union Cyc_Absyn_Constraint _tmp2C6=*_tmp2A9;_LL1C5: if((_tmp2C6.No_constr).tag != 3)goto _LL1C7;_LL1C6:
# 1217
 goto _LL1C4;_LL1C7:;_LL1C8:
# 1219
 Cyc_Port_unify_ct(_tmp2C5,((int(*)(union Cyc_Absyn_Constraint*x))Cyc_Absyn_conref_val)(_tmp2A9)?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct());
goto _LL1C4;_LL1C4:;}
# 1222
return Cyc_Port_array_ct(_tmp2C2,_tmp2C4,_tmp2C5);};}else{
# 1224
return Cyc_Port_array_ct(_tmp2C2,_tmp2A8.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),
orig_zt?Cyc_Port_zterm_ct(): Cyc_Port_nozterm_ct());}}_LL1A2: {struct Cyc_Absyn_FnType_Absyn_Type_struct*_tmp2AB=(struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp298;if(_tmp2AB->tag != 9)goto _LL1A4;else{_tmp2AC=(_tmp2AB->f1).ret_typ;_tmp2AD=(_tmp2AB->f1).args;}}_LL1A3: {
# 1228
void*_tmp2C7=Cyc_Port_type_to_ctype(Cyc_Port_set_cpos(env,Cyc_Port_FnRes_pos),_tmp2AC);
struct Cyc_Port_Cenv*_tmp2C8=Cyc_Port_set_cpos(env,Cyc_Port_FnArg_pos);
struct Cyc_List_List*cargs=0;
for(0;_tmp2AD != 0;_tmp2AD=_tmp2AD->tl){
struct Cyc_Absyn_Tqual _tmp2CA;void*_tmp2CB;struct _tuple8 _tmp2C9=*((struct _tuple8*)_tmp2AD->hd);_tmp2CA=_tmp2C9.f2;_tmp2CB=_tmp2C9.f3;{
struct Cyc_List_List*_tmp55D;cargs=((_tmp55D=_cycalloc(sizeof(*_tmp55D)),((_tmp55D->hd=Cyc_Port_type_to_ctype(_tmp2C8,_tmp2CB),((_tmp55D->tl=cargs,_tmp55D))))));};}
# 1235
return Cyc_Port_fn_ct(_tmp2C7,((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(cargs));}_LL1A4: {struct Cyc_Absyn_AggrType_Absyn_Type_struct*_tmp2AE=(struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp298;if(_tmp2AE->tag != 11)goto _LL1A6;else{_tmp2AF=(_tmp2AE->f1).aggr_info;}}_LL1A5: {
# 1237
union Cyc_Absyn_AggrInfoU _tmp2CD=_tmp2AF;struct _tuple0*_tmp2CE;struct _tuple0*_tmp2CF;struct Cyc_Absyn_Aggrdecl**_tmp2D0;_LL1CA: if((_tmp2CD.UnknownAggr).tag != 1)goto _LL1CC;if(((struct _tuple2)(_tmp2CD.UnknownAggr).val).f1 != Cyc_Absyn_StructA)goto _LL1CC;_tmp2CE=((struct _tuple2)(_tmp2CD.UnknownAggr).val).f2;_LL1CB:
# 1239
(*_tmp2CE).f1=Cyc_Absyn_Loc_n;{
struct _tuple11*_tmp2D1=Cyc_Port_lookup_struct_decl(env,_tmp2CE);
return Cyc_Port_known_aggr_ct(_tmp2D1);};_LL1CC: if((_tmp2CD.UnknownAggr).tag != 1)goto _LL1CE;if(((struct _tuple2)(_tmp2CD.UnknownAggr).val).f1 != Cyc_Absyn_UnionA)goto _LL1CE;_tmp2CF=((struct _tuple2)(_tmp2CD.UnknownAggr).val).f2;_LL1CD:
# 1243
(*_tmp2CF).f1=Cyc_Absyn_Loc_n;{
struct _tuple11*_tmp2D2=Cyc_Port_lookup_union_decl(env,_tmp2CF);
return Cyc_Port_known_aggr_ct(_tmp2D2);};_LL1CE: if((_tmp2CD.KnownAggr).tag != 2)goto _LL1C9;_tmp2D0=(struct Cyc_Absyn_Aggrdecl**)(_tmp2CD.KnownAggr).val;_LL1CF: {
# 1247
struct Cyc_Absyn_Aggrdecl*_tmp2D3=*_tmp2D0;
switch(_tmp2D3->kind){case Cyc_Absyn_StructA: _LL1D0: {
# 1250
struct _tuple11*_tmp2D4=Cyc_Port_lookup_struct_decl(env,_tmp2D3->name);
return Cyc_Port_known_aggr_ct(_tmp2D4);}case Cyc_Absyn_UnionA: _LL1D1: {
# 1253
struct _tuple11*_tmp2D5=Cyc_Port_lookup_union_decl(env,_tmp2D3->name);
return Cyc_Port_known_aggr_ct(_tmp2D5);}}}_LL1C9:;}_LL1A6: {struct Cyc_Absyn_EnumType_Absyn_Type_struct*_tmp2B0=(struct Cyc_Absyn_EnumType_Absyn_Type_struct*)_tmp298;if(_tmp2B0->tag != 13)goto _LL1A8;}_LL1A7:
# 1257
 return Cyc_Port_arith_ct();_LL1A8: {struct Cyc_Absyn_AnonEnumType_Absyn_Type_struct*_tmp2B1=(struct Cyc_Absyn_AnonEnumType_Absyn_Type_struct*)_tmp298;if(_tmp2B1->tag != 14)goto _LL1AA;else{_tmp2B2=_tmp2B1->f1;}}_LL1A9:
# 1260
 for(0;(unsigned int)_tmp2B2;_tmp2B2=_tmp2B2->tl){
(*((struct Cyc_Absyn_Enumfield*)_tmp2B2->hd)->name).f1=Cyc_Absyn_Loc_n;
env=Cyc_Port_add_var(env,((struct Cyc_Absyn_Enumfield*)_tmp2B2->hd)->name,Cyc_Absyn_sint_typ,Cyc_Port_const_ct(),Cyc_Port_arith_ct());}
# 1264
return Cyc_Port_arith_ct();_LL1AA:;_LL1AB:
 return Cyc_Port_arith_ct();_LL195:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1270
static void*Cyc_Port_gen_primop(struct Cyc_Port_Cenv*env,enum Cyc_Absyn_Primop p,struct Cyc_List_List*args){
void*_tmp2D6=Cyc_Port_compress_ct((void*)((struct Cyc_List_List*)_check_null(args))->hd);
struct Cyc_List_List*_tmp2D7=args->tl;
enum Cyc_Absyn_Primop _tmp2D8=p;enum Cyc_Absyn_Primop _tmp2D9;_LL1D4: if(_tmp2D8 != Cyc_Absyn_Plus)goto _LL1D6;_LL1D5: {
# 1278
void*_tmp2DA=Cyc_Port_compress_ct((void*)((struct Cyc_List_List*)_check_null(_tmp2D7))->hd);
if(Cyc_Port_leq(_tmp2D6,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var()))){
Cyc_Port_leq(_tmp2DA,Cyc_Port_arith_ct());
return _tmp2D6;}else{
if(Cyc_Port_leq(_tmp2DA,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var()))){
Cyc_Port_leq(_tmp2D6,Cyc_Port_arith_ct());
return _tmp2DA;}else{
# 1286
Cyc_Port_leq(_tmp2D6,Cyc_Port_arith_ct());
Cyc_Port_leq(_tmp2DA,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();}}}_LL1D6: if(_tmp2D8 != Cyc_Absyn_Minus)goto _LL1D8;_LL1D7:
# 1295
 if(_tmp2D7 == 0){
# 1297
Cyc_Port_leq(_tmp2D6,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();}else{
# 1300
void*_tmp2DB=Cyc_Port_compress_ct((void*)_tmp2D7->hd);
if(Cyc_Port_leq(_tmp2D6,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var()))){
if(Cyc_Port_leq(_tmp2DB,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var())))
return Cyc_Port_arith_ct();else{
# 1305
Cyc_Port_leq(_tmp2DB,Cyc_Port_arith_ct());
return _tmp2D6;}}else{
# 1309
Cyc_Port_leq(_tmp2D6,Cyc_Port_arith_ct());
Cyc_Port_leq(_tmp2D6,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();}}_LL1D8: if(_tmp2D8 != Cyc_Absyn_Times)goto _LL1DA;_LL1D9:
# 1314
 goto _LL1DB;_LL1DA: if(_tmp2D8 != Cyc_Absyn_Div)goto _LL1DC;_LL1DB: goto _LL1DD;_LL1DC: if(_tmp2D8 != Cyc_Absyn_Mod)goto _LL1DE;_LL1DD: goto _LL1DF;_LL1DE: if(_tmp2D8 != Cyc_Absyn_Eq)goto _LL1E0;_LL1DF: goto _LL1E1;_LL1E0: if(_tmp2D8 != Cyc_Absyn_Neq)goto _LL1E2;_LL1E1: goto _LL1E3;_LL1E2: if(_tmp2D8 != Cyc_Absyn_Lt)goto _LL1E4;_LL1E3: goto _LL1E5;_LL1E4: if(_tmp2D8 != Cyc_Absyn_Gt)goto _LL1E6;_LL1E5: goto _LL1E7;_LL1E6: if(_tmp2D8 != Cyc_Absyn_Lte)goto _LL1E8;_LL1E7:
 goto _LL1E9;_LL1E8: if(_tmp2D8 != Cyc_Absyn_Gte)goto _LL1EA;_LL1E9: goto _LL1EB;_LL1EA: if(_tmp2D8 != Cyc_Absyn_Bitand)goto _LL1EC;_LL1EB: goto _LL1ED;_LL1EC: if(_tmp2D8 != Cyc_Absyn_Bitor)goto _LL1EE;_LL1ED: goto _LL1EF;_LL1EE: if(_tmp2D8 != Cyc_Absyn_Bitxor)goto _LL1F0;_LL1EF: goto _LL1F1;_LL1F0: if(_tmp2D8 != Cyc_Absyn_Bitlshift)goto _LL1F2;_LL1F1: goto _LL1F3;_LL1F2: if(_tmp2D8 != Cyc_Absyn_Bitlrshift)goto _LL1F4;_LL1F3:
 goto _LL1F5;_LL1F4: if(_tmp2D8 != Cyc_Absyn_Bitarshift)goto _LL1F6;_LL1F5:
 Cyc_Port_leq(_tmp2D6,Cyc_Port_arith_ct());
Cyc_Port_leq((void*)((struct Cyc_List_List*)_check_null(_tmp2D7))->hd,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();_LL1F6: if(_tmp2D8 != Cyc_Absyn_Not)goto _LL1F8;_LL1F7:
 goto _LL1F9;_LL1F8: if(_tmp2D8 != Cyc_Absyn_Bitnot)goto _LL1FA;_LL1F9:
# 1322
 Cyc_Port_leq(_tmp2D6,Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();_LL1FA: _tmp2D9=_tmp2D8;_LL1FB: {
const char*_tmp560;void*_tmp55F;(_tmp55F=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp560=".size primop",_tag_dyneither(_tmp560,sizeof(char),13))),_tag_dyneither(_tmp55F,sizeof(void*),0)));}_LL1D3:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;
# 1329
static struct _tuple10 Cyc_Port_gen_lhs(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Exp*e){
void*_tmp2DE=e->r;struct _tuple0*_tmp2E0;struct Cyc_Absyn_Exp*_tmp2E2;struct Cyc_Absyn_Exp*_tmp2E3;struct Cyc_Absyn_Exp*_tmp2E5;struct Cyc_Absyn_Exp*_tmp2E7;struct _dyneither_ptr*_tmp2E8;struct Cyc_Absyn_Exp*_tmp2EA;struct _dyneither_ptr*_tmp2EB;_LL1FD: {struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*_tmp2DF=(struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp2DE;if(_tmp2DF->tag != 1)goto _LL1FF;else{_tmp2E0=_tmp2DF->f1;}}_LL1FE:
# 1332
(*_tmp2E0).f1=Cyc_Absyn_Loc_n;
return Cyc_Port_lookup_var(env,_tmp2E0);_LL1FF: {struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*_tmp2E1=(struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp2DE;if(_tmp2E1->tag != 22)goto _LL201;else{_tmp2E2=_tmp2E1->f1;_tmp2E3=_tmp2E1->f2;}}_LL200: {
# 1335
void*_tmp2EC=Cyc_Port_var();
void*_tmp2ED=Cyc_Port_var();
void*_tmp2EE=Cyc_Port_gen_exp(env,_tmp2E2);
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2E3),Cyc_Port_arith_ct());{
void*_tmp2EF=Cyc_Port_ptr_ct(_tmp2EC,_tmp2ED,Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var());
Cyc_Port_leq(_tmp2EE,_tmp2EF);{
struct _tuple10 _tmp561;return(_tmp561.f1=_tmp2ED,((_tmp561.f2=_tmp2EC,_tmp561)));};};}_LL201: {struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*_tmp2E4=(struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmp2DE;if(_tmp2E4->tag != 19)goto _LL203;else{_tmp2E5=_tmp2E4->f1;}}_LL202: {
# 1343
void*_tmp2F1=Cyc_Port_var();
void*_tmp2F2=Cyc_Port_var();
void*_tmp2F3=Cyc_Port_ptr_ct(_tmp2F1,_tmp2F2,Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var());
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2E5),_tmp2F3);{
struct _tuple10 _tmp562;return(_tmp562.f1=_tmp2F2,((_tmp562.f2=_tmp2F1,_tmp562)));};}_LL203: {struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*_tmp2E6=(struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp2DE;if(_tmp2E6->tag != 20)goto _LL205;else{_tmp2E7=_tmp2E6->f1;_tmp2E8=_tmp2E6->f2;}}_LL204: {
# 1349
void*_tmp2F5=Cyc_Port_var();
void*_tmp2F6=Cyc_Port_var();
void*_tmp2F7=Cyc_Port_gen_exp(env,_tmp2E7);
{struct Cyc_Port_Cfield*_tmp565;struct Cyc_Port_Cfield*_tmp564[1];Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2E7),Cyc_Port_unknown_aggr_ct(((_tmp564[0]=((_tmp565=_cycalloc(sizeof(*_tmp565)),((_tmp565->qual=_tmp2F6,((_tmp565->f=_tmp2E8,((_tmp565->type=_tmp2F5,_tmp565)))))))),((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp564,sizeof(struct Cyc_Port_Cfield*),1))))));}{
struct _tuple10 _tmp566;return(_tmp566.f1=_tmp2F6,((_tmp566.f2=_tmp2F5,_tmp566)));};}_LL205: {struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*_tmp2E9=(struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp2DE;if(_tmp2E9->tag != 21)goto _LL207;else{_tmp2EA=_tmp2E9->f1;_tmp2EB=_tmp2E9->f2;}}_LL206: {
# 1355
void*_tmp2FB=Cyc_Port_var();
void*_tmp2FC=Cyc_Port_var();
void*_tmp2FD=Cyc_Port_gen_exp(env,_tmp2EA);
{struct Cyc_Port_Cfield*_tmp569;struct Cyc_Port_Cfield*_tmp568[1];Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp2EA),Cyc_Port_ptr_ct(Cyc_Port_unknown_aggr_ct(((_tmp568[0]=((_tmp569=_cycalloc(sizeof(*_tmp569)),((_tmp569->qual=_tmp2FC,((_tmp569->f=_tmp2EB,((_tmp569->type=_tmp2FB,_tmp569)))))))),((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp568,sizeof(struct Cyc_Port_Cfield*),1))))),
Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var()));}{
struct _tuple10 _tmp56A;return(_tmp56A.f1=_tmp2FC,((_tmp56A.f2=_tmp2FB,_tmp56A)));};}_LL207:;_LL208: {
struct Cyc_String_pa_PrintArg_struct _tmp572;void*_tmp571[1];const char*_tmp570;void*_tmp56F;(_tmp56F=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)((struct _dyneither_ptr)((_tmp572.tag=0,((_tmp572.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(e)),((_tmp571[0]=& _tmp572,Cyc_aprintf(((_tmp570="gen_lhs: %s",_tag_dyneither(_tmp570,sizeof(char),12))),_tag_dyneither(_tmp571,sizeof(void*),1)))))))),_tag_dyneither(_tmp56F,sizeof(void*),0)));}_LL1FC:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;
# 1366
static void*Cyc_Port_gen_exp(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Exp*e){
void*_tmp305=e->r;struct _tuple0*_tmp316;enum Cyc_Absyn_Primop _tmp318;struct Cyc_List_List*_tmp319;struct Cyc_Absyn_Exp*_tmp31B;struct Cyc_Core_Opt*_tmp31C;struct Cyc_Absyn_Exp*_tmp31D;struct Cyc_Absyn_Exp*_tmp31F;enum Cyc_Absyn_Incrementor _tmp320;struct Cyc_Absyn_Exp*_tmp322;struct Cyc_Absyn_Exp*_tmp323;struct Cyc_Absyn_Exp*_tmp324;struct Cyc_Absyn_Exp*_tmp326;struct Cyc_Absyn_Exp*_tmp327;struct Cyc_Absyn_Exp*_tmp329;struct Cyc_Absyn_Exp*_tmp32A;struct Cyc_Absyn_Exp*_tmp32C;struct Cyc_Absyn_Exp*_tmp32D;struct Cyc_Absyn_Exp*_tmp32F;struct Cyc_List_List*_tmp330;struct Cyc_Absyn_Exp*_tmp333;void*_tmp336;struct Cyc_Absyn_Exp*_tmp337;struct Cyc_Absyn_Exp*_tmp339;struct Cyc_Absyn_Exp*_tmp33B;struct Cyc_Absyn_Exp*_tmp33C;struct Cyc_Absyn_Exp*_tmp33E;struct Cyc_Absyn_Exp*_tmp340;struct _dyneither_ptr*_tmp341;struct Cyc_Absyn_Exp*_tmp343;struct _dyneither_ptr*_tmp344;void**_tmp346;struct Cyc_Absyn_Exp*_tmp347;struct Cyc_Absyn_Exp*_tmp349;struct Cyc_Absyn_Exp*_tmp34A;struct Cyc_Absyn_Stmt*_tmp355;_LL20A: {struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp306=(struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp306->tag != 0)goto _LL20C;else{if(((_tmp306->f1).Char_c).tag != 2)goto _LL20C;}}_LL20B:
 goto _LL20D;_LL20C: {struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp307=(struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp307->tag != 0)goto _LL20E;else{if(((_tmp307->f1).Wchar_c).tag != 3)goto _LL20E;}}_LL20D:
 goto _LL20F;_LL20E: {struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp308=(struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp308->tag != 0)goto _LL210;else{if(((_tmp308->f1).Short_c).tag != 4)goto _LL210;}}_LL20F:
 goto _LL211;_LL210: {struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp309=(struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp309->tag != 0)goto _LL212;else{if(((_tmp309->f1).LongLong_c).tag != 6)goto _LL212;}}_LL211:
 goto _LL213;_LL212: {struct Cyc_Absyn_Sizeoftyp_e_Absyn_Raw_exp_struct*_tmp30A=(struct Cyc_Absyn_Sizeoftyp_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp30A->tag != 16)goto _LL214;}_LL213:
 goto _LL215;_LL214: {struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct*_tmp30B=(struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp30B->tag != 17)goto _LL216;}_LL215:
 goto _LL217;_LL216: {struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct*_tmp30C=(struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp30C->tag != 18)goto _LL218;}_LL217:
 goto _LL219;_LL218: {struct Cyc_Absyn_Enum_e_Absyn_Raw_exp_struct*_tmp30D=(struct Cyc_Absyn_Enum_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp30D->tag != 30)goto _LL21A;}_LL219:
 goto _LL21B;_LL21A: {struct Cyc_Absyn_AnonEnum_e_Absyn_Raw_exp_struct*_tmp30E=(struct Cyc_Absyn_AnonEnum_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp30E->tag != 31)goto _LL21C;}_LL21B:
 goto _LL21D;_LL21C: {struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp30F=(struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp30F->tag != 0)goto _LL21E;else{if(((_tmp30F->f1).Float_c).tag != 7)goto _LL21E;}}_LL21D:
 return Cyc_Port_arith_ct();_LL21E: {struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp310=(struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp310->tag != 0)goto _LL220;else{if(((_tmp310->f1).Int_c).tag != 5)goto _LL220;if(((struct _tuple5)((_tmp310->f1).Int_c).val).f2 != 0)goto _LL220;}}_LL21F:
 return Cyc_Port_zero_ct();_LL220: {struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp311=(struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp311->tag != 0)goto _LL222;else{if(((_tmp311->f1).Int_c).tag != 5)goto _LL222;}}_LL221:
 return Cyc_Port_arith_ct();_LL222: {struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp312=(struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp312->tag != 0)goto _LL224;else{if(((_tmp312->f1).String_c).tag != 8)goto _LL224;}}_LL223:
# 1381
 return Cyc_Port_ptr_ct(Cyc_Port_arith_ct(),Cyc_Port_const_ct(),Cyc_Port_fat_ct(),Cyc_Port_heap_ct(),Cyc_Port_zterm_ct());_LL224: {struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp313=(struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp313->tag != 0)goto _LL226;else{if(((_tmp313->f1).Wstring_c).tag != 9)goto _LL226;}}_LL225:
# 1383
 return Cyc_Port_ptr_ct(Cyc_Port_arith_ct(),Cyc_Port_const_ct(),Cyc_Port_fat_ct(),Cyc_Port_heap_ct(),Cyc_Port_zterm_ct());_LL226: {struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp314=(struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp314->tag != 0)goto _LL228;else{if(((_tmp314->f1).Null_c).tag != 1)goto _LL228;}}_LL227:
 return Cyc_Port_zero_ct();_LL228: {struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*_tmp315=(struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp315->tag != 1)goto _LL22A;else{_tmp316=_tmp315->f1;}}_LL229:
# 1386
(*_tmp316).f1=Cyc_Absyn_Loc_n;{
void*_tmp35A;struct _tuple10 _tmp359=Cyc_Port_lookup_var(env,_tmp316);_tmp35A=_tmp359.f2;
if(Cyc_Port_isfn(env,_tmp316)){
_tmp35A=Cyc_Port_instantiate(_tmp35A);
return Cyc_Port_ptr_ct(_tmp35A,Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_heap_ct(),Cyc_Port_nozterm_ct());}else{
# 1392
if(Cyc_Port_isarray(env,_tmp316)){
void*_tmp35B=Cyc_Port_var();
void*_tmp35C=Cyc_Port_var();
void*_tmp35D=Cyc_Port_var();
void*_tmp35E=Cyc_Port_array_ct(_tmp35B,_tmp35C,_tmp35D);
Cyc_Port_unifies(_tmp35A,_tmp35E);{
void*_tmp35F=Cyc_Port_ptr_ct(_tmp35B,_tmp35C,Cyc_Port_fat_ct(),Cyc_Port_var(),_tmp35D);
return _tmp35F;};}else{
return _tmp35A;}}};_LL22A: {struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*_tmp317=(struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp317->tag != 2)goto _LL22C;else{_tmp318=_tmp317->f1;_tmp319=_tmp317->f2;}}_LL22B:
 return Cyc_Port_gen_primop(env,_tmp318,((struct Cyc_List_List*(*)(void*(*f)(struct Cyc_Port_Cenv*,struct Cyc_Absyn_Exp*),struct Cyc_Port_Cenv*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_gen_exp,env,_tmp319));_LL22C: {struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*_tmp31A=(struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp31A->tag != 3)goto _LL22E;else{_tmp31B=_tmp31A->f1;_tmp31C=_tmp31A->f2;_tmp31D=_tmp31A->f3;}}_LL22D: {
# 1403
void*_tmp361;void*_tmp362;struct _tuple10 _tmp360=Cyc_Port_gen_lhs(env,_tmp31B);_tmp361=_tmp360.f1;_tmp362=_tmp360.f2;
Cyc_Port_unifies(_tmp361,Cyc_Port_notconst_ct());{
void*_tmp363=Cyc_Port_gen_exp(env,_tmp31D);
if((unsigned int)_tmp31C){
struct Cyc_List_List _tmp573;struct Cyc_List_List x2=(_tmp573.hd=_tmp363,((_tmp573.tl=0,_tmp573)));
struct Cyc_List_List _tmp574;struct Cyc_List_List x1=(_tmp574.hd=_tmp362,((_tmp574.tl=& x2,_tmp574)));
void*_tmp364=Cyc_Port_gen_primop(env,(enum Cyc_Absyn_Primop)_tmp31C->v,& x1);
Cyc_Port_leq(_tmp364,_tmp362);
return _tmp362;}else{
# 1413
Cyc_Port_leq(_tmp363,_tmp362);
return _tmp362;}};}_LL22E: {struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*_tmp31E=(struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp31E->tag != 4)goto _LL230;else{_tmp31F=_tmp31E->f1;_tmp320=_tmp31E->f2;}}_LL22F: {
# 1417
void*_tmp368;void*_tmp369;struct _tuple10 _tmp367=Cyc_Port_gen_lhs(env,_tmp31F);_tmp368=_tmp367.f1;_tmp369=_tmp367.f2;
Cyc_Port_unifies(_tmp368,Cyc_Port_notconst_ct());
# 1420
Cyc_Port_leq(_tmp369,Cyc_Port_ptr_ct(Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var()));
Cyc_Port_leq(_tmp369,Cyc_Port_arith_ct());
return _tmp369;}_LL230: {struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*_tmp321=(struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp321->tag != 5)goto _LL232;else{_tmp322=_tmp321->f1;_tmp323=_tmp321->f2;_tmp324=_tmp321->f3;}}_LL231: {
# 1424
void*_tmp36A=Cyc_Port_var();
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp322),Cyc_Port_arith_ct());
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp323),_tmp36A);
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp324),_tmp36A);
return _tmp36A;}_LL232: {struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*_tmp325=(struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp325->tag != 6)goto _LL234;else{_tmp326=_tmp325->f1;_tmp327=_tmp325->f2;}}_LL233:
 _tmp329=_tmp326;_tmp32A=_tmp327;goto _LL235;_LL234: {struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*_tmp328=(struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp328->tag != 7)goto _LL236;else{_tmp329=_tmp328->f1;_tmp32A=_tmp328->f2;}}_LL235:
# 1431
 Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp329),Cyc_Port_arith_ct());
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp32A),Cyc_Port_arith_ct());
return Cyc_Port_arith_ct();_LL236: {struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*_tmp32B=(struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp32B->tag != 8)goto _LL238;else{_tmp32C=_tmp32B->f1;_tmp32D=_tmp32B->f2;}}_LL237:
# 1435
 Cyc_Port_gen_exp(env,_tmp32C);
return Cyc_Port_gen_exp(env,_tmp32D);_LL238: {struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*_tmp32E=(struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp32E->tag != 9)goto _LL23A;else{_tmp32F=_tmp32E->f1;_tmp330=_tmp32E->f2;}}_LL239: {
# 1438
void*_tmp36B=Cyc_Port_var();
void*_tmp36C=Cyc_Port_gen_exp(env,_tmp32F);
struct Cyc_List_List*_tmp36D=((struct Cyc_List_List*(*)(void*(*f)(struct Cyc_Port_Cenv*,struct Cyc_Absyn_Exp*),struct Cyc_Port_Cenv*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Port_gen_exp,env,_tmp330);
struct Cyc_List_List*_tmp36E=((struct Cyc_List_List*(*)(void*(*f)(void*),struct Cyc_List_List*x))Cyc_List_map)((void*(*)(void*x))Cyc_Port_new_var,_tmp36D);
Cyc_Port_unifies(_tmp36C,Cyc_Port_ptr_ct(Cyc_Port_fn_ct(_tmp36B,_tmp36E),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_nozterm_ct()));
for(0;_tmp36D != 0;(_tmp36D=_tmp36D->tl,_tmp36E=_tmp36E->tl)){
Cyc_Port_leq((void*)_tmp36D->hd,(void*)((struct Cyc_List_List*)_check_null(_tmp36E))->hd);}
# 1446
return _tmp36B;}_LL23A: {struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct*_tmp331=(struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp331->tag != 10)goto _LL23C;}_LL23B: {
const char*_tmp577;void*_tmp576;(_tmp576=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp577="throw",_tag_dyneither(_tmp577,sizeof(char),6))),_tag_dyneither(_tmp576,sizeof(void*),0)));}_LL23C: {struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct*_tmp332=(struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp332->tag != 11)goto _LL23E;else{_tmp333=_tmp332->f1;}}_LL23D:
 return Cyc_Port_gen_exp(env,_tmp333);_LL23E: {struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct*_tmp334=(struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp334->tag != 12)goto _LL240;}_LL23F: {
const char*_tmp57A;void*_tmp579;(_tmp579=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp57A="instantiate",_tag_dyneither(_tmp57A,sizeof(char),12))),_tag_dyneither(_tmp579,sizeof(void*),0)));}_LL240: {struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*_tmp335=(struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp335->tag != 13)goto _LL242;else{_tmp336=(void*)_tmp335->f1;_tmp337=_tmp335->f2;}}_LL241: {
# 1451
void*_tmp373=Cyc_Port_type_to_ctype(env,_tmp336);
void*_tmp374=Cyc_Port_gen_exp(env,_tmp337);
Cyc_Port_leq(_tmp373,_tmp374);
return _tmp374;}_LL242: {struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*_tmp338=(struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp338->tag != 14)goto _LL244;else{_tmp339=_tmp338->f1;}}_LL243: {
# 1456
void*_tmp376;void*_tmp377;struct _tuple10 _tmp375=Cyc_Port_gen_lhs(env,_tmp339);_tmp376=_tmp375.f1;_tmp377=_tmp375.f2;
return Cyc_Port_ptr_ct(_tmp377,_tmp376,Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var());}_LL244: {struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*_tmp33A=(struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp33A->tag != 22)goto _LL246;else{_tmp33B=_tmp33A->f1;_tmp33C=_tmp33A->f2;}}_LL245: {
# 1459
void*_tmp378=Cyc_Port_var();
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp33C),Cyc_Port_arith_ct());{
void*_tmp379=Cyc_Port_gen_exp(env,_tmp33B);
Cyc_Port_leq(_tmp379,Cyc_Port_ptr_ct(_tmp378,Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_var(),Cyc_Port_var()));
return _tmp378;};}_LL246: {struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*_tmp33D=(struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp33D->tag != 19)goto _LL248;else{_tmp33E=_tmp33D->f1;}}_LL247: {
# 1465
void*_tmp37A=Cyc_Port_var();
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp33E),Cyc_Port_ptr_ct(_tmp37A,Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var()));
return _tmp37A;}_LL248: {struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*_tmp33F=(struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp33F->tag != 20)goto _LL24A;else{_tmp340=_tmp33F->f1;_tmp341=_tmp33F->f2;}}_LL249: {
# 1469
void*_tmp37B=Cyc_Port_var();
void*_tmp37C=Cyc_Port_gen_exp(env,_tmp340);
{struct Cyc_Port_Cfield*_tmp57D;struct Cyc_Port_Cfield*_tmp57C[1];Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp340),Cyc_Port_unknown_aggr_ct(((_tmp57C[0]=((_tmp57D=_cycalloc(sizeof(*_tmp57D)),((_tmp57D->qual=Cyc_Port_var(),((_tmp57D->f=_tmp341,((_tmp57D->type=_tmp37B,_tmp57D)))))))),((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp57C,sizeof(struct Cyc_Port_Cfield*),1))))));}
return _tmp37B;}_LL24A: {struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*_tmp342=(struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp342->tag != 21)goto _LL24C;else{_tmp343=_tmp342->f1;_tmp344=_tmp342->f2;}}_LL24B: {
# 1474
void*_tmp37F=Cyc_Port_var();
void*_tmp380=Cyc_Port_gen_exp(env,_tmp343);
{struct Cyc_Port_Cfield*_tmp580;struct Cyc_Port_Cfield*_tmp57F[1];Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp343),Cyc_Port_ptr_ct(Cyc_Port_unknown_aggr_ct(((_tmp57F[0]=((_tmp580=_cycalloc(sizeof(*_tmp580)),((_tmp580->qual=Cyc_Port_var(),((_tmp580->f=_tmp344,((_tmp580->type=_tmp37F,_tmp580)))))))),((struct Cyc_List_List*(*)(struct _dyneither_ptr))Cyc_List_list)(_tag_dyneither(_tmp57F,sizeof(struct Cyc_Port_Cfield*),1))))),
Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var(),Cyc_Port_var()));}
return _tmp37F;}_LL24C: {struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*_tmp345=(struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp345->tag != 32)goto _LL24E;else{_tmp346=(_tmp345->f1).elt_type;_tmp347=(_tmp345->f1).num_elts;}}_LL24D:
# 1480
 Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp347),Cyc_Port_arith_ct());{
void*_tmp383=(unsigned int)_tmp346?Cyc_Port_type_to_ctype(env,*_tmp346): Cyc_Port_var();
return Cyc_Port_ptr_ct(_tmp383,Cyc_Port_var(),Cyc_Port_fat_ct(),Cyc_Port_heap_ct(),Cyc_Port_var());};_LL24E: {struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct*_tmp348=(struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp348->tag != 33)goto _LL250;else{_tmp349=_tmp348->f1;_tmp34A=_tmp348->f2;}}_LL24F: {
const char*_tmp583;void*_tmp582;(_tmp582=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp583="Swap_e",_tag_dyneither(_tmp583,sizeof(char),7))),_tag_dyneither(_tmp582,sizeof(void*),0)));}_LL250: {struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct*_tmp34B=(struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp34B->tag != 15)goto _LL252;}_LL251: {
const char*_tmp586;void*_tmp585;(_tmp585=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp586="New_e",_tag_dyneither(_tmp586,sizeof(char),6))),_tag_dyneither(_tmp585,sizeof(void*),0)));}_LL252: {struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct*_tmp34C=(struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp34C->tag != 23)goto _LL254;}_LL253: {
const char*_tmp589;void*_tmp588;(_tmp588=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp589="Tuple_e",_tag_dyneither(_tmp589,sizeof(char),8))),_tag_dyneither(_tmp588,sizeof(void*),0)));}_LL254: {struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct*_tmp34D=(struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp34D->tag != 24)goto _LL256;}_LL255: {
const char*_tmp58C;void*_tmp58B;(_tmp58B=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp58C="CompoundLit_e",_tag_dyneither(_tmp58C,sizeof(char),14))),_tag_dyneither(_tmp58B,sizeof(void*),0)));}_LL256: {struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*_tmp34E=(struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp34E->tag != 25)goto _LL258;}_LL257: {
const char*_tmp58F;void*_tmp58E;(_tmp58E=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp58F="Array_e",_tag_dyneither(_tmp58F,sizeof(char),8))),_tag_dyneither(_tmp58E,sizeof(void*),0)));}_LL258: {struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct*_tmp34F=(struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp34F->tag != 26)goto _LL25A;}_LL259: {
const char*_tmp592;void*_tmp591;(_tmp591=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp592="Comprehension_e",_tag_dyneither(_tmp592,sizeof(char),16))),_tag_dyneither(_tmp591,sizeof(void*),0)));}_LL25A: {struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*_tmp350=(struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp350->tag != 27)goto _LL25C;}_LL25B: {
const char*_tmp595;void*_tmp594;(_tmp594=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp595="Aggregate_e",_tag_dyneither(_tmp595,sizeof(char),12))),_tag_dyneither(_tmp594,sizeof(void*),0)));}_LL25C: {struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct*_tmp351=(struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp351->tag != 28)goto _LL25E;}_LL25D: {
const char*_tmp598;void*_tmp597;(_tmp597=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp598="AnonStruct_e",_tag_dyneither(_tmp598,sizeof(char),13))),_tag_dyneither(_tmp597,sizeof(void*),0)));}_LL25E: {struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct*_tmp352=(struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp352->tag != 29)goto _LL260;}_LL25F: {
const char*_tmp59B;void*_tmp59A;(_tmp59A=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp59B="Datatype_e",_tag_dyneither(_tmp59B,sizeof(char),11))),_tag_dyneither(_tmp59A,sizeof(void*),0)));}_LL260: {struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*_tmp353=(struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp353->tag != 34)goto _LL262;}_LL261: {
const char*_tmp59E;void*_tmp59D;(_tmp59D=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp59E="UnresolvedMem_e",_tag_dyneither(_tmp59E,sizeof(char),16))),_tag_dyneither(_tmp59D,sizeof(void*),0)));}_LL262: {struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct*_tmp354=(struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp354->tag != 35)goto _LL264;else{_tmp355=_tmp354->f1;}}_LL263: {
const char*_tmp5A1;void*_tmp5A0;(_tmp5A0=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp5A1="StmtExp_e",_tag_dyneither(_tmp5A1,sizeof(char),10))),_tag_dyneither(_tmp5A0,sizeof(void*),0)));}_LL264: {struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct*_tmp356=(struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp356->tag != 37)goto _LL266;}_LL265: {
const char*_tmp5A4;void*_tmp5A3;(_tmp5A3=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp5A4="Valueof_e",_tag_dyneither(_tmp5A4,sizeof(char),10))),_tag_dyneither(_tmp5A3,sizeof(void*),0)));}_LL266: {struct Cyc_Absyn_Asm_e_Absyn_Raw_exp_struct*_tmp357=(struct Cyc_Absyn_Asm_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp357->tag != 38)goto _LL268;}_LL267: {
const char*_tmp5A7;void*_tmp5A6;(_tmp5A6=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp5A7="Asm_e",_tag_dyneither(_tmp5A7,sizeof(char),6))),_tag_dyneither(_tmp5A6,sizeof(void*),0)));}_LL268: {struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct*_tmp358=(struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct*)_tmp305;if(_tmp358->tag != 36)goto _LL209;}_LL269: {
const char*_tmp5AA;void*_tmp5A9;(_tmp5A9=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp5AA="Tagcheck_e",_tag_dyneither(_tmp5AA,sizeof(char),11))),_tag_dyneither(_tmp5A9,sizeof(void*),0)));}_LL209:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1501
static void Cyc_Port_gen_stmt(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Stmt*s){
void*_tmp3A0=s->r;struct Cyc_Absyn_Exp*_tmp3A3;struct Cyc_Absyn_Stmt*_tmp3A5;struct Cyc_Absyn_Stmt*_tmp3A6;struct Cyc_Absyn_Exp*_tmp3A8;struct Cyc_Absyn_Exp*_tmp3AA;struct Cyc_Absyn_Stmt*_tmp3AB;struct Cyc_Absyn_Stmt*_tmp3AC;struct Cyc_Absyn_Exp*_tmp3AE;struct Cyc_Absyn_Stmt*_tmp3AF;struct Cyc_Absyn_Exp*_tmp3B4;struct Cyc_Absyn_Exp*_tmp3B5;struct Cyc_Absyn_Exp*_tmp3B6;struct Cyc_Absyn_Stmt*_tmp3B7;struct Cyc_Absyn_Exp*_tmp3B9;struct Cyc_List_List*_tmp3BA;struct Cyc_Absyn_Decl*_tmp3BD;struct Cyc_Absyn_Stmt*_tmp3BE;struct Cyc_Absyn_Stmt*_tmp3C0;struct Cyc_Absyn_Stmt*_tmp3C2;struct Cyc_Absyn_Exp*_tmp3C3;_LL26B: {struct Cyc_Absyn_Skip_s_Absyn_Raw_stmt_struct*_tmp3A1=(struct Cyc_Absyn_Skip_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3A1->tag != 0)goto _LL26D;}_LL26C:
 return;_LL26D: {struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct*_tmp3A2=(struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3A2->tag != 1)goto _LL26F;else{_tmp3A3=_tmp3A2->f1;}}_LL26E:
 Cyc_Port_gen_exp(env,_tmp3A3);return;_LL26F: {struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*_tmp3A4=(struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3A4->tag != 2)goto _LL271;else{_tmp3A5=_tmp3A4->f1;_tmp3A6=_tmp3A4->f2;}}_LL270:
 Cyc_Port_gen_stmt(env,_tmp3A5);Cyc_Port_gen_stmt(env,_tmp3A6);return;_LL271: {struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct*_tmp3A7=(struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3A7->tag != 3)goto _LL273;else{_tmp3A8=_tmp3A7->f1;}}_LL272: {
# 1507
void*_tmp3C6=Cyc_Port_lookup_return_type(env);
void*_tmp3C7=(unsigned int)_tmp3A8?Cyc_Port_gen_exp(env,_tmp3A8): Cyc_Port_void_ct();
Cyc_Port_leq(_tmp3C7,_tmp3C6);
return;}_LL273: {struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*_tmp3A9=(struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3A9->tag != 4)goto _LL275;else{_tmp3AA=_tmp3A9->f1;_tmp3AB=_tmp3A9->f2;_tmp3AC=_tmp3A9->f3;}}_LL274:
# 1512
 Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp3AA),Cyc_Port_arith_ct());
Cyc_Port_gen_stmt(env,_tmp3AB);
Cyc_Port_gen_stmt(env,_tmp3AC);
return;_LL275: {struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*_tmp3AD=(struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3AD->tag != 5)goto _LL277;else{_tmp3AE=(_tmp3AD->f1).f1;_tmp3AF=_tmp3AD->f2;}}_LL276:
# 1517
 Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp3AE),Cyc_Port_arith_ct());
Cyc_Port_gen_stmt(env,_tmp3AF);
return;_LL277: {struct Cyc_Absyn_Break_s_Absyn_Raw_stmt_struct*_tmp3B0=(struct Cyc_Absyn_Break_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3B0->tag != 6)goto _LL279;}_LL278:
 goto _LL27A;_LL279: {struct Cyc_Absyn_Continue_s_Absyn_Raw_stmt_struct*_tmp3B1=(struct Cyc_Absyn_Continue_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3B1->tag != 7)goto _LL27B;}_LL27A:
 goto _LL27C;_LL27B: {struct Cyc_Absyn_Goto_s_Absyn_Raw_stmt_struct*_tmp3B2=(struct Cyc_Absyn_Goto_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3B2->tag != 8)goto _LL27D;}_LL27C:
 return;_LL27D: {struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*_tmp3B3=(struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3B3->tag != 9)goto _LL27F;else{_tmp3B4=_tmp3B3->f1;_tmp3B5=(_tmp3B3->f2).f1;_tmp3B6=(_tmp3B3->f3).f1;_tmp3B7=_tmp3B3->f4;}}_LL27E:
# 1524
 Cyc_Port_gen_exp(env,_tmp3B4);
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp3B5),Cyc_Port_arith_ct());
Cyc_Port_gen_exp(env,_tmp3B6);
Cyc_Port_gen_stmt(env,_tmp3B7);
return;_LL27F: {struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*_tmp3B8=(struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3B8->tag != 10)goto _LL281;else{_tmp3B9=_tmp3B8->f1;_tmp3BA=_tmp3B8->f2;}}_LL280:
# 1530
 Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp3B9),Cyc_Port_arith_ct());
for(0;(unsigned int)_tmp3BA;_tmp3BA=_tmp3BA->tl){
# 1533
Cyc_Port_gen_stmt(env,((struct Cyc_Absyn_Switch_clause*)_tmp3BA->hd)->body);}
# 1535
return;_LL281: {struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct*_tmp3BB=(struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3BB->tag != 11)goto _LL283;}_LL282: {
const char*_tmp5AD;void*_tmp5AC;(_tmp5AC=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp5AD="fallthru",_tag_dyneither(_tmp5AD,sizeof(char),9))),_tag_dyneither(_tmp5AC,sizeof(void*),0)));}_LL283: {struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*_tmp3BC=(struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3BC->tag != 12)goto _LL285;else{_tmp3BD=_tmp3BC->f1;_tmp3BE=_tmp3BC->f2;}}_LL284:
 env=Cyc_Port_gen_localdecl(env,_tmp3BD);Cyc_Port_gen_stmt(env,_tmp3BE);return;_LL285: {struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct*_tmp3BF=(struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3BF->tag != 13)goto _LL287;else{_tmp3C0=_tmp3BF->f2;}}_LL286:
 Cyc_Port_gen_stmt(env,_tmp3C0);return;_LL287: {struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*_tmp3C1=(struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3C1->tag != 14)goto _LL289;else{_tmp3C2=_tmp3C1->f1;_tmp3C3=(_tmp3C1->f2).f1;}}_LL288:
# 1540
 Cyc_Port_gen_stmt(env,_tmp3C2);
Cyc_Port_leq(Cyc_Port_gen_exp(env,_tmp3C3),Cyc_Port_arith_ct());
return;_LL289: {struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct*_tmp3C4=(struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3C4->tag != 15)goto _LL28B;}_LL28A: {
const char*_tmp5B0;void*_tmp5AF;(_tmp5AF=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp5B0="try/catch",_tag_dyneither(_tmp5B0,sizeof(char),10))),_tag_dyneither(_tmp5AF,sizeof(void*),0)));}_LL28B: {struct Cyc_Absyn_ResetRegion_s_Absyn_Raw_stmt_struct*_tmp3C5=(struct Cyc_Absyn_ResetRegion_s_Absyn_Raw_stmt_struct*)_tmp3A0;if(_tmp3C5->tag != 16)goto _LL26A;}_LL28C: {
const char*_tmp5B3;void*_tmp5B2;(_tmp5B2=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp5B3="reset region",_tag_dyneither(_tmp5B3,sizeof(char),13))),_tag_dyneither(_tmp5B2,sizeof(void*),0)));}_LL26A:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct _tuple15{struct Cyc_List_List*f1;struct Cyc_Absyn_Exp*f2;};
# 1549
static void*Cyc_Port_gen_initializer(struct Cyc_Port_Cenv*env,void*t,struct Cyc_Absyn_Exp*e){
void*_tmp3CE=e->r;struct Cyc_List_List*_tmp3D0;struct Cyc_List_List*_tmp3D2;struct Cyc_List_List*_tmp3D4;_LL28E: {struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*_tmp3CF=(struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmp3CE;if(_tmp3CF->tag != 34)goto _LL290;else{_tmp3D0=_tmp3CF->f2;}}_LL28F:
 _tmp3D2=_tmp3D0;goto _LL291;_LL290: {struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*_tmp3D1=(struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*)_tmp3CE;if(_tmp3D1->tag != 25)goto _LL292;else{_tmp3D2=_tmp3D1->f1;}}_LL291:
 _tmp3D4=_tmp3D2;goto _LL293;_LL292: {struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct*_tmp3D3=(struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct*)_tmp3CE;if(_tmp3D3->tag != 24)goto _LL294;else{_tmp3D4=_tmp3D3->f2;}}_LL293:
# 1554
 LOOP: {
void*_tmp3D7=t;struct _tuple0*_tmp3D9;void*_tmp3DB;union Cyc_Absyn_Constraint*_tmp3DC;unsigned int _tmp3DD;struct _tuple0*_tmp3DF;struct Cyc_Absyn_Aggrdecl*_tmp3E1;_LL29B: {struct Cyc_Absyn_TypedefType_Absyn_Type_struct*_tmp3D8=(struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp3D7;if(_tmp3D8->tag != 17)goto _LL29D;else{_tmp3D9=_tmp3D8->f1;}}_LL29C:
# 1557
(*_tmp3D9).f1=Cyc_Absyn_Loc_n;
t=Cyc_Port_lookup_typedef(env,_tmp3D9);goto LOOP;_LL29D: {struct Cyc_Absyn_ArrayType_Absyn_Type_struct*_tmp3DA=(struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp3D7;if(_tmp3DA->tag != 8)goto _LL29F;else{_tmp3DB=(_tmp3DA->f1).elt_type;_tmp3DC=(_tmp3DA->f1).zero_term;_tmp3DD=(_tmp3DA->f1).zt_loc;}}_LL29E: {
# 1560
void*_tmp3E2=Cyc_Port_var();
void*_tmp3E3=Cyc_Port_var();
void*_tmp3E4=Cyc_Port_var();
void*_tmp3E5=Cyc_Port_type_to_ctype(env,_tmp3DB);
for(0;_tmp3D4 != 0;_tmp3D4=_tmp3D4->tl){
struct Cyc_List_List*_tmp3E7;struct Cyc_Absyn_Exp*_tmp3E8;struct _tuple15 _tmp3E6=*((struct _tuple15*)_tmp3D4->hd);_tmp3E7=_tmp3E6.f1;_tmp3E8=_tmp3E6.f2;
if((unsigned int)_tmp3E7){const char*_tmp5B6;void*_tmp5B5;(_tmp5B5=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp5B6="designators in initializers",_tag_dyneither(_tmp5B6,sizeof(char),28))),_tag_dyneither(_tmp5B5,sizeof(void*),0)));}{
void*_tmp3EB=Cyc_Port_gen_initializer(env,_tmp3DB,_tmp3E8);
Cyc_Port_leq(_tmp3EB,_tmp3E2);};}
# 1570
return Cyc_Port_array_ct(_tmp3E2,_tmp3E3,_tmp3E4);}_LL29F: {struct Cyc_Absyn_AggrType_Absyn_Type_struct*_tmp3DE=(struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp3D7;if(_tmp3DE->tag != 11)goto _LL2A1;else{if((((_tmp3DE->f1).aggr_info).UnknownAggr).tag != 1)goto _LL2A1;if(((struct _tuple2)(((_tmp3DE->f1).aggr_info).UnknownAggr).val).f1 != Cyc_Absyn_StructA)goto _LL2A1;_tmp3DF=((struct _tuple2)(((_tmp3DE->f1).aggr_info).UnknownAggr).val).f2;}}_LL2A0:
# 1572
(*_tmp3DF).f1=Cyc_Absyn_Loc_n;{
struct Cyc_Absyn_Aggrdecl*_tmp3ED;struct _tuple11 _tmp3EC=*Cyc_Port_lookup_struct_decl(env,_tmp3DF);_tmp3ED=_tmp3EC.f1;
if(_tmp3ED == 0){const char*_tmp5B9;void*_tmp5B8;(_tmp5B8=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp5B9="struct is not yet defined",_tag_dyneither(_tmp5B9,sizeof(char),26))),_tag_dyneither(_tmp5B8,sizeof(void*),0)));}
_tmp3E1=_tmp3ED;goto _LL2A2;};_LL2A1: {struct Cyc_Absyn_AggrType_Absyn_Type_struct*_tmp3E0=(struct Cyc_Absyn_AggrType_Absyn_Type_struct*)_tmp3D7;if(_tmp3E0->tag != 11)goto _LL2A3;else{if((((_tmp3E0->f1).aggr_info).KnownAggr).tag != 2)goto _LL2A3;_tmp3E1=*((struct Cyc_Absyn_Aggrdecl**)(((_tmp3E0->f1).aggr_info).KnownAggr).val);}}_LL2A2: {
# 1577
struct Cyc_List_List*_tmp3F0=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp3E1->impl))->fields;
for(0;_tmp3D4 != 0;_tmp3D4=_tmp3D4->tl){
struct Cyc_List_List*_tmp3F2;struct Cyc_Absyn_Exp*_tmp3F3;struct _tuple15 _tmp3F1=*((struct _tuple15*)_tmp3D4->hd);_tmp3F2=_tmp3F1.f1;_tmp3F3=_tmp3F1.f2;
if((unsigned int)_tmp3F2){const char*_tmp5BC;void*_tmp5BB;(_tmp5BB=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp5BC="designators in initializers",_tag_dyneither(_tmp5BC,sizeof(char),28))),_tag_dyneither(_tmp5BB,sizeof(void*),0)));}{
struct Cyc_Absyn_Aggrfield*_tmp3F6=(struct Cyc_Absyn_Aggrfield*)((struct Cyc_List_List*)_check_null(_tmp3F0))->hd;
_tmp3F0=_tmp3F0->tl;{
void*_tmp3F7=Cyc_Port_gen_initializer(env,_tmp3F6->type,_tmp3F3);;};};}
# 1585
return Cyc_Port_type_to_ctype(env,t);}_LL2A3:;_LL2A4: {
const char*_tmp5BF;void*_tmp5BE;(_tmp5BE=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp5BF="bad type for aggregate initializer",_tag_dyneither(_tmp5BF,sizeof(char),35))),_tag_dyneither(_tmp5BE,sizeof(void*),0)));}_LL29A:;}_LL294: {struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp3D5=(struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp3CE;if(_tmp3D5->tag != 0)goto _LL296;else{if(((_tmp3D5->f1).String_c).tag != 8)goto _LL296;}}_LL295:
# 1588
 goto _LL297;_LL296: {struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp3D6=(struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp3CE;if(_tmp3D6->tag != 0)goto _LL298;else{if(((_tmp3D6->f1).Wstring_c).tag != 9)goto _LL298;}}_LL297:
# 1590
 LOOP2: {
void*_tmp3FA=t;struct _tuple0*_tmp3FC;_LL2A6: {struct Cyc_Absyn_TypedefType_Absyn_Type_struct*_tmp3FB=(struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_tmp3FA;if(_tmp3FB->tag != 17)goto _LL2A8;else{_tmp3FC=_tmp3FB->f1;}}_LL2A7:
# 1593
(*_tmp3FC).f1=Cyc_Absyn_Loc_n;
t=Cyc_Port_lookup_typedef(env,_tmp3FC);goto LOOP2;_LL2A8: {struct Cyc_Absyn_ArrayType_Absyn_Type_struct*_tmp3FD=(struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp3FA;if(_tmp3FD->tag != 8)goto _LL2AA;}_LL2A9:
 return Cyc_Port_array_ct(Cyc_Port_arith_ct(),Cyc_Port_const_ct(),Cyc_Port_zterm_ct());_LL2AA:;_LL2AB:
 return Cyc_Port_gen_exp(env,e);_LL2A5:;}_LL298:;_LL299:
# 1598
 return Cyc_Port_gen_exp(env,e);_LL28D:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1603
static struct Cyc_Port_Cenv*Cyc_Port_gen_localdecl(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Decl*d){
void*_tmp3FE=d->r;struct Cyc_Absyn_Vardecl*_tmp400;_LL2AD: {struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*_tmp3FF=(struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp3FE;if(_tmp3FF->tag != 0)goto _LL2AF;else{_tmp400=_tmp3FF->f1;}}_LL2AE: {
# 1606
void*_tmp401=Cyc_Port_var();
void*q;
if((env->gcenv)->porting){
q=Cyc_Port_var();
Cyc_Port_register_const_cvar(env,q,
(_tmp400->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),(_tmp400->tq).loc);}else{
# 1619
q=(_tmp400->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct();}
# 1621
(*_tmp400->name).f1=Cyc_Absyn_Loc_n;
env=Cyc_Port_add_var(env,_tmp400->name,_tmp400->type,q,_tmp401);
Cyc_Port_unifies(_tmp401,Cyc_Port_type_to_ctype(env,_tmp400->type));
if((unsigned int)_tmp400->initializer){
struct Cyc_Absyn_Exp*e=(struct Cyc_Absyn_Exp*)_check_null(_tmp400->initializer);
void*t2=Cyc_Port_gen_initializer(env,_tmp400->type,e);
Cyc_Port_leq(t2,_tmp401);}
# 1629
return env;}_LL2AF:;_LL2B0: {
const char*_tmp5C2;void*_tmp5C1;(_tmp5C1=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((_tmp5C2="non-local decl that isn't a variable",_tag_dyneither(_tmp5C2,sizeof(char),37))),_tag_dyneither(_tmp5C1,sizeof(void*),0)));}_LL2AC:;}
# 1634
static struct _tuple8*Cyc_Port_make_targ(struct _tuple8*arg){
struct _dyneither_ptr*_tmp405;struct Cyc_Absyn_Tqual _tmp406;void*_tmp407;struct _tuple8 _tmp404=*arg;_tmp405=_tmp404.f1;_tmp406=_tmp404.f2;_tmp407=_tmp404.f3;{
struct _tuple8*_tmp5C3;return(_tmp5C3=_cycalloc(sizeof(*_tmp5C3)),((_tmp5C3->f1=0,((_tmp5C3->f2=_tmp406,((_tmp5C3->f3=_tmp407,_tmp5C3)))))));};}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1639
static struct Cyc_Port_Cenv*Cyc_Port_gen_topdecls(struct Cyc_Port_Cenv*env,struct Cyc_List_List*ds);struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct _tuple16{struct _dyneither_ptr*f1;void*f2;void*f3;void*f4;};struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cfield;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1641
static struct Cyc_Port_Cenv*Cyc_Port_gen_topdecl(struct Cyc_Port_Cenv*env,struct Cyc_Absyn_Decl*d){
void*_tmp409=d->r;struct Cyc_Absyn_Vardecl*_tmp40D;struct Cyc_Absyn_Fndecl*_tmp40F;struct Cyc_Absyn_Typedefdecl*_tmp411;struct Cyc_Absyn_Aggrdecl*_tmp413;struct Cyc_Absyn_Enumdecl*_tmp415;_LL2B2: {struct Cyc_Absyn_Porton_d_Absyn_Raw_decl_struct*_tmp40A=(struct Cyc_Absyn_Porton_d_Absyn_Raw_decl_struct*)_tmp409;if(_tmp40A->tag != 14)goto _LL2B4;}_LL2B3:
# 1644
(env->gcenv)->porting=1;
return env;_LL2B4: {struct Cyc_Absyn_Portoff_d_Absyn_Raw_decl_struct*_tmp40B=(struct Cyc_Absyn_Portoff_d_Absyn_Raw_decl_struct*)_tmp409;if(_tmp40B->tag != 15)goto _LL2B6;}_LL2B5:
# 1647
(env->gcenv)->porting=0;
return env;_LL2B6: {struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*_tmp40C=(struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_tmp409;if(_tmp40C->tag != 0)goto _LL2B8;else{_tmp40D=_tmp40C->f1;}}_LL2B7:
# 1650
(*_tmp40D->name).f1=Cyc_Absyn_Loc_n;
# 1654
if(Cyc_Port_declared_var(env,_tmp40D->name)){
void*_tmp417;void*_tmp418;struct _tuple10 _tmp416=Cyc_Port_lookup_var(env,_tmp40D->name);_tmp417=_tmp416.f1;_tmp418=_tmp416.f2;{
void*q2;
if((env->gcenv)->porting  && !Cyc_Port_isfn(env,_tmp40D->name)){
q2=Cyc_Port_var();
Cyc_Port_register_const_cvar(env,q2,
(_tmp40D->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),(_tmp40D->tq).loc);}else{
# 1668
q2=(_tmp40D->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct();}
# 1670
Cyc_Port_unifies(_tmp417,q2);
Cyc_Port_unifies(_tmp418,Cyc_Port_type_to_ctype(env,_tmp40D->type));
if((unsigned int)_tmp40D->initializer){
struct Cyc_Absyn_Exp*e=(struct Cyc_Absyn_Exp*)_check_null(_tmp40D->initializer);
Cyc_Port_leq(Cyc_Port_gen_initializer(env,_tmp40D->type,e),_tmp418);}
# 1676
return env;};}else{
# 1678
return Cyc_Port_gen_localdecl(env,d);}_LL2B8: {struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*_tmp40E=(struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_tmp409;if(_tmp40E->tag != 1)goto _LL2BA;else{_tmp40F=_tmp40E->f1;}}_LL2B9:
# 1684
(*_tmp40F->name).f1=Cyc_Absyn_Loc_n;{
struct _tuple13*predeclared=0;
# 1687
if(Cyc_Port_declared_var(env,_tmp40F->name))
predeclared=Cyc_Port_lookup_full_var(env,_tmp40F->name);{
# 1690
void*_tmp419=_tmp40F->ret_type;
struct Cyc_List_List*_tmp41A=_tmp40F->args;
struct Cyc_List_List*_tmp41B=((struct Cyc_List_List*(*)(struct _tuple8*(*f)(struct _tuple8*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Port_make_targ,_tmp41A);
struct Cyc_Absyn_FnType_Absyn_Type_struct _tmp5C9;struct Cyc_Absyn_FnInfo _tmp5C8;struct Cyc_Absyn_FnType_Absyn_Type_struct*_tmp5C7;struct Cyc_Absyn_FnType_Absyn_Type_struct*_tmp41C=
(_tmp5C7=_cycalloc(sizeof(*_tmp5C7)),((_tmp5C7[0]=((_tmp5C9.tag=9,((_tmp5C9.f1=((_tmp5C8.tvars=0,((_tmp5C8.effect=0,((_tmp5C8.ret_tqual=Cyc_Absyn_empty_tqual(0),((_tmp5C8.ret_typ=_tmp419,((_tmp5C8.args=_tmp41B,((_tmp5C8.c_varargs=0,((_tmp5C8.cyc_varargs=0,((_tmp5C8.rgn_po=0,((_tmp5C8.attributes=0,_tmp5C8)))))))))))))))))),_tmp5C9)))),_tmp5C7)));
# 1697
struct Cyc_Port_Cenv*_tmp41D=Cyc_Port_set_cpos(env,Cyc_Port_FnBody_pos);
void*_tmp41E=Cyc_Port_type_to_ctype(_tmp41D,_tmp419);
struct Cyc_List_List*c_args=0;
struct Cyc_List_List*c_arg_types=0;
{struct Cyc_List_List*_tmp41F=_tmp41A;for(0;(unsigned int)_tmp41F;_tmp41F=_tmp41F->tl){
struct _dyneither_ptr*_tmp421;struct Cyc_Absyn_Tqual _tmp422;void*_tmp423;struct _tuple8 _tmp420=*((struct _tuple8*)_tmp41F->hd);_tmp421=_tmp420.f1;_tmp422=_tmp420.f2;_tmp423=_tmp420.f3;{
# 1705
void*_tmp424=Cyc_Port_type_to_ctype(_tmp41D,_tmp423);
void*tqv;
if((env->gcenv)->porting){
tqv=Cyc_Port_var();
Cyc_Port_register_const_cvar(env,tqv,_tmp422.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),_tmp422.loc);}else{
# 1717
tqv=_tmp422.print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct();}
# 1719
{struct _tuple16*_tmp5CC;struct Cyc_List_List*_tmp5CB;c_args=((_tmp5CB=_cycalloc(sizeof(*_tmp5CB)),((_tmp5CB->hd=((_tmp5CC=_cycalloc(sizeof(*_tmp5CC)),((_tmp5CC->f1=_tmp421,((_tmp5CC->f2=_tmp423,((_tmp5CC->f3=tqv,((_tmp5CC->f4=_tmp424,_tmp5CC)))))))))),((_tmp5CB->tl=c_args,_tmp5CB))))));}{
struct Cyc_List_List*_tmp5CD;c_arg_types=((_tmp5CD=_cycalloc(sizeof(*_tmp5CD)),((_tmp5CD->hd=_tmp424,((_tmp5CD->tl=c_arg_types,_tmp5CD))))));};};}}
# 1722
c_args=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(c_args);
c_arg_types=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(c_arg_types);{
void*_tmp428=Cyc_Port_fn_ct(_tmp41E,c_arg_types);
# 1728
(*_tmp40F->name).f1=Cyc_Absyn_Loc_n;
_tmp41D=Cyc_Port_add_var(_tmp41D,_tmp40F->name,(void*)_tmp41C,Cyc_Port_const_ct(),_tmp428);
Cyc_Port_add_return_type(_tmp41D,_tmp41E);
{struct Cyc_List_List*_tmp429=c_args;for(0;(unsigned int)_tmp429;_tmp429=_tmp429->tl){
struct _dyneither_ptr*_tmp42B;void*_tmp42C;void*_tmp42D;void*_tmp42E;struct _tuple16 _tmp42A=*((struct _tuple16*)_tmp429->hd);_tmp42B=_tmp42A.f1;_tmp42C=_tmp42A.f2;_tmp42D=_tmp42A.f3;_tmp42E=_tmp42A.f4;{
struct _tuple0*_tmp5CE;_tmp41D=Cyc_Port_add_var(_tmp41D,((_tmp5CE=_cycalloc(sizeof(*_tmp5CE)),((_tmp5CE->f1=Cyc_Absyn_Loc_n,((_tmp5CE->f2=_tmp42B,_tmp5CE)))))),_tmp42C,_tmp42D,_tmp42E);};}}
# 1735
Cyc_Port_gen_stmt(_tmp41D,_tmp40F->body);
# 1740
Cyc_Port_generalize(0,_tmp428);{
# 1747
struct Cyc_Dict_Dict counts=((struct Cyc_Dict_Dict(*)(int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))Cyc_Dict_empty)(Cyc_strptrcmp);
Cyc_Port_region_counts(& counts,_tmp428);
# 1751
Cyc_Port_register_rgns(env,counts,1,(void*)_tmp41C,_tmp428);
env=Cyc_Port_add_var(_tmp41D,_tmp40F->name,(void*)_tmp41C,Cyc_Port_const_ct(),_tmp428);
if((unsigned int)predeclared){
# 1760
void*_tmp431;void*_tmp432;void*_tmp433;struct _tuple13 _tmp430=*predeclared;_tmp431=_tmp430.f1;_tmp432=(_tmp430.f2)->f1;_tmp433=(_tmp430.f2)->f2;
Cyc_Port_unifies(_tmp432,Cyc_Port_const_ct());
Cyc_Port_unifies(_tmp433,Cyc_Port_instantiate(_tmp428));
# 1764
Cyc_Port_register_rgns(env,counts,1,_tmp431,_tmp428);}
# 1766
return env;};};};};_LL2BA: {struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*_tmp410=(struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*)_tmp409;if(_tmp410->tag != 9)goto _LL2BC;else{_tmp411=_tmp410->f1;}}_LL2BB: {
# 1772
void*_tmp437=(void*)_check_null(_tmp411->defn);
void*_tmp438=Cyc_Port_type_to_ctype(env,_tmp437);
(*_tmp411->name).f1=Cyc_Absyn_Loc_n;
Cyc_Port_add_typedef(env,_tmp411->name,_tmp437,_tmp438);
return env;}_LL2BC: {struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*_tmp412=(struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*)_tmp409;if(_tmp412->tag != 6)goto _LL2BE;else{_tmp413=_tmp412->f1;}}_LL2BD: {
# 1782
struct _tuple0*_tmp439=_tmp413->name;
(*_tmp413->name).f1=Cyc_Absyn_Loc_n;{
struct _tuple11*previous;
struct Cyc_List_List*curr=0;
switch(_tmp413->kind){case Cyc_Absyn_StructA: _LL2C2:
# 1788
 previous=Cyc_Port_lookup_struct_decl(env,_tmp439);
break;case Cyc_Absyn_UnionA: _LL2C3:
# 1791
 previous=Cyc_Port_lookup_union_decl(env,_tmp439);
break;}
# 1794
if((unsigned int)_tmp413->impl){
struct Cyc_List_List*cf=(*previous).f2;
{struct Cyc_List_List*_tmp43A=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp413->impl))->fields;for(0;(unsigned int)_tmp43A;_tmp43A=_tmp43A->tl){
struct Cyc_Absyn_Aggrfield*_tmp43B=(struct Cyc_Absyn_Aggrfield*)_tmp43A->hd;
void*qv;
if((env->gcenv)->porting){
qv=Cyc_Port_var();
Cyc_Port_register_const_cvar(env,qv,
(_tmp43B->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct(),(_tmp43B->tq).loc);}else{
# 1810
qv=(_tmp43B->tq).print_const?Cyc_Port_const_ct(): Cyc_Port_notconst_ct();}{
# 1812
void*_tmp43C=Cyc_Port_type_to_ctype(env,_tmp43B->type);
if(cf != 0){
void*_tmp43E;void*_tmp43F;struct Cyc_Port_Cfield _tmp43D=*((struct Cyc_Port_Cfield*)cf->hd);_tmp43E=_tmp43D.qual;_tmp43F=_tmp43D.type;
cf=cf->tl;
Cyc_Port_unifies(qv,_tmp43E);
Cyc_Port_unifies(_tmp43C,_tmp43F);}{
# 1819
struct Cyc_Port_Cfield*_tmp5D1;struct Cyc_List_List*_tmp5D0;curr=((_tmp5D0=_cycalloc(sizeof(*_tmp5D0)),((_tmp5D0->hd=((_tmp5D1=_cycalloc(sizeof(*_tmp5D1)),((_tmp5D1->qual=qv,((_tmp5D1->f=_tmp43B->name,((_tmp5D1->type=_tmp43C,_tmp5D1)))))))),((_tmp5D0->tl=curr,_tmp5D0))))));};};}}
# 1821
curr=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(curr);
if((*previous).f2 == 0)
(*previous).f2=curr;}
# 1826
return env;};}_LL2BE: {struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*_tmp414=(struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*)_tmp409;if(_tmp414->tag != 8)goto _LL2C0;else{_tmp415=_tmp414->f1;}}_LL2BF:
# 1831
(*_tmp415->name).f1=Cyc_Absyn_Loc_n;
# 1833
if((unsigned int)_tmp415->fields){
struct Cyc_List_List*_tmp442=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp415->fields))->v;for(0;(unsigned int)_tmp442;_tmp442=_tmp442->tl){
(*((struct Cyc_Absyn_Enumfield*)_tmp442->hd)->name).f1=Cyc_Absyn_Loc_n;{
struct Cyc_Absyn_EnumType_Absyn_Type_struct _tmp5D4;struct Cyc_Absyn_EnumType_Absyn_Type_struct*_tmp5D3;env=Cyc_Port_add_var(env,((struct Cyc_Absyn_Enumfield*)_tmp442->hd)->name,(void*)((_tmp5D3=_cycalloc(sizeof(*_tmp5D3)),((_tmp5D3[0]=((_tmp5D4.tag=13,((_tmp5D4.f1=_tmp415->name,((_tmp5D4.f2=_tmp415,_tmp5D4)))))),_tmp5D3)))),
Cyc_Port_const_ct(),Cyc_Port_arith_ct());};}}
# 1839
return env;_LL2C0:;_LL2C1:
# 1841
 if((env->gcenv)->porting){
const char*_tmp5D7;void*_tmp5D6;(_tmp5D6=0,Cyc_fprintf(Cyc_stderr,((_tmp5D7="Warning: Cyclone declaration found in code to be ported -- skipping.",_tag_dyneither(_tmp5D7,sizeof(char),69))),_tag_dyneither(_tmp5D6,sizeof(void*),0)));}
return env;_LL2B1:;}struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;struct Cyc_Port_Cenv;
# 1848
static struct Cyc_Port_Cenv*Cyc_Port_gen_topdecls(struct Cyc_Port_Cenv*env,struct Cyc_List_List*ds){
for(0;(unsigned int)ds;ds=ds->tl){
env=Cyc_Port_gen_topdecl(env,(struct Cyc_Absyn_Decl*)ds->hd);}
return env;}struct Cyc_Port_Cenv;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;
# 1855
static struct Cyc_List_List*Cyc_Port_gen_edits(struct Cyc_List_List*ds){
# 1857
struct Cyc_Port_Cenv*env=Cyc_Port_gen_topdecls(Cyc_Port_initial_cenv(),ds);
# 1859
struct Cyc_List_List*_tmp447=(env->gcenv)->pointer_edits;
struct Cyc_List_List*_tmp448=(env->gcenv)->qualifier_edits;
struct Cyc_List_List*_tmp449=(env->gcenv)->zeroterm_edits;
struct Cyc_List_List*_tmp44A=(env->gcenv)->edits;
# 1864
{struct Cyc_List_List*_tmp44B=_tmp447;for(0;(unsigned int)_tmp44B;_tmp44B=_tmp44B->tl){
void*_tmp44D;void*_tmp44E;unsigned int _tmp44F;struct _tuple14 _tmp44C=*((struct _tuple14*)_tmp44B->hd);_tmp44D=_tmp44C.f1;_tmp44E=_tmp44C.f2;_tmp44F=_tmp44C.f3;
Cyc_Port_unifies(_tmp44D,_tmp44E);}}
# 1868
{struct Cyc_List_List*_tmp450=_tmp448;for(0;(unsigned int)_tmp450;_tmp450=_tmp450->tl){
void*_tmp452;void*_tmp453;unsigned int _tmp454;struct _tuple14 _tmp451=*((struct _tuple14*)_tmp450->hd);_tmp452=_tmp451.f1;_tmp453=_tmp451.f2;_tmp454=_tmp451.f3;
Cyc_Port_unifies(_tmp452,_tmp453);}}
# 1872
{struct Cyc_List_List*_tmp455=_tmp449;for(0;(unsigned int)_tmp455;_tmp455=_tmp455->tl){
void*_tmp457;void*_tmp458;unsigned int _tmp459;struct _tuple14 _tmp456=*((struct _tuple14*)_tmp455->hd);_tmp457=_tmp456.f1;_tmp458=_tmp456.f2;_tmp459=_tmp456.f3;
Cyc_Port_unifies(_tmp457,_tmp458);}}
# 1878
for(0;(unsigned int)_tmp447;_tmp447=_tmp447->tl){
void*_tmp45B;void*_tmp45C;unsigned int _tmp45D;struct _tuple14 _tmp45A=*((struct _tuple14*)_tmp447->hd);_tmp45B=_tmp45A.f1;_tmp45C=_tmp45A.f2;_tmp45D=_tmp45A.f3;
if(!Cyc_Port_unifies(_tmp45B,_tmp45C) && (int)_tmp45D){
void*_tmp45E=Cyc_Port_compress_ct(_tmp45C);_LL2C6: {struct Cyc_Port_Thin_ct_Port_Ctype_struct*_tmp45F=(struct Cyc_Port_Thin_ct_Port_Ctype_struct*)_tmp45E;if(_tmp45F->tag != 2)goto _LL2C8;}_LL2C7:
# 1883
{struct Cyc_Port_Edit*_tmp5E0;const char*_tmp5DF;const char*_tmp5DE;struct Cyc_List_List*_tmp5DD;_tmp44A=((_tmp5DD=_cycalloc(sizeof(*_tmp5DD)),((_tmp5DD->hd=((_tmp5E0=_cycalloc(sizeof(*_tmp5E0)),((_tmp5E0->loc=_tmp45D,((_tmp5E0->old_string=((_tmp5DF="?",_tag_dyneither(_tmp5DF,sizeof(char),2))),((_tmp5E0->new_string=((_tmp5DE="*",_tag_dyneither(_tmp5DE,sizeof(char),2))),_tmp5E0)))))))),((_tmp5DD->tl=_tmp44A,_tmp5DD))))));}
goto _LL2C5;_LL2C8: {struct Cyc_Port_Fat_ct_Port_Ctype_struct*_tmp460=(struct Cyc_Port_Fat_ct_Port_Ctype_struct*)_tmp45E;if(_tmp460->tag != 3)goto _LL2CA;}_LL2C9:
# 1886
{struct Cyc_Port_Edit*_tmp5E9;const char*_tmp5E8;const char*_tmp5E7;struct Cyc_List_List*_tmp5E6;_tmp44A=((_tmp5E6=_cycalloc(sizeof(*_tmp5E6)),((_tmp5E6->hd=((_tmp5E9=_cycalloc(sizeof(*_tmp5E9)),((_tmp5E9->loc=_tmp45D,((_tmp5E9->old_string=((_tmp5E8="*",_tag_dyneither(_tmp5E8,sizeof(char),2))),((_tmp5E9->new_string=((_tmp5E7="?",_tag_dyneither(_tmp5E7,sizeof(char),2))),_tmp5E9)))))))),((_tmp5E6->tl=_tmp44A,_tmp5E6))))));}
goto _LL2C5;_LL2CA:;_LL2CB:
 goto _LL2C5;_LL2C5:;}}
# 1893
for(0;(unsigned int)_tmp448;_tmp448=_tmp448->tl){
void*_tmp46A;void*_tmp46B;unsigned int _tmp46C;struct _tuple14 _tmp469=*((struct _tuple14*)_tmp448->hd);_tmp46A=_tmp469.f1;_tmp46B=_tmp469.f2;_tmp46C=_tmp469.f3;
if(!Cyc_Port_unifies(_tmp46A,_tmp46B) && (int)_tmp46C){
void*_tmp46D=Cyc_Port_compress_ct(_tmp46B);_LL2CD: {struct Cyc_Port_Notconst_ct_Port_Ctype_struct*_tmp46E=(struct Cyc_Port_Notconst_ct_Port_Ctype_struct*)_tmp46D;if(_tmp46E->tag != 1)goto _LL2CF;}_LL2CE:
# 1898
{struct Cyc_Port_Edit*_tmp5F2;const char*_tmp5F1;const char*_tmp5F0;struct Cyc_List_List*_tmp5EF;_tmp44A=((_tmp5EF=_cycalloc(sizeof(*_tmp5EF)),((_tmp5EF->hd=((_tmp5F2=_cycalloc(sizeof(*_tmp5F2)),((_tmp5F2->loc=_tmp46C,((_tmp5F2->old_string=((_tmp5F1="const ",_tag_dyneither(_tmp5F1,sizeof(char),7))),((_tmp5F2->new_string=((_tmp5F0="",_tag_dyneither(_tmp5F0,sizeof(char),1))),_tmp5F2)))))))),((_tmp5EF->tl=_tmp44A,_tmp5EF))))));}goto _LL2CC;_LL2CF: {struct Cyc_Port_Const_ct_Port_Ctype_struct*_tmp46F=(struct Cyc_Port_Const_ct_Port_Ctype_struct*)_tmp46D;if(_tmp46F->tag != 0)goto _LL2D1;}_LL2D0:
# 1900
{struct Cyc_Port_Edit*_tmp5FB;const char*_tmp5FA;const char*_tmp5F9;struct Cyc_List_List*_tmp5F8;_tmp44A=((_tmp5F8=_cycalloc(sizeof(*_tmp5F8)),((_tmp5F8->hd=((_tmp5FB=_cycalloc(sizeof(*_tmp5FB)),((_tmp5FB->loc=_tmp46C,((_tmp5FB->old_string=((_tmp5FA="",_tag_dyneither(_tmp5FA,sizeof(char),1))),((_tmp5FB->new_string=((_tmp5F9="const ",_tag_dyneither(_tmp5F9,sizeof(char),7))),_tmp5FB)))))))),((_tmp5F8->tl=_tmp44A,_tmp5F8))))));}goto _LL2CC;_LL2D1:;_LL2D2:
 goto _LL2CC;_LL2CC:;}}
# 1906
for(0;(unsigned int)_tmp449;_tmp449=_tmp449->tl){
void*_tmp479;void*_tmp47A;unsigned int _tmp47B;struct _tuple14 _tmp478=*((struct _tuple14*)_tmp449->hd);_tmp479=_tmp478.f1;_tmp47A=_tmp478.f2;_tmp47B=_tmp478.f3;
if(!Cyc_Port_unifies(_tmp479,_tmp47A) && (int)_tmp47B){
void*_tmp47C=Cyc_Port_compress_ct(_tmp47A);_LL2D4: {struct Cyc_Port_Zterm_ct_Port_Ctype_struct*_tmp47D=(struct Cyc_Port_Zterm_ct_Port_Ctype_struct*)_tmp47C;if(_tmp47D->tag != 8)goto _LL2D6;}_LL2D5:
# 1911
{struct Cyc_Port_Edit*_tmp604;const char*_tmp603;const char*_tmp602;struct Cyc_List_List*_tmp601;_tmp44A=((_tmp601=_cycalloc(sizeof(*_tmp601)),((_tmp601->hd=((_tmp604=_cycalloc(sizeof(*_tmp604)),((_tmp604->loc=_tmp47B,((_tmp604->old_string=((_tmp603="NOZEROTERM ",_tag_dyneither(_tmp603,sizeof(char),12))),((_tmp604->new_string=((_tmp602="",_tag_dyneither(_tmp602,sizeof(char),1))),_tmp604)))))))),((_tmp601->tl=_tmp44A,_tmp601))))));}goto _LL2D3;_LL2D6: {struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*_tmp47E=(struct Cyc_Port_Nozterm_ct_Port_Ctype_struct*)_tmp47C;if(_tmp47E->tag != 9)goto _LL2D8;}_LL2D7:
# 1913
{struct Cyc_Port_Edit*_tmp60D;const char*_tmp60C;const char*_tmp60B;struct Cyc_List_List*_tmp60A;_tmp44A=((_tmp60A=_cycalloc(sizeof(*_tmp60A)),((_tmp60A->hd=((_tmp60D=_cycalloc(sizeof(*_tmp60D)),((_tmp60D->loc=_tmp47B,((_tmp60D->old_string=((_tmp60C="ZEROTERM ",_tag_dyneither(_tmp60C,sizeof(char),10))),((_tmp60D->new_string=((_tmp60B="",_tag_dyneither(_tmp60B,sizeof(char),1))),_tmp60D)))))))),((_tmp60A->tl=_tmp44A,_tmp60A))))));}goto _LL2D3;_LL2D8:;_LL2D9:
 goto _LL2D3;_LL2D3:;}}
# 1920
_tmp44A=((struct Cyc_List_List*(*)(int(*cmp)(struct Cyc_Port_Edit*,struct Cyc_Port_Edit*),struct Cyc_List_List*x))Cyc_List_merge_sort)(Cyc_Port_cmp_edit,_tmp44A);
# 1922
while((unsigned int)_tmp44A  && ((struct Cyc_Port_Edit*)_tmp44A->hd)->loc == 0){
# 1926
_tmp44A=_tmp44A->tl;}
# 1928
return _tmp44A;}struct Cyc_Port_Edit;struct Cyc_Port_Edit;
# 1931
static unsigned int Cyc_Port_get_seg(struct Cyc_Port_Edit*e){
return e->loc;}struct Cyc_Port_Edit;struct Cyc_Port_Edit;struct Cyc_Port_Edit;
# 1937
void Cyc_Port_port(struct Cyc_List_List*ds){
struct Cyc_List_List*_tmp487=Cyc_Port_gen_edits(ds);
struct Cyc_List_List*_tmp488=((struct Cyc_List_List*(*)(unsigned int(*f)(struct Cyc_Port_Edit*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Port_get_seg,_tmp487);
Cyc_Position_use_gcc_style_location=0;{
struct Cyc_List_List*_tmp489=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(Cyc_Position_strings_of_segments(_tmp488));
for(0;(unsigned int)_tmp487;(_tmp487=_tmp487->tl,_tmp489=_tmp489->tl)){
unsigned int _tmp48B;struct _dyneither_ptr _tmp48C;struct _dyneither_ptr _tmp48D;struct Cyc_Port_Edit _tmp48A=*((struct Cyc_Port_Edit*)_tmp487->hd);_tmp48B=_tmp48A.loc;_tmp48C=_tmp48A.old_string;_tmp48D=_tmp48A.new_string;{
struct _dyneither_ptr sloc=(struct _dyneither_ptr)*((struct _dyneither_ptr*)((struct Cyc_List_List*)_check_null(_tmp489))->hd);
const char*_tmp613;void*_tmp612[3];struct Cyc_String_pa_PrintArg_struct _tmp611;struct Cyc_String_pa_PrintArg_struct _tmp610;struct Cyc_String_pa_PrintArg_struct _tmp60F;(_tmp60F.tag=0,((_tmp60F.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp48D),((_tmp610.tag=0,((_tmp610.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp48C),((_tmp611.tag=0,((_tmp611.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)sloc),((_tmp612[0]=& _tmp611,((_tmp612[1]=& _tmp610,((_tmp612[2]=& _tmp60F,Cyc_printf(((_tmp613="%s: insert `%s' for `%s'\n",_tag_dyneither(_tmp613,sizeof(char),26))),_tag_dyneither(_tmp612,sizeof(void*),3)))))))))))))))))));};}};}
