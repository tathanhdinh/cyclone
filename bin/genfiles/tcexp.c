#include <setjmp.h>
/* This is a C header file to be used by the output of the Cyclone to
   C translator.  The corresponding definitions are in file lib/runtime_cyc.c */
#ifndef _CYC_INCLUDE_H_
#define _CYC_INCLUDE_H_

/***********************************************************************/
/* Runtime Stack routines (runtime_stack.c).                           */
/***********************************************************************/

/* Need one of these per thread (we don't have threads)
   The runtime maintains a stack that contains either _handler_cons
   structs or _RegionHandle structs.  The tag is 0 for a handler_cons
   and 1 for a region handle.  */
struct _RuntimeStack {
  int tag; /* 0 for an exception handler, 1 for a region handle */
  struct _RuntimeStack *next;
  void (*cleanup)(struct _RuntimeStack *frame);
};

/***********************************************************************/
/* Low-level representations etc.                                      */
/***********************************************************************/

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

/* Regions */
struct _RegionPage
#ifdef CYC_REGION_PROFILE
{ unsigned total_bytes;
  unsigned free_bytes;
  /* MWH: wish we didn't have to include the stuff below ... */
  struct _RegionPage *next;
  char data[1];
}
#endif
; // abstract -- defined in runtime_memory.c

struct _RegionHandle {
  struct _RuntimeStack s;
  struct _RegionPage *curr;
  char               *offset;
  char               *last_plus_one;
  struct _DynRegionHandle *sub_regions;
#ifdef CYC_REGION_PROFILE
  const char         *name;
#else
  unsigned used_bytes;
  unsigned wasted_bytes;
#endif
};

struct _DynRegionFrame {
  struct _RuntimeStack s;
  struct _DynRegionHandle *x;
};

// A dynamic region is just a region handle.  We have the
// wrapper struct for type abstraction reasons.
struct Cyc_Core_DynamicRegion {
  struct _RegionHandle h;
};

extern struct _RegionHandle _new_region(const char *);
extern void * _region_malloc(struct _RegionHandle *, unsigned);
extern void * _region_calloc(struct _RegionHandle *, unsigned t, unsigned n);
extern void   _free_region(struct _RegionHandle *);
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
extern void* _throw_null_fn(const char *filename, unsigned lineno);
extern void* _throw_arraybounds_fn(const char *filename, unsigned lineno);
extern void* _throw_badalloc_fn(const char *filename, unsigned lineno);
extern void* _throw_match_fn(const char *filename, unsigned lineno);
extern void* _throw_fn(void* e, const char *filename, unsigned lineno);
extern void* _rethrow(void* e);
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
#ifdef CYC_ANSI_OUTPUT
#define _INLINE  
#else
#define _INLINE inline
#endif

#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#define _check_null(ptr) \
  ({ void*_cks_null = (void*)(ptr); \
     if (!_cks_null) _throw_null(); \
     _cks_null; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_null(ptr,bound,elt_sz,index)\
   ((char *)ptr) + (elt_sz)*(index))
#define _check_known_subscript_notnull(bound,index) (index)
#define _check_known_subscript_notnullX(bound,index)\
   ((char *)ptr) + (elt_sz)*(index))

#define _zero_arr_plus_char_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_short_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_int_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_float_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_double_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_longdouble_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_voidstar_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  char*_cks_ptr = (char*)(ptr); \
  unsigned _cks_bound = (bound); \
  unsigned _cks_elt_sz = (elt_sz); \
  unsigned _cks_index = (index); \
  if (!_cks_ptr) _throw_null(); \
  if (_cks_index >= _cks_bound) _throw_arraybounds(); \
  (_cks_ptr) + _cks_elt_sz*_cks_index; })
#define _check_known_subscript_notnull(ptr,bound,elt_sz,index) ({ \
  char*_cks_ptr = (char*)(ptr); \
  unsigned _cks_bound = (bound); \
  unsigned _cks_elt_sz = (elt_sz); \
  unsigned _cks_index = (index); \
  if (_cks_index >= _cks_bound) _throw_arraybounds(); \
  (_cks_ptr) + _cks_elt_sz*_cks_index; })

/* Add i to zero-terminated pointer x.  Checks for x being null and
   ensures that x[0..i-1] are not 0. */
char * _zero_arr_plus_char_fn(char *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno);
short * _zero_arr_plus_short_fn(short *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno);
int * _zero_arr_plus_int_fn(int *orig_x, unsigned int orig_sz, int orig_i, const char *filename, unsigned lineno);
float * _zero_arr_plus_float_fn(float *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno);
double * _zero_arr_plus_double_fn(double *orig_x, unsigned int orig_sz, int orig_i,const char *filename, unsigned lineno);
long double * _zero_arr_plus_longdouble_fn(long double *orig_x, unsigned int orig_sz, int orig_i, const char *filename, unsigned lineno);
void * _zero_arr_plus_voidstar_fn(void **orig_x, unsigned int orig_sz, int orig_i,const char *filename,unsigned lineno);
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
int _get_zero_arr_size_char(const char *orig_x, unsigned int orig_offset);
int _get_zero_arr_size_short(const short *orig_x, unsigned int orig_offset);
int _get_zero_arr_size_int(const int *orig_x, unsigned int orig_offset);
int _get_zero_arr_size_float(const float *orig_x, unsigned int orig_offset);
int _get_zero_arr_size_double(const double *orig_x, unsigned int orig_offset);
int _get_zero_arr_size_longdouble(const long double *orig_x, unsigned int orig_offset);
int _get_zero_arr_size_voidstar(const void **orig_x, unsigned int orig_offset);

/* Does in-place addition of a zero-terminated pointer (x += e and ++x).  
   Note that this expands to call _zero_arr_plus_<type>_fn. */
char * _zero_arr_inplace_plus_char_fn(char **x, int orig_i,const char *filename,unsigned lineno);
short * _zero_arr_inplace_plus_short_fn(short **x, int orig_i,const char *filename,unsigned lineno);
int * _zero_arr_inplace_plus_int(int **x, int orig_i,const char *filename,unsigned lineno);
float * _zero_arr_inplace_plus_float_fn(float **x, int orig_i,const char *filename,unsigned lineno);
double * _zero_arr_inplace_plus_double_fn(double **x, int orig_i,const char *filename,unsigned lineno);
long double * _zero_arr_inplace_plus_longdouble_fn(long double **x, int orig_i,const char *filename,unsigned lineno);
void * _zero_arr_inplace_plus_voidstar_fn(void ***x, int orig_i,const char *filename,unsigned lineno);
#define _zero_arr_inplace_plus_char(x,i) \
  _zero_arr_inplace_plus_char_fn((char **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_short(x,i) \
  _zero_arr_inplace_plus_short_fn((short **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_int(x,i) \
  _zero_arr_inplace_plus_int_fn((int **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_float(x,i) \
  _zero_arr_inplace_plus_float_fn((float **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_double(x,i) \
  _zero_arr_inplace_plus_double_fn((double **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_longdouble(x,i) \
  _zero_arr_inplace_plus_longdouble_fn((long double **)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_voidstar(x,i) \
  _zero_arr_inplace_plus_voidstar_fn((void ***)(x),i,__FILE__,__LINE__)

/* Does in-place increment of a zero-terminated pointer (e.g., x++). */
char * _zero_arr_inplace_plus_post_char_fn(char **x, int orig_i,const char *filename,unsigned lineno);
short * _zero_arr_inplace_plus_post_short_fn(short **x, int orig_i,const char *filename,unsigned lineno);
int * _zero_arr_inplace_plus_post_int_fn(int **x, int orig_i,const char *filename, unsigned lineno);
float * _zero_arr_inplace_plus_post_float_fn(float **x, int orig_i,const char *filename, unsigned lineno);
double * _zero_arr_inplace_plus_post_double_fn(double **x, int orig_i,const char *filename,unsigned lineno);
long double * _zero_arr_inplace_plus_post_longdouble_fn(long double **x, int orig_i,const char *filename,unsigned lineno);
void ** _zero_arr_inplace_plus_post_voidstar_fn(void ***x, int orig_i,const char *filename,unsigned lineno);
#define _zero_arr_inplace_plus_post_char(x,i) \
  _zero_arr_inplace_plus_post_char_fn((char **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_short(x,i) \
  _zero_arr_inplace_plus_post_short_fn((short **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_int(x,i) \
  _zero_arr_inplace_plus_post_int_fn((int **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_float(x,i) \
  _zero_arr_inplace_plus_post_float_fn((float **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_double(x,i) \
  _zero_arr_inplace_plus_post_double_fn((double **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_longdouble(x,i) \
  _zero_arr_inplace_plus_post_longdouble_fn((long double **)(x),(i),__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_voidstar(x,i) \
  _zero_arr_inplace_plus_post_voidstar_fn((void***)(x),(i),__FILE__,__LINE__)

/* functions for dealing with dynamically sized pointers */
#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_dyneither_subscript(arr,elt_sz,index) ({ \
  struct _dyneither_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  _cus_ans; })
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

#define _tag_dyneither(tcurr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _tag_arr_ans; \
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr); \
  /* JGM: if we're tagging NULL, ignore num_elts */ \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base ? (_tag_arr_ans.base + (elt_sz) * (num_elts)) : 0; \
  _tag_arr_ans; })

#ifdef NO_CYC_BOUNDS_CHECKS
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ((arr).curr)
#else
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if ((_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one) &&\
      _curr != (unsigned char *)0) \
    _throw_arraybounds(); \
  _curr; })
#endif

#define _get_dyneither_size(arr,elt_sz) \
  ({struct _dyneither_ptr _get_arr_size_temp = (arr); \
    unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr; \
    unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one; \
    (_get_arr_size_curr < _get_arr_size_temp.base || \
     _get_arr_size_curr >= _get_arr_size_last) ? 0 : \
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));})

#define _dyneither_ptr_plus(arr,elt_sz,change) ({ \
  struct _dyneither_ptr _ans = (arr); \
  _ans.curr += ((int)(elt_sz))*(change); \
  _ans; })

#define _dyneither_ptr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  *_arr_ptr; })

#define _dyneither_ptr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  struct _dyneither_ptr _ans = *_arr_ptr; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })

/* Decrease the upper bound on a fat pointer by numelts where sz is
   the size of the pointer's type.  Note that this can't be a macro
   if we're to get initializers right. */
static struct
 _dyneither_ptr _dyneither_ptr_decrease_size(struct _dyneither_ptr x,
                                            unsigned int sz,
                                            unsigned int numelts) {
  unsigned delta = sz * numelts;
  /* Don't let someone decrease the size so much that it wraps around.
   * This is crucial to get NULL right. */
  if (x.last_plus_one - x.base >= delta)
    x.last_plus_one -= delta;
  else x.last_plus_one = x.base;
  return x; 
}

/* Allocation */
extern void* GC_malloc(int);
extern void* GC_malloc_atomic(int);
extern void* GC_calloc(unsigned,unsigned);
extern void* GC_calloc_atomic(unsigned,unsigned);
/* bound the allocation size to be less than MAX_ALLOC_SIZE,
   which is defined in runtime_memory.c
*/
extern void* _bounded_GC_malloc(int,const char *file,int lineno);
extern void* _bounded_GC_malloc_atomic(int,const char *file,int lineno);
extern void* _bounded_GC_calloc(unsigned n, unsigned s,
                                const char *file,int lineno);
extern void* _bounded_GC_calloc_atomic(unsigned n, unsigned s,
                                       const char *file,
                                       int lineno);
/* FIX?  Not sure if we want to pass filename and lineno in here... */
#ifndef CYC_REGION_PROFILE
#define _cycalloc(n) _bounded_GC_malloc(n,__FILE__,__LINE__)
#define _cycalloc_atomic(n) _bounded_GC_malloc_atomic(n,__FILE__,__LINE__)
#define _cyccalloc(n,s) _bounded_GC_calloc(n,s,__FILE__,__LINE__)
#define _cyccalloc_atomic(n,s) _bounded_GC_calloc_atomic(n,s,__FILE__,__LINE__)
#endif

#define MAX_MALLOC_SIZE (1 << 28)
static _INLINE unsigned int _check_times(unsigned x, unsigned y) {
  unsigned long long whole_ans = 
    ((unsigned long long) x)*((unsigned long long)y);
  unsigned word_ans = (unsigned)whole_ans;
  if(word_ans < whole_ans || word_ans > MAX_MALLOC_SIZE)
    _throw_badalloc();
  return word_ans;
}

#define _CYC_MAX_REGION_CONST 2
#define _CYC_MIN_ALIGNMENT (sizeof(double))

#ifdef CYC_REGION_PROFILE
extern int rgn_total_bytes;
#endif

static _INLINE void *_fast_region_malloc(struct _RegionHandle *r, unsigned orig_s) {  
  if (r > (struct _RegionHandle *)_CYC_MAX_REGION_CONST && r->curr != 0) { 
#ifdef CYC_NOALIGN
    unsigned s =  orig_s;
#else
    unsigned s =  (orig_s + _CYC_MIN_ALIGNMENT - 1) & (~(_CYC_MIN_ALIGNMENT -1)); 
#endif
    char *result; 
    result = r->offset; 
    if (s <= (r->last_plus_one - result)) {
      r->offset = result + s; 
#ifdef CYC_REGION_PROFILE
    r->curr->free_bytes = r->curr->free_bytes - s;
    rgn_total_bytes += s;
#endif
      return result;
    }
  } 
  return _region_malloc(r,orig_s); 
}

#if defined(CYC_REGION_PROFILE) 
extern void* _profile_GC_malloc(int,const char *file,const char *func,
                                int lineno);
extern void* _profile_GC_malloc_atomic(int,const char *file,
                                       const char *func,int lineno);
extern void* _profile_GC_calloc(unsigned n, unsigned s,
                                const char *file, const char *func, int lineno);
extern void* _profile_GC_calloc_atomic(unsigned n, unsigned s,
                                       const char *file, const char *func,
                                       int lineno);
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
#define _cyccalloc(n,s) _profile_GC_calloc(n,s,__FILE__,__FUNCTION__,__LINE__)
#define _cyccalloc_atomic(n,s) _profile_GC_calloc_atomic(n,s,__FILE__,__FUNCTION__,__LINE__)
#endif
#endif
 struct Cyc___cycFILE;struct Cyc_String_pa_PrintArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Int_pa_PrintArg_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_PrintArg_struct{int tag;double f1;};struct Cyc_LongDouble_pa_PrintArg_struct{int tag;long double f1;};struct Cyc_ShortPtr_pa_PrintArg_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_PrintArg_struct{int tag;unsigned long*f1;};
# 73 "cycboot.h"
struct _dyneither_ptr Cyc_aprintf(struct _dyneither_ptr,struct _dyneither_ptr);struct Cyc_ShortPtr_sa_ScanfArg_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_ScanfArg_struct{int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_ScanfArg_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_ScanfArg_struct{int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_DoublePtr_sa_ScanfArg_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_ScanfArg_struct{int tag;float*f1;};struct Cyc_CharPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};
# 232 "cycboot.h"
struct _dyneither_ptr Cyc_vrprintf(struct _RegionHandle*,struct _dyneither_ptr,struct _dyneither_ptr);extern char Cyc_FileCloseError[15U];struct Cyc_FileCloseError_exn_struct{char*tag;};extern char Cyc_FileOpenError[14U];struct Cyc_FileOpenError_exn_struct{char*tag;struct _dyneither_ptr f1;};struct Cyc_Core_Opt{void*v;};struct _tuple0{void*f1;void*f2;};
# 113 "core.h"
void*Cyc_Core_snd(struct _tuple0*);extern char Cyc_Core_Invalid_argument[17U];struct Cyc_Core_Invalid_argument_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Failure[8U];struct Cyc_Core_Failure_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Impossible[11U];struct Cyc_Core_Impossible_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Not_found[10U];struct Cyc_Core_Not_found_exn_struct{char*tag;};extern char Cyc_Core_Unreachable[12U];struct Cyc_Core_Unreachable_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 167
extern struct _RegionHandle*Cyc_Core_heap_region;
# 170
extern struct _RegionHandle*Cyc_Core_unique_region;struct Cyc_Core_DynamicRegion;struct Cyc_Core_NewDynamicRegion{struct Cyc_Core_DynamicRegion*key;};struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};
# 61 "list.h"
int Cyc_List_length(struct Cyc_List_List*x);
# 76
struct Cyc_List_List*Cyc_List_map(void*(*f)(void*),struct Cyc_List_List*x);
# 83
struct Cyc_List_List*Cyc_List_map_c(void*(*f)(void*,void*),void*env,struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[14U];struct Cyc_List_List_mismatch_exn_struct{char*tag;};
# 178
struct Cyc_List_List*Cyc_List_imp_rev(struct Cyc_List_List*x);
# 184
struct Cyc_List_List*Cyc_List_append(struct Cyc_List_List*x,struct Cyc_List_List*y);extern char Cyc_List_Nth[4U];struct Cyc_List_Nth_exn_struct{char*tag;};
# 242
void*Cyc_List_nth(struct Cyc_List_List*x,int n);
# 246
struct Cyc_List_List*Cyc_List_nth_tail(struct Cyc_List_List*x,int i);
# 265
void*Cyc_List_find_c(void*(*pred)(void*,void*),void*env,struct Cyc_List_List*x);
# 270
struct Cyc_List_List*Cyc_List_zip(struct Cyc_List_List*x,struct Cyc_List_List*y);
# 38 "string.h"
unsigned long Cyc_strlen(struct _dyneither_ptr s);
# 49 "string.h"
int Cyc_strcmp(struct _dyneither_ptr s1,struct _dyneither_ptr s2);struct Cyc_PP_Ppstate;struct Cyc_PP_Out;struct Cyc_PP_Doc;struct Cyc_Position_Error;struct Cyc_Relations_Reln;struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_C_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{int tag;int val;};union Cyc_Absyn_Nmspace{struct _union_Nmspace_Rel_n Rel_n;struct _union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_C_n C_n;struct _union_Nmspace_Loc_n Loc_n;};
# 96 "absyn.h"
union Cyc_Absyn_Nmspace Cyc_Absyn_Loc_n;
union Cyc_Absyn_Nmspace Cyc_Absyn_Rel_n(struct Cyc_List_List*);
# 99
union Cyc_Absyn_Nmspace Cyc_Absyn_Abs_n(struct Cyc_List_List*ns,int C_scope);struct _tuple1{union Cyc_Absyn_Nmspace f1;struct _dyneither_ptr*f2;};
# 159
enum Cyc_Absyn_Scope{Cyc_Absyn_Static  = 0U,Cyc_Absyn_Abstract  = 1U,Cyc_Absyn_Public  = 2U,Cyc_Absyn_Extern  = 3U,Cyc_Absyn_ExternC  = 4U,Cyc_Absyn_Register  = 5U};struct Cyc_Absyn_Tqual{int print_const: 1;int q_volatile: 1;int q_restrict: 1;int real_const: 1;unsigned int loc;};
# 180
enum Cyc_Absyn_Size_of{Cyc_Absyn_Char_sz  = 0U,Cyc_Absyn_Short_sz  = 1U,Cyc_Absyn_Int_sz  = 2U,Cyc_Absyn_Long_sz  = 3U,Cyc_Absyn_LongLong_sz  = 4U};
# 185
enum Cyc_Absyn_AliasQual{Cyc_Absyn_Aliasable  = 0U,Cyc_Absyn_Unique  = 1U,Cyc_Absyn_Top  = 2U};
# 191
enum Cyc_Absyn_KindQual{Cyc_Absyn_AnyKind  = 0U,Cyc_Absyn_MemKind  = 1U,Cyc_Absyn_BoxKind  = 2U,Cyc_Absyn_RgnKind  = 3U,Cyc_Absyn_EffKind  = 4U,Cyc_Absyn_IntKind  = 5U,Cyc_Absyn_BoolKind  = 6U,Cyc_Absyn_PtrBndKind  = 7U};struct Cyc_Absyn_Kind{enum Cyc_Absyn_KindQual kind;enum Cyc_Absyn_AliasQual aliasqual;};
# 213
enum Cyc_Absyn_Sign{Cyc_Absyn_Signed  = 0U,Cyc_Absyn_Unsigned  = 1U,Cyc_Absyn_None  = 2U};
# 215
enum Cyc_Absyn_AggrKind{Cyc_Absyn_StructA  = 0U,Cyc_Absyn_UnionA  = 1U};struct Cyc_Absyn_Eq_kb_Absyn_KindBound_struct{int tag;struct Cyc_Absyn_Kind*f1;};struct Cyc_Absyn_Unknown_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_Absyn_KindBound_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_Tvar{struct _dyneither_ptr*name;int identity;void*kind;};struct Cyc_Absyn_PtrLoc{unsigned int ptr_loc;unsigned int rgn_loc;unsigned int zt_loc;};struct Cyc_Absyn_PtrAtts{void*rgn;void*nullable;void*bounds;void*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;};struct Cyc_Absyn_PtrInfo{void*elt_type;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};struct Cyc_Absyn_VarargInfo{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;};struct Cyc_Absyn_UnknownDatatypeInfo{struct _tuple1*name;int is_extensible;};struct _union_DatatypeInfo_UnknownDatatype{int tag;struct Cyc_Absyn_UnknownDatatypeInfo val;};struct _union_DatatypeInfo_KnownDatatype{int tag;struct Cyc_Absyn_Datatypedecl**val;};union Cyc_Absyn_DatatypeInfo{struct _union_DatatypeInfo_UnknownDatatype UnknownDatatype;struct _union_DatatypeInfo_KnownDatatype KnownDatatype;};struct Cyc_Absyn_UnknownDatatypeFieldInfo{struct _tuple1*datatype_name;struct _tuple1*field_name;int is_extensible;};struct _union_DatatypeFieldInfo_UnknownDatatypefield{int tag;struct Cyc_Absyn_UnknownDatatypeFieldInfo val;};struct _tuple2{struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;};struct _union_DatatypeFieldInfo_KnownDatatypefield{int tag;struct _tuple2 val;};union Cyc_Absyn_DatatypeFieldInfo{struct _union_DatatypeFieldInfo_UnknownDatatypefield UnknownDatatypefield;struct _union_DatatypeFieldInfo_KnownDatatypefield KnownDatatypefield;};
# 317
union Cyc_Absyn_DatatypeFieldInfo Cyc_Absyn_KnownDatatypefield(struct Cyc_Absyn_Datatypedecl*,struct Cyc_Absyn_Datatypefield*);struct _tuple3{enum Cyc_Absyn_AggrKind f1;struct _tuple1*f2;struct Cyc_Core_Opt*f3;};struct _union_AggrInfo_UnknownAggr{int tag;struct _tuple3 val;};struct _union_AggrInfo_KnownAggr{int tag;struct Cyc_Absyn_Aggrdecl**val;};union Cyc_Absyn_AggrInfo{struct _union_AggrInfo_UnknownAggr UnknownAggr;struct _union_AggrInfo_KnownAggr KnownAggr;};
# 325
union Cyc_Absyn_AggrInfo Cyc_Absyn_KnownAggr(struct Cyc_Absyn_Aggrdecl**);struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;void*zero_term;unsigned int zt_loc;};struct Cyc_Absyn_Aggr_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Enum_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Datatype_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_TypeDecl{void*r;unsigned int loc;};struct Cyc_Absyn_VoidCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_IntCon_Absyn_TyCon_struct{int tag;enum Cyc_Absyn_Sign f1;enum Cyc_Absyn_Size_of f2;};struct Cyc_Absyn_FloatCon_Absyn_TyCon_struct{int tag;int f1;};struct Cyc_Absyn_RgnHandleCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_TagCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_HeapCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_UniqueCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_RefCntCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_AccessCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_JoinCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_RgnsCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_TrueCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_FalseCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_ThinCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_FatCon_Absyn_TyCon_struct{int tag;};struct Cyc_Absyn_EnumCon_Absyn_TyCon_struct{int tag;struct _tuple1*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumCon_Absyn_TyCon_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_BuiltinCon_Absyn_TyCon_struct{int tag;struct _dyneither_ptr f1;struct Cyc_Absyn_Kind*f2;};struct Cyc_Absyn_DatatypeCon_Absyn_TyCon_struct{int tag;union Cyc_Absyn_DatatypeInfo f1;};struct Cyc_Absyn_DatatypeFieldCon_Absyn_TyCon_struct{int tag;union Cyc_Absyn_DatatypeFieldInfo f1;};struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct{int tag;union Cyc_Absyn_AggrInfo f1;};struct Cyc_Absyn_AppType_Absyn_Type_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Evar_Absyn_Type_struct{int tag;struct Cyc_Core_Opt*f1;void*f2;int f3;struct Cyc_Core_Opt*f4;};struct Cyc_Absyn_VarType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_PointerType_Absyn_Type_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_ArrayType_Absyn_Type_struct{int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_Absyn_Type_struct{int tag;struct Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_Absyn_Type_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct{int tag;enum Cyc_Absyn_AggrKind f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_TypedefType_Absyn_Type_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void*f4;};struct Cyc_Absyn_ValueofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct{int tag;struct Cyc_Absyn_TypeDecl*f1;void**f2;};struct Cyc_Absyn_TypeofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_NoTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;};struct Cyc_Absyn_WithTypes_Absyn_Funcparams_struct{int tag;struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;void*f4;struct Cyc_List_List*f5;struct Cyc_Absyn_Exp*f6;struct Cyc_Absyn_Exp*f7;};
# 427 "absyn.h"
enum Cyc_Absyn_Format_Type{Cyc_Absyn_Printf_ft  = 0U,Cyc_Absyn_Scanf_ft  = 1U};struct Cyc_Absyn_Regparm_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Stdcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Cdecl_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Fastcall_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Noreturn_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Const_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Aligned_att_Absyn_Attribute_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Packed_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Section_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Nocommon_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Shared_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Unused_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Weak_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllimport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Dllexport_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_instrument_function_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Constructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Destructor_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_No_check_memory_usage_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Format_att_Absyn_Attribute_struct{int tag;enum Cyc_Absyn_Format_Type f1;int f2;int f3;};struct Cyc_Absyn_Initializes_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noliveunique_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Noconsume_att_Absyn_Attribute_struct{int tag;int f1;};struct Cyc_Absyn_Pure_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Mode_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Alias_att_Absyn_Attribute_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Always_inline_att_Absyn_Attribute_struct{int tag;};struct Cyc_Absyn_Carray_mod_Absyn_Type_modifier_struct{int tag;void*f1;unsigned int f2;};struct Cyc_Absyn_ConstArray_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;unsigned int f3;};struct Cyc_Absyn_Pointer_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_Absyn_Type_modifier_struct{int tag;void*f1;};struct Cyc_Absyn_TypeParams_mod_Absyn_Type_modifier_struct{int tag;struct Cyc_List_List*f1;unsigned int f2;int f3;};struct Cyc_Absyn_Attributes_mod_Absyn_Type_modifier_struct{int tag;unsigned int f1;struct Cyc_List_List*f2;};struct _union_Cnst_Null_c{int tag;int val;};struct _tuple4{enum Cyc_Absyn_Sign f1;char f2;};struct _union_Cnst_Char_c{int tag;struct _tuple4 val;};struct _union_Cnst_Wchar_c{int tag;struct _dyneither_ptr val;};struct _tuple5{enum Cyc_Absyn_Sign f1;short f2;};struct _union_Cnst_Short_c{int tag;struct _tuple5 val;};struct _tuple6{enum Cyc_Absyn_Sign f1;int f2;};struct _union_Cnst_Int_c{int tag;struct _tuple6 val;};struct _tuple7{enum Cyc_Absyn_Sign f1;long long f2;};struct _union_Cnst_LongLong_c{int tag;struct _tuple7 val;};struct _tuple8{struct _dyneither_ptr f1;int f2;};struct _union_Cnst_Float_c{int tag;struct _tuple8 val;};struct _union_Cnst_String_c{int tag;struct _dyneither_ptr val;};struct _union_Cnst_Wstring_c{int tag;struct _dyneither_ptr val;};union Cyc_Absyn_Cnst{struct _union_Cnst_Null_c Null_c;struct _union_Cnst_Char_c Char_c;struct _union_Cnst_Wchar_c Wchar_c;struct _union_Cnst_Short_c Short_c;struct _union_Cnst_Int_c Int_c;struct _union_Cnst_LongLong_c LongLong_c;struct _union_Cnst_Float_c Float_c;struct _union_Cnst_String_c String_c;struct _union_Cnst_Wstring_c Wstring_c;};
# 507
union Cyc_Absyn_Cnst Cyc_Absyn_Char_c(enum Cyc_Absyn_Sign,char);
# 509
union Cyc_Absyn_Cnst Cyc_Absyn_Short_c(enum Cyc_Absyn_Sign,short);
union Cyc_Absyn_Cnst Cyc_Absyn_Int_c(enum Cyc_Absyn_Sign,int);
# 517
enum Cyc_Absyn_Primop{Cyc_Absyn_Plus  = 0U,Cyc_Absyn_Times  = 1U,Cyc_Absyn_Minus  = 2U,Cyc_Absyn_Div  = 3U,Cyc_Absyn_Mod  = 4U,Cyc_Absyn_Eq  = 5U,Cyc_Absyn_Neq  = 6U,Cyc_Absyn_Gt  = 7U,Cyc_Absyn_Lt  = 8U,Cyc_Absyn_Gte  = 9U,Cyc_Absyn_Lte  = 10U,Cyc_Absyn_Not  = 11U,Cyc_Absyn_Bitnot  = 12U,Cyc_Absyn_Bitand  = 13U,Cyc_Absyn_Bitor  = 14U,Cyc_Absyn_Bitxor  = 15U,Cyc_Absyn_Bitlshift  = 16U,Cyc_Absyn_Bitlrshift  = 17U,Cyc_Absyn_Bitarshift  = 18U,Cyc_Absyn_Numelts  = 19U};
# 524
enum Cyc_Absyn_Incrementor{Cyc_Absyn_PreInc  = 0U,Cyc_Absyn_PostInc  = 1U,Cyc_Absyn_PreDec  = 2U,Cyc_Absyn_PostDec  = 3U};struct Cyc_Absyn_VarargCallInfo{int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_TupleIndex_Absyn_OffsetofField_struct{int tag;unsigned int f1;};
# 542
enum Cyc_Absyn_Coercion{Cyc_Absyn_Unknown_coercion  = 0U,Cyc_Absyn_No_coercion  = 1U,Cyc_Absyn_Null_to_NonNull  = 2U,Cyc_Absyn_Other_coercion  = 3U};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;int inline_call;};struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct{int tag;union Cyc_Absyn_Cnst f1;};struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Pragma_e_Absyn_Raw_exp_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct{int tag;enum Cyc_Absyn_Primop f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;enum Cyc_Absyn_Incrementor f2;};struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;int f4;};struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;int f2;};struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;enum Cyc_Absyn_Coercion f4;};struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftype_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;int f3;int f4;};struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct _tuple9{struct _dyneither_ptr*f1;struct Cyc_Absyn_Tqual f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct{int tag;struct _tuple9*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;int f4;};struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;int f3;};struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*f4;};struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Datatypedecl*f2;struct Cyc_Absyn_Datatypefield*f3;};struct Cyc_Absyn_Enum_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _dyneither_ptr*f2;};struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Asm_e_Absyn_Raw_exp_struct{int tag;int f1;struct _dyneither_ptr f2;};struct Cyc_Absyn_Extension_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Exp{void*topt;void*r;unsigned int loc;void*annot;};struct Cyc_Absyn_Skip_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_IfThenElse_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};struct _tuple10{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_While_s_Absyn_Raw_stmt_struct{int tag;struct _tuple10 f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Continue_s_Absyn_Raw_stmt_struct{int tag;};struct Cyc_Absyn_Goto_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_For_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple10 f2;struct _tuple10 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Fallthru_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Label_s_Absyn_Raw_stmt_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple10 f2;};struct Cyc_Absyn_TryCatch_s_Absyn_Raw_stmt_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*f2;void*f3;};struct Cyc_Absyn_Stmt{void*r;unsigned int loc;void*annot;};struct Cyc_Absyn_Wild_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Var_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_AliasVar_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Reference_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_Absyn_Raw_pat_struct{int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_Absyn_Raw_pat_struct{int tag;union Cyc_Absyn_AggrInfo*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Datatype_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;struct Cyc_Absyn_Datatypefield*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Null_p_Absyn_Raw_pat_struct{int tag;};struct Cyc_Absyn_Int_p_Absyn_Raw_pat_struct{int tag;enum Cyc_Absyn_Sign f1;int f2;};struct Cyc_Absyn_Char_p_Absyn_Raw_pat_struct{int tag;char f1;};struct Cyc_Absyn_Float_p_Absyn_Raw_pat_struct{int tag;struct _dyneither_ptr f1;int f2;};struct Cyc_Absyn_Enum_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_AnonEnum_p_Absyn_Raw_pat_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Absyn_UnknownId_p_Absyn_Raw_pat_struct{int tag;struct _tuple1*f1;};struct Cyc_Absyn_UnknownCall_p_Absyn_Raw_pat_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_Absyn_Raw_pat_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Pat{void*r;void*topt;unsigned int loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*body;unsigned int loc;};struct Cyc_Absyn_Unresolved_b_Absyn_Binding_struct{int tag;struct _tuple1*f1;};struct Cyc_Absyn_Global_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Param_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Local_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_Absyn_Binding_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{enum Cyc_Absyn_Scope sc;struct _tuple1*name;unsigned int varloc;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;void*rgn;struct Cyc_List_List*attributes;int escapes;};struct Cyc_Absyn_Fndecl{enum Cyc_Absyn_Scope sc;int is_inline;struct _tuple1*name;struct Cyc_List_List*tvs;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;void*cached_type;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_List_List*requires_relns;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_List_List*ensures_relns;};struct Cyc_Absyn_Aggrfield{struct _dyneither_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct Cyc_List_List*rgn_po;struct Cyc_List_List*fields;int tagged;};struct Cyc_Absyn_Aggrdecl{enum Cyc_Absyn_AggrKind kind;enum Cyc_Absyn_Scope sc;struct _tuple1*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*impl;struct Cyc_List_List*attributes;int expected_mem_kind;};struct Cyc_Absyn_Datatypefield{struct _tuple1*name;struct Cyc_List_List*typs;unsigned int loc;enum Cyc_Absyn_Scope sc;};struct Cyc_Absyn_Datatypedecl{enum Cyc_Absyn_Scope sc;struct _tuple1*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int is_extensible;};struct Cyc_Absyn_Enumfield{struct _tuple1*name;struct Cyc_Absyn_Exp*tag;unsigned int loc;};struct Cyc_Absyn_Enumdecl{enum Cyc_Absyn_Scope sc;struct _tuple1*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct _tuple1*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*kind;void*defn;struct Cyc_List_List*atts;int extern_c;};struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Let_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;void*f4;};struct Cyc_Absyn_Letv_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Region_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Datatype_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_Absyn_Raw_decl_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_Absyn_Raw_decl_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_ExternCinclude_d_Absyn_Raw_decl_struct{int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Porton_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Portoff_d_Absyn_Raw_decl_struct{int tag;};struct Cyc_Absyn_Decl{void*r;unsigned int loc;};struct Cyc_Absyn_ArrayElement_Absyn_Designator_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_Absyn_Designator_struct{int tag;struct _dyneither_ptr*f1;};extern char Cyc_Absyn_EmptyAnnot[11U];struct Cyc_Absyn_EmptyAnnot_Absyn_AbsynAnnot_struct{char*tag;};
# 907 "absyn.h"
struct Cyc_Absyn_Tqual Cyc_Absyn_const_tqual(unsigned int);
# 909
struct Cyc_Absyn_Tqual Cyc_Absyn_empty_tqual(unsigned int);
# 923
void*Cyc_Absyn_new_evar(struct Cyc_Core_Opt*k,struct Cyc_Core_Opt*tenv);
# 925
void*Cyc_Absyn_wildtyp(struct Cyc_Core_Opt*);
# 928
extern void*Cyc_Absyn_char_type;extern void*Cyc_Absyn_uchar_type;extern void*Cyc_Absyn_ushort_type;extern void*Cyc_Absyn_uint_type;extern void*Cyc_Absyn_ulonglong_type;
# 930
extern void*Cyc_Absyn_schar_type;extern void*Cyc_Absyn_sshort_type;extern void*Cyc_Absyn_sint_type;extern void*Cyc_Absyn_slonglong_type;
# 932
extern void*Cyc_Absyn_double_type;void*Cyc_Absyn_wchar_type();
void*Cyc_Absyn_gen_float_type(unsigned int i);
# 935
extern void*Cyc_Absyn_heap_rgn_type;extern void*Cyc_Absyn_unique_rgn_type;
# 939
extern void*Cyc_Absyn_true_type;extern void*Cyc_Absyn_false_type;
# 941
extern void*Cyc_Absyn_void_type;void*Cyc_Absyn_tag_type(void*);void*Cyc_Absyn_rgn_handle_type(void*);void*Cyc_Absyn_enum_type(struct _tuple1*n,struct Cyc_Absyn_Enumdecl*d);
# 958
void*Cyc_Absyn_exn_type();
# 966
extern void*Cyc_Absyn_fat_bound_type;
# 968
void*Cyc_Absyn_thin_bounds_exp(struct Cyc_Absyn_Exp*);
# 970
void*Cyc_Absyn_bounds_one();
# 977
void*Cyc_Absyn_atb_type(void*t,void*rgn,struct Cyc_Absyn_Tqual tq,void*b,void*zero_term);
# 980
void*Cyc_Absyn_star_type(void*t,void*rgn,struct Cyc_Absyn_Tqual tq,void*zero_term);
# 983
void*Cyc_Absyn_at_type(void*t,void*rgn,struct Cyc_Absyn_Tqual tq,void*zero_term);
# 988
void*Cyc_Absyn_fatptr_type(void*t,void*rgn,struct Cyc_Absyn_Tqual tq,void*zeroterm);
# 996
void*Cyc_Absyn_array_type(void*elt_type,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Exp*num_elts,void*zero_term,unsigned int ztloc);
# 1000
void*Cyc_Absyn_datatype_field_type(union Cyc_Absyn_DatatypeFieldInfo,struct Cyc_List_List*args);
void*Cyc_Absyn_aggr_type(union Cyc_Absyn_AggrInfo,struct Cyc_List_List*args);
# 1004
struct Cyc_Absyn_Exp*Cyc_Absyn_new_exp(void*,unsigned int);
# 1006
struct Cyc_Absyn_Exp*Cyc_Absyn_copy_exp(struct Cyc_Absyn_Exp*);
struct Cyc_Absyn_Exp*Cyc_Absyn_const_exp(union Cyc_Absyn_Cnst,unsigned int);
# 1014
struct Cyc_Absyn_Exp*Cyc_Absyn_uint_exp(unsigned int,unsigned int);
# 1028
struct Cyc_Absyn_Exp*Cyc_Absyn_add_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,unsigned int);
# 1054
struct Cyc_Absyn_Exp*Cyc_Absyn_cast_exp(void*,struct Cyc_Absyn_Exp*,int user_cast,enum Cyc_Absyn_Coercion,unsigned int);
# 1056
struct Cyc_Absyn_Exp*Cyc_Absyn_sizeoftype_exp(void*t,unsigned int);
# 1064
struct Cyc_Absyn_Exp*Cyc_Absyn_stmt_exp(struct Cyc_Absyn_Stmt*,unsigned int);
# 1068
struct Cyc_Absyn_Exp*Cyc_Absyn_valueof_exp(void*,unsigned int);
# 1075
extern struct Cyc_Absyn_Exp*Cyc_Absyn_uniquergn_exp;
# 1079
struct Cyc_Absyn_Stmt*Cyc_Absyn_exp_stmt(struct Cyc_Absyn_Exp*e,unsigned int loc);
# 1090
struct Cyc_Absyn_Stmt*Cyc_Absyn_decl_stmt(struct Cyc_Absyn_Decl*d,struct Cyc_Absyn_Stmt*s,unsigned int loc);
# 1151
void*Cyc_Absyn_pointer_expand(void*,int fresh_evar);
# 1153
int Cyc_Absyn_is_lvalue(struct Cyc_Absyn_Exp*);
# 1156
struct Cyc_Absyn_Aggrfield*Cyc_Absyn_lookup_field(struct Cyc_List_List*,struct _dyneither_ptr*);
# 1158
struct Cyc_Absyn_Aggrfield*Cyc_Absyn_lookup_decl_field(struct Cyc_Absyn_Aggrdecl*,struct _dyneither_ptr*);struct Cyc_Absynpp_Params{int expand_typedefs;int qvar_to_Cids;int add_cyc_prefix;int to_VC;int decls_first;int rewrite_temp_tvars;int print_all_tvars;int print_all_kinds;int print_all_effects;int print_using_stmts;int print_externC_stmts;int print_full_evars;int print_zeroterm;int generate_line_directives;int use_curr_namespace;struct Cyc_List_List*curr_namespace;};
# 62 "absynpp.h"
struct _dyneither_ptr Cyc_Absynpp_typ2string(void*);
# 67
struct _dyneither_ptr Cyc_Absynpp_exp2string(struct Cyc_Absyn_Exp*);
# 69
struct _dyneither_ptr Cyc_Absynpp_qvar2string(struct _tuple1*);struct Cyc_Iter_Iter{void*env;int(*next)(void*env,void*dest);};
# 37 "iter.h"
int Cyc_Iter_next(struct Cyc_Iter_Iter,void*);struct Cyc_Set_Set;extern char Cyc_Set_Absent[7U];struct Cyc_Set_Absent_exn_struct{char*tag;};struct Cyc_Dict_T;struct Cyc_Dict_Dict{int(*rel)(void*,void*);struct _RegionHandle*r;const struct Cyc_Dict_T*t;};extern char Cyc_Dict_Present[8U];struct Cyc_Dict_Present_exn_struct{char*tag;};extern char Cyc_Dict_Absent[7U];struct Cyc_Dict_Absent_exn_struct{char*tag;};struct Cyc_RgnOrder_RgnPO;
# 32 "rgnorder.h"
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_initial_fn_po(struct Cyc_List_List*tvs,struct Cyc_List_List*po,void*effect,struct Cyc_Absyn_Tvar*fst_rgn,unsigned int);
# 39
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_outlives_constraint(struct Cyc_RgnOrder_RgnPO*,void*eff,void*rgn,unsigned int);
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_youngest(struct Cyc_RgnOrder_RgnPO*,struct Cyc_Absyn_Tvar*rgn,int opened);
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_unordered(struct Cyc_RgnOrder_RgnPO*,struct Cyc_Absyn_Tvar*rgn);
int Cyc_RgnOrder_effect_outlives(struct Cyc_RgnOrder_RgnPO*,void*eff,void*rgn);
int Cyc_RgnOrder_satisfies_constraints(struct Cyc_RgnOrder_RgnPO*,struct Cyc_List_List*constraints,void*default_bound,int do_pin);
# 45
int Cyc_RgnOrder_eff_outlives_eff(struct Cyc_RgnOrder_RgnPO*,void*eff1,void*eff2);
# 48
void Cyc_RgnOrder_print_region_po(struct Cyc_RgnOrder_RgnPO*po);extern char Cyc_Tcenv_Env_error[10U];struct Cyc_Tcenv_Env_error_exn_struct{char*tag;};struct Cyc_Tcenv_Genv{struct Cyc_Dict_Dict aggrdecls;struct Cyc_Dict_Dict datatypedecls;struct Cyc_Dict_Dict enumdecls;struct Cyc_Dict_Dict typedefs;struct Cyc_Dict_Dict ordinaries;};struct Cyc_Tcenv_Fenv;struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Tcenv_Genv*ae;struct Cyc_Tcenv_Fenv*le;int allow_valueof: 1;int in_extern_c_include: 1;};
# 74 "tcenv.h"
void*Cyc_Tcenv_lookup_ordinary_global(struct Cyc_Tcenv_Tenv*,unsigned int,struct _tuple1*,int is_use);
struct Cyc_Absyn_Aggrdecl**Cyc_Tcenv_lookup_aggrdecl(struct Cyc_Tcenv_Tenv*,unsigned int,struct _tuple1*);
struct Cyc_Absyn_Datatypedecl**Cyc_Tcenv_lookup_datatypedecl(struct Cyc_Tcenv_Tenv*,unsigned int,struct _tuple1*);
# 81
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_allow_valueof(struct Cyc_Tcenv_Tenv*);
# 84
enum Cyc_Tcenv_NewStatus{Cyc_Tcenv_NoneNew  = 0U,Cyc_Tcenv_InNew  = 1U,Cyc_Tcenv_InNewAggr  = 2U};
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_set_new_status(enum Cyc_Tcenv_NewStatus,struct Cyc_Tcenv_Tenv*);
enum Cyc_Tcenv_NewStatus Cyc_Tcenv_new_status(struct Cyc_Tcenv_Tenv*);
# 88
int Cyc_Tcenv_abstract_val_ok(struct Cyc_Tcenv_Tenv*);
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_enter_abstract_val_ok(struct Cyc_Tcenv_Tenv*);
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_clear_abstract_val_ok(struct Cyc_Tcenv_Tenv*);
# 94
struct Cyc_List_List*Cyc_Tcenv_lookup_type_vars(struct Cyc_Tcenv_Tenv*);
struct Cyc_Core_Opt*Cyc_Tcenv_lookup_opt_type_vars(struct Cyc_Tcenv_Tenv*);
# 110
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_enter_notreadctxt(struct Cyc_Tcenv_Tenv*);
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_clear_notreadctxt(struct Cyc_Tcenv_Tenv*);
int Cyc_Tcenv_in_notreadctxt(struct Cyc_Tcenv_Tenv*);
# 115
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_enter_lhs(struct Cyc_Tcenv_Tenv*);
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_clear_lhs(struct Cyc_Tcenv_Tenv*);
# 119
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_enter_stmt_exp(struct Cyc_Tcenv_Tenv*);
# 126
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_new_block(unsigned int,struct Cyc_Tcenv_Tenv*);
# 133
void*Cyc_Tcenv_curr_rgn(struct Cyc_Tcenv_Tenv*);
# 137
void Cyc_Tcenv_check_rgn_accessible(struct Cyc_Tcenv_Tenv*,unsigned int,void*rgn);
# 139
void Cyc_Tcenv_check_effect_accessible(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void*eff);
# 144
void Cyc_Tcenv_check_rgn_partial_order(struct Cyc_Tcenv_Tenv*te,unsigned int loc,struct Cyc_List_List*po);
# 31 "tcutil.h"
void*Cyc_Tcutil_impos(struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 33
void Cyc_Tcutil_terr(unsigned int,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 35
void Cyc_Tcutil_warn(unsigned int,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);
# 46
int Cyc_Tcutil_is_arithmetic_type(void*);
# 49
int Cyc_Tcutil_is_pointer_type(void*t);
int Cyc_Tcutil_is_array_type(void*t);
int Cyc_Tcutil_is_boxed(void*t);
# 57
int Cyc_Tcutil_is_bound_one(void*b);
# 59
int Cyc_Tcutil_is_tagged_pointer_type(void*);
# 62
int Cyc_Tcutil_is_bits_only_type(void*t);
# 69
void*Cyc_Tcutil_pointer_elt_type(void*t);
# 71
void*Cyc_Tcutil_pointer_region(void*t);
# 74
int Cyc_Tcutil_rgn_of_pointer(void*t,void**rgn);
# 77
struct Cyc_Absyn_Exp*Cyc_Tcutil_get_bounds_exp(void*def,void*b);
# 82
int Cyc_Tcutil_is_tagged_pointer_type_elt(void*t,void**elt_dest);
# 84
int Cyc_Tcutil_is_zero_pointer_type_elt(void*t,void**elt_type_dest);
# 90
struct Cyc_Absyn_Exp*Cyc_Tcutil_get_bounds_exp(void*def,void*b);
# 94
int Cyc_Tcutil_is_integral(struct Cyc_Absyn_Exp*);
int Cyc_Tcutil_is_numeric(struct Cyc_Absyn_Exp*);
int Cyc_Tcutil_is_zero(struct Cyc_Absyn_Exp*e);
# 102
void*Cyc_Tcutil_copy_type(void*t);
# 105
struct Cyc_Absyn_Exp*Cyc_Tcutil_deep_copy_exp(int preserve_types,struct Cyc_Absyn_Exp*);
# 108
int Cyc_Tcutil_kind_leq(struct Cyc_Absyn_Kind*k1,struct Cyc_Absyn_Kind*k2);
# 112
struct Cyc_Absyn_Kind*Cyc_Tcutil_tvar_kind(struct Cyc_Absyn_Tvar*t,struct Cyc_Absyn_Kind*def);
struct Cyc_Absyn_Kind*Cyc_Tcutil_type_kind(void*t);
# 115
void*Cyc_Tcutil_compress(void*t);
void Cyc_Tcutil_unchecked_cast(struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Exp*,void*,enum Cyc_Absyn_Coercion);
int Cyc_Tcutil_coerce_arg(struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Exp*,void*,int*alias_coercion);
int Cyc_Tcutil_coerce_assign(struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Exp*,void*);
int Cyc_Tcutil_coerce_to_bool(struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Exp*);
int Cyc_Tcutil_coerce_list(struct Cyc_Tcenv_Tenv*,void*,struct Cyc_List_List*);
int Cyc_Tcutil_coerce_uint_type(struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Exp*);
int Cyc_Tcutil_coerce_sint_type(struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Exp*);
# 125
int Cyc_Tcutil_silent_castable(struct Cyc_Tcenv_Tenv*,unsigned int,void*,void*);
# 127
enum Cyc_Absyn_Coercion Cyc_Tcutil_castable(struct Cyc_Tcenv_Tenv*,unsigned int,void*,void*);struct _tuple11{struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Exp*f2;};
# 133
struct _tuple11 Cyc_Tcutil_insert_alias(struct Cyc_Absyn_Exp*e,void*e_typ);
# 143
extern struct Cyc_Absyn_Kind Cyc_Tcutil_ak;
extern struct Cyc_Absyn_Kind Cyc_Tcutil_bk;
# 152
extern struct Cyc_Absyn_Kind Cyc_Tcutil_tak;
# 154
extern struct Cyc_Absyn_Kind Cyc_Tcutil_tmk;
# 161
extern struct Cyc_Core_Opt Cyc_Tcutil_rko;
extern struct Cyc_Core_Opt Cyc_Tcutil_ako;
extern struct Cyc_Core_Opt Cyc_Tcutil_bko;
# 170
extern struct Cyc_Core_Opt Cyc_Tcutil_trko;
extern struct Cyc_Core_Opt Cyc_Tcutil_tako;
# 173
extern struct Cyc_Core_Opt Cyc_Tcutil_tmko;
# 189
int Cyc_Tcutil_zero_to_null(struct Cyc_Tcenv_Tenv*,void*t,struct Cyc_Absyn_Exp*e);
# 191
void*Cyc_Tcutil_max_arithmetic_type(void*,void*);
# 195
void Cyc_Tcutil_explain_failure();
# 197
int Cyc_Tcutil_unify(void*,void*);
# 202
void*Cyc_Tcutil_substitute(struct Cyc_List_List*,void*);
# 205
struct Cyc_List_List*Cyc_Tcutil_rsubst_rgnpo(struct _RegionHandle*,struct Cyc_List_List*,struct Cyc_List_List*);
# 226
void*Cyc_Tcutil_fndecl2type(struct Cyc_Absyn_Fndecl*);struct _tuple12{struct Cyc_List_List*f1;struct _RegionHandle*f2;};struct _tuple13{struct Cyc_Absyn_Tvar*f1;void*f2;};
# 231
struct _tuple13*Cyc_Tcutil_r_make_inst_var(struct _tuple12*,struct Cyc_Absyn_Tvar*);
# 273 "tcutil.h"
void Cyc_Tcutil_check_type(unsigned int,struct Cyc_Tcenv_Tenv*,struct Cyc_List_List*bound_tvars,struct Cyc_Absyn_Kind*k,int allow_evars,int allow_abs_aggr,void*);
# 283
void Cyc_Tcutil_check_nonzero_bound(unsigned int,void*);
# 285
void Cyc_Tcutil_check_bound(unsigned int,unsigned int i,void*,int do_warn);
# 289
struct Cyc_List_List*Cyc_Tcutil_resolve_aggregate_designators(struct _RegionHandle*rgn,unsigned int loc,struct Cyc_List_List*des,enum Cyc_Absyn_AggrKind,struct Cyc_List_List*fields);
# 302
int Cyc_Tcutil_is_noalias_region(void*r,int must_be_unique);
# 305
int Cyc_Tcutil_is_noalias_pointer(void*t,int must_be_unique);
# 310
int Cyc_Tcutil_is_noalias_path(struct Cyc_Absyn_Exp*e);
# 315
int Cyc_Tcutil_is_noalias_pointer_or_aggr(void*t);struct _tuple14{int f1;void*f2;};
# 319
struct _tuple14 Cyc_Tcutil_addressof_props(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e);
# 345
int Cyc_Tcutil_is_const_exp(struct Cyc_Absyn_Exp*e);
# 370
void Cyc_Tcutil_check_no_qual(unsigned int loc,void*t);
# 381
void*Cyc_Tcutil_promote_array(void*t,void*rgn,int convert_tag);
# 384
int Cyc_Tcutil_zeroable_type(void*t);
# 388
int Cyc_Tcutil_force_type2bool(int desired,void*t);
# 391
void*Cyc_Tcutil_any_bool(struct Cyc_Tcenv_Tenv**te);
# 393
void*Cyc_Tcutil_any_bounds(struct Cyc_Tcenv_Tenv**te);struct _tuple15{unsigned int f1;int f2;};
# 28 "evexp.h"
struct _tuple15 Cyc_Evexp_eval_const_uint_exp(struct Cyc_Absyn_Exp*e);
# 32
int Cyc_Evexp_c_can_eval(struct Cyc_Absyn_Exp*e);
# 41 "evexp.h"
int Cyc_Evexp_same_const_exp(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2);
# 48
int Cyc_Evexp_okay_szofarg(void*t);
# 26 "tcstmt.h"
void Cyc_Tcstmt_tcStmt(struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Stmt*,int new_block);
# 30 "formatstr.h"
struct Cyc_List_List*Cyc_Formatstr_get_format_types(struct Cyc_Tcenv_Tenv*,struct _dyneither_ptr,unsigned int);
# 33
struct Cyc_List_List*Cyc_Formatstr_get_scanf_types(struct Cyc_Tcenv_Tenv*,struct _dyneither_ptr,unsigned int);
# 31 "tc.h"
extern int Cyc_Tc_aggressive_warn;
# 28 "tcexp.h"
void*Cyc_Tcexp_tcExp(struct Cyc_Tcenv_Tenv*,void**,struct Cyc_Absyn_Exp*);
void*Cyc_Tcexp_tcExpInitializer(struct Cyc_Tcenv_Tenv*,void**,struct Cyc_Absyn_Exp*);
void Cyc_Tcexp_tcTest(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e,struct _dyneither_ptr msg_part);struct Cyc_Hashtable_Table;struct Cyc_JumpAnalysis_Jump_Anal_Result{struct Cyc_Hashtable_Table*pop_tables;struct Cyc_Hashtable_Table*succ_tables;struct Cyc_Hashtable_Table*pat_pop_tables;};
# 46 "jump_analysis.h"
struct Cyc_JumpAnalysis_Jump_Anal_Result*Cyc_JumpAnalysis_jump_analysis(struct Cyc_List_List*tds);struct _union_RelnOp_RConst{int tag;unsigned int val;};struct _union_RelnOp_RVar{int tag;struct Cyc_Absyn_Vardecl*val;};struct _union_RelnOp_RNumelts{int tag;struct Cyc_Absyn_Vardecl*val;};struct _union_RelnOp_RType{int tag;void*val;};struct _union_RelnOp_RParam{int tag;unsigned int val;};struct _union_RelnOp_RParamNumelts{int tag;unsigned int val;};struct _union_RelnOp_RReturn{int tag;unsigned int val;};union Cyc_Relations_RelnOp{struct _union_RelnOp_RConst RConst;struct _union_RelnOp_RVar RVar;struct _union_RelnOp_RNumelts RNumelts;struct _union_RelnOp_RType RType;struct _union_RelnOp_RParam RParam;struct _union_RelnOp_RParamNumelts RParamNumelts;struct _union_RelnOp_RReturn RReturn;};
# 49 "relations-ap.h"
enum Cyc_Relations_Relation{Cyc_Relations_Req  = 0U,Cyc_Relations_Rneq  = 1U,Cyc_Relations_Rlte  = 2U,Cyc_Relations_Rlt  = 3U};struct Cyc_Relations_Reln{union Cyc_Relations_RelnOp rop1;enum Cyc_Relations_Relation relation;union Cyc_Relations_RelnOp rop2;};
# 41 "cf_flowinfo.h"
int Cyc_CfFlowInfo_anal_error;
void Cyc_CfFlowInfo_aerr(unsigned int loc,struct _dyneither_ptr fmt,struct _dyneither_ptr ap);struct Cyc_CfFlowInfo_VarRoot_CfFlowInfo_Root_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_CfFlowInfo_MallocPt_CfFlowInfo_Root_struct{int tag;struct Cyc_Absyn_Exp*f1;void*f2;};struct Cyc_CfFlowInfo_InitParam_CfFlowInfo_Root_struct{int tag;int f1;void*f2;};struct Cyc_CfFlowInfo_Dot_CfFlowInfo_PathCon_struct{int tag;int f1;};struct Cyc_CfFlowInfo_Star_CfFlowInfo_PathCon_struct{int tag;};struct Cyc_CfFlowInfo_Place{void*root;struct Cyc_List_List*path;};
# 74
enum Cyc_CfFlowInfo_InitLevel{Cyc_CfFlowInfo_NoneIL  = 0U,Cyc_CfFlowInfo_AllIL  = 1U};extern char Cyc_CfFlowInfo_IsZero[7U];struct Cyc_CfFlowInfo_IsZero_Absyn_AbsynAnnot_struct{char*tag;};extern char Cyc_CfFlowInfo_NotZero[8U];struct Cyc_CfFlowInfo_NotZero_Absyn_AbsynAnnot_struct{char*tag;struct Cyc_List_List*f1;};extern char Cyc_CfFlowInfo_UnknownZ[9U];struct Cyc_CfFlowInfo_UnknownZ_Absyn_AbsynAnnot_struct{char*tag;struct Cyc_List_List*f1;};struct _union_AbsLVal_PlaceL{int tag;struct Cyc_CfFlowInfo_Place*val;};struct _union_AbsLVal_UnknownL{int tag;int val;};union Cyc_CfFlowInfo_AbsLVal{struct _union_AbsLVal_PlaceL PlaceL;struct _union_AbsLVal_UnknownL UnknownL;};struct Cyc_CfFlowInfo_UnionRInfo{int is_union;int fieldnum;};struct Cyc_CfFlowInfo_Zero_CfFlowInfo_AbsRVal_struct{int tag;};struct Cyc_CfFlowInfo_NotZeroAll_CfFlowInfo_AbsRVal_struct{int tag;};struct Cyc_CfFlowInfo_UnknownR_CfFlowInfo_AbsRVal_struct{int tag;enum Cyc_CfFlowInfo_InitLevel f1;};struct Cyc_CfFlowInfo_Esc_CfFlowInfo_AbsRVal_struct{int tag;enum Cyc_CfFlowInfo_InitLevel f1;};struct Cyc_CfFlowInfo_AddressOf_CfFlowInfo_AbsRVal_struct{int tag;struct Cyc_CfFlowInfo_Place*f1;};struct Cyc_CfFlowInfo_UniquePtr_CfFlowInfo_AbsRVal_struct{int tag;void*f1;};struct Cyc_CfFlowInfo_Aggregate_CfFlowInfo_AbsRVal_struct{int tag;struct Cyc_CfFlowInfo_UnionRInfo f1;struct _dyneither_ptr f2;};struct Cyc_CfFlowInfo_Consumed_CfFlowInfo_AbsRVal_struct{int tag;struct Cyc_Absyn_Exp*f1;int f2;void*f3;};struct Cyc_CfFlowInfo_NamedLocation_CfFlowInfo_AbsRVal_struct{int tag;struct Cyc_Absyn_Vardecl*f1;void*f2;};struct _union_FlowInfo_BottomFL{int tag;int val;};struct _tuple16{struct Cyc_Dict_Dict f1;struct Cyc_List_List*f2;};struct _union_FlowInfo_ReachableFL{int tag;struct _tuple16 val;};union Cyc_CfFlowInfo_FlowInfo{struct _union_FlowInfo_BottomFL BottomFL;struct _union_FlowInfo_ReachableFL ReachableFL;};struct Cyc_CfFlowInfo_FlowEnv{void*zero;void*notzeroall;void*unknown_none;void*unknown_all;void*esc_none;void*esc_all;struct Cyc_Dict_Dict mt_flowdict;struct Cyc_CfFlowInfo_Place*dummy_place;};
# 47 "tcexp.cyc"
static void*Cyc_Tcexp_expr_err(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct _dyneither_ptr msg,struct _dyneither_ptr ap){
# 51
({void*_tmp0=0U;({unsigned int _tmp558=loc;struct _dyneither_ptr _tmp557=(struct _dyneither_ptr)Cyc_vrprintf(Cyc_Core_heap_region,msg,ap);Cyc_Tcutil_terr(_tmp558,_tmp557,_tag_dyneither(_tmp0,sizeof(void*),0U));});});
if(topt == 0)
return Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te));else{
# 55
return*topt;}}
# 64
static void Cyc_Tcexp_resolve_unresolved_mem(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e,struct Cyc_List_List*des){
# 68
if(topt == 0){
# 70
({void*_tmp559=(void*)({struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*_tmp1=_cycalloc(sizeof(*_tmp1));_tmp1->tag=26U,_tmp1->f1=des;_tmp1;});e->r=_tmp559;});
return;}{
# 73
void*t=*topt;
void*_tmp2=Cyc_Tcutil_compress(t);void*_tmp3=_tmp2;void*_tmpE;struct Cyc_Absyn_Tqual _tmpD;union Cyc_Absyn_AggrInfo _tmpC;switch(*((int*)_tmp3)){case 0U: if(((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3)->f1)->tag == 20U){_LL1: _tmpC=((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3)->f1)->f1;_LL2:
# 76
{union Cyc_Absyn_AggrInfo _tmp4=_tmpC;struct Cyc_Absyn_Aggrdecl*_tmp8;if((_tmp4.UnknownAggr).tag == 1){_LLA: _LLB:
({void*_tmp5=0U;({struct _dyneither_ptr _tmp55A=({const char*_tmp6="struct type not properly set";_tag_dyneither(_tmp6,sizeof(char),29U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp55A,_tag_dyneither(_tmp5,sizeof(void*),0U));});});}else{_LLC: _tmp8=*(_tmp4.KnownAggr).val;_LLD:
({void*_tmp55B=(void*)({struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*_tmp7=_cycalloc(sizeof(*_tmp7));_tmp7->tag=29U,_tmp7->f1=_tmp8->name,_tmp7->f2=0,_tmp7->f3=des,_tmp7->f4=_tmp8;_tmp7;});e->r=_tmp55B;});}_LL9:;}
# 80
goto _LL0;}else{goto _LL7;}case 4U: _LL3: _tmpE=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp3)->f1).elt_type;_tmpD=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp3)->f1).tq;_LL4:
({void*_tmp55C=(void*)({struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*_tmp9=_cycalloc(sizeof(*_tmp9));_tmp9->tag=26U,_tmp9->f1=des;_tmp9;});e->r=_tmp55C;});goto _LL0;case 7U: _LL5: _LL6:
({void*_tmp55D=(void*)({struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct*_tmpA=_cycalloc(sizeof(*_tmpA));_tmpA->tag=30U,_tmpA->f1=t,_tmpA->f2=des;_tmpA;});e->r=_tmp55D;});goto _LL0;default: _LL7: _LL8:
({void*_tmp55E=(void*)({struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*_tmpB=_cycalloc(sizeof(*_tmpB));_tmpB->tag=26U,_tmpB->f1=des;_tmpB;});e->r=_tmp55E;});goto _LL0;}_LL0:;};}
# 90
static void Cyc_Tcexp_tcExpNoInst(struct Cyc_Tcenv_Tenv*te,void**topt,struct Cyc_Absyn_Exp*e);
static void*Cyc_Tcexp_tcExpNoPromote(struct Cyc_Tcenv_Tenv*te,void**topt,struct Cyc_Absyn_Exp*e);
# 94
static void Cyc_Tcexp_tcExpList(struct Cyc_Tcenv_Tenv*te,struct Cyc_List_List*es){
for(0;es != 0;es=es->tl){
Cyc_Tcexp_tcExp(te,0,(struct Cyc_Absyn_Exp*)es->hd);}}
# 100
static void Cyc_Tcexp_check_contains_assign(struct Cyc_Absyn_Exp*e){
void*_tmpF=e->r;void*_tmp10=_tmpF;if(((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp10)->tag == 4U){if(((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp10)->f2 == 0){_LL1: _LL2:
# 103
 if(Cyc_Tc_aggressive_warn)({void*_tmp11=0U;({unsigned int _tmp560=e->loc;struct _dyneither_ptr _tmp55F=({const char*_tmp12="assignment in test";_tag_dyneither(_tmp12,sizeof(char),19U);});Cyc_Tcutil_warn(_tmp560,_tmp55F,_tag_dyneither(_tmp11,sizeof(void*),0U));});});
goto _LL0;}else{goto _LL3;}}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 110
void Cyc_Tcexp_tcTest(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e,struct _dyneither_ptr msg_part){
Cyc_Tcexp_check_contains_assign(e);
Cyc_Tcexp_tcExp(te,& Cyc_Absyn_sint_type,e);
if(!Cyc_Tcutil_coerce_to_bool(te,e))
({struct Cyc_String_pa_PrintArg_struct _tmp15=({struct Cyc_String_pa_PrintArg_struct _tmp4D3;_tmp4D3.tag=0U,_tmp4D3.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)msg_part);_tmp4D3;});struct Cyc_String_pa_PrintArg_struct _tmp16=({struct Cyc_String_pa_PrintArg_struct _tmp4D2;_tmp4D2.tag=0U,({
struct _dyneither_ptr _tmp561=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(e->topt)));_tmp4D2.f1=_tmp561;});_tmp4D2;});void*_tmp13[2U];_tmp13[0]=& _tmp15,_tmp13[1]=& _tmp16;({unsigned int _tmp563=e->loc;struct _dyneither_ptr _tmp562=({const char*_tmp14="test of %s has type %s instead of integral or pointer type";_tag_dyneither(_tmp14,sizeof(char),59U);});Cyc_Tcutil_terr(_tmp563,_tmp562,_tag_dyneither(_tmp13,sizeof(void*),2U));});});}
# 136 "tcexp.cyc"
static int Cyc_Tcexp_wchar_numelts(struct _dyneither_ptr s){
return 1;}
# 141
static void*Cyc_Tcexp_tcConst(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,union Cyc_Absyn_Cnst*c,struct Cyc_Absyn_Exp*e){
void*t;
void*string_elt_typ=Cyc_Absyn_char_type;
int string_numelts=0;
{union Cyc_Absyn_Cnst _tmp17=*((union Cyc_Absyn_Cnst*)_check_null(c));union Cyc_Absyn_Cnst _tmp18=_tmp17;struct _dyneither_ptr _tmp4B;struct _dyneither_ptr _tmp4A;enum Cyc_Absyn_Sign _tmp49;int _tmp48;int _tmp47;enum Cyc_Absyn_Sign _tmp46;enum Cyc_Absyn_Sign _tmp45;switch((_tmp18.Wstring_c).tag){case 2U: switch(((_tmp18.Char_c).val).f1){case Cyc_Absyn_Signed: _LL1: _LL2:
 t=Cyc_Absyn_schar_type;goto _LL0;case Cyc_Absyn_Unsigned: _LL3: _LL4:
 t=Cyc_Absyn_uchar_type;goto _LL0;default: _LL5: _LL6:
 t=Cyc_Absyn_char_type;goto _LL0;}case 3U: _LL7: _LL8:
 t=Cyc_Absyn_wchar_type();goto _LL0;case 4U: _LL9: _tmp45=((_tmp18.Short_c).val).f1;_LLA:
# 151
 t=_tmp45 == Cyc_Absyn_Unsigned?Cyc_Absyn_ushort_type: Cyc_Absyn_sshort_type;goto _LL0;case 6U: _LLB: _tmp46=((_tmp18.LongLong_c).val).f1;_LLC:
# 153
 t=_tmp46 == Cyc_Absyn_Unsigned?Cyc_Absyn_ulonglong_type: Cyc_Absyn_slonglong_type;goto _LL0;case 7U: _LLD: _tmp47=((_tmp18.Float_c).val).f2;_LLE:
# 155
 if(topt == 0)t=Cyc_Absyn_gen_float_type((unsigned int)_tmp47);else{
# 159
void*_tmp19=Cyc_Tcutil_compress(*topt);void*_tmp1A=_tmp19;int _tmp1B;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp1A)->tag == 0U){if(((struct Cyc_Absyn_FloatCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp1A)->f1)->tag == 2U){_LL18: _tmp1B=((struct Cyc_Absyn_FloatCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp1A)->f1)->f1;_LL19:
 t=Cyc_Absyn_gen_float_type((unsigned int)_tmp1B);goto _LL17;}else{goto _LL1A;}}else{_LL1A: _LL1B:
# 163
 t=Cyc_Absyn_gen_float_type((unsigned int)_tmp47);
goto _LL17;}_LL17:;}
# 167
goto _LL0;case 5U: _LLF: _tmp49=((_tmp18.Int_c).val).f1;_tmp48=((_tmp18.Int_c).val).f2;_LL10:
# 169
 if(topt == 0)
t=_tmp49 == Cyc_Absyn_Unsigned?Cyc_Absyn_uint_type: Cyc_Absyn_sint_type;else{
# 176
void*_tmp1C=Cyc_Tcutil_compress(*topt);void*_tmp1D=_tmp1C;void*_tmp30;struct Cyc_Absyn_Tqual _tmp2F;void*_tmp2E;void*_tmp2D;void*_tmp2C;void*_tmp2B;void*_tmp2A;enum Cyc_Absyn_Sign _tmp29;enum Cyc_Absyn_Sign _tmp28;enum Cyc_Absyn_Sign _tmp27;enum Cyc_Absyn_Sign _tmp26;switch(*((int*)_tmp1D)){case 0U: switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp1D)->f1)){case 1U: switch(((struct Cyc_Absyn_IntCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp1D)->f1)->f2){case Cyc_Absyn_Char_sz: _LL1D: _tmp26=((struct Cyc_Absyn_IntCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp1D)->f1)->f1;_LL1E:
# 178
{enum Cyc_Absyn_Sign _tmp1E=_tmp26;switch(_tmp1E){case Cyc_Absyn_Unsigned: _LL2C: _LL2D:
 t=Cyc_Absyn_uchar_type;goto _LL2B;case Cyc_Absyn_Signed: _LL2E: _LL2F:
 t=Cyc_Absyn_schar_type;goto _LL2B;default: _LL30: _LL31:
 t=Cyc_Absyn_char_type;goto _LL2B;}_LL2B:;}
# 183
({union Cyc_Absyn_Cnst _tmp564=Cyc_Absyn_Char_c(_tmp26,(char)_tmp48);*c=_tmp564;});
goto _LL1C;case Cyc_Absyn_Short_sz: _LL1F: _tmp27=((struct Cyc_Absyn_IntCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp1D)->f1)->f1;_LL20:
# 186
 t=_tmp27 == Cyc_Absyn_Unsigned?Cyc_Absyn_ushort_type: Cyc_Absyn_sshort_type;
({union Cyc_Absyn_Cnst _tmp565=Cyc_Absyn_Short_c(_tmp27,(short)_tmp48);*c=_tmp565;});
goto _LL1C;case Cyc_Absyn_Int_sz: _LL21: _tmp28=((struct Cyc_Absyn_IntCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp1D)->f1)->f1;_LL22:
# 190
 t=_tmp28 == Cyc_Absyn_Unsigned?Cyc_Absyn_uint_type: Cyc_Absyn_sint_type;
# 193
({union Cyc_Absyn_Cnst _tmp566=Cyc_Absyn_Int_c(_tmp28,_tmp48);*c=_tmp566;});
goto _LL1C;case Cyc_Absyn_Long_sz: _LL23: _tmp29=((struct Cyc_Absyn_IntCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp1D)->f1)->f1;_LL24:
# 196
 t=_tmp29 == Cyc_Absyn_Unsigned?Cyc_Absyn_uint_type: Cyc_Absyn_sint_type;
# 199
({union Cyc_Absyn_Cnst _tmp567=Cyc_Absyn_Int_c(_tmp29,_tmp48);*c=_tmp567;});
goto _LL1C;default: goto _LL29;}case 4U: if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp1D)->f2 != 0){_LL27: _tmp2A=(void*)(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp1D)->f2)->hd;_LL28: {
# 213
struct Cyc_Absyn_ValueofType_Absyn_Type_struct*_tmp24=({struct Cyc_Absyn_ValueofType_Absyn_Type_struct*_tmp25=_cycalloc(sizeof(*_tmp25));_tmp25->tag=9U,({struct Cyc_Absyn_Exp*_tmp568=Cyc_Absyn_uint_exp((unsigned int)_tmp48,0U);_tmp25->f1=_tmp568;});_tmp25;});
# 220
t=Cyc_Absyn_tag_type((void*)_tmp24);
goto _LL1C;}}else{goto _LL29;}default: goto _LL29;}case 3U: _LL25: _tmp30=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D)->f1).elt_type;_tmp2F=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D)->f1).elt_tq;_tmp2E=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D)->f1).ptr_atts).rgn;_tmp2D=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D)->f1).ptr_atts).nullable;_tmp2C=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D)->f1).ptr_atts).bounds;_tmp2B=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D)->f1).ptr_atts).zero_term;if(_tmp48 == 0){_LL26: {
# 203
static struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct nullc={0U,{.Null_c={1,0}}};
e->r=(void*)& nullc;
if(Cyc_Tcutil_force_type2bool(1,_tmp2D))return*topt;
({struct Cyc_String_pa_PrintArg_struct _tmp21=({struct Cyc_String_pa_PrintArg_struct _tmp4D4;_tmp4D4.tag=0U,({
struct _dyneither_ptr _tmp569=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(*topt));_tmp4D4.f1=_tmp569;});_tmp4D4;});void*_tmp1F[1U];_tmp1F[0]=& _tmp21;({unsigned int _tmp56B=e->loc;struct _dyneither_ptr _tmp56A=({const char*_tmp20="Used NULL when expecting a non-NULL value of type %s";_tag_dyneither(_tmp20,sizeof(char),53U);});Cyc_Tcutil_terr(_tmp56B,_tmp56A,_tag_dyneither(_tmp1F,sizeof(void*),1U));});});{
struct Cyc_List_List*_tmp22=Cyc_Tcenv_lookup_type_vars(te);
t=(void*)({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp23=_cycalloc(sizeof(*_tmp23));_tmp23->tag=3U,(_tmp23->f1).elt_type=_tmp30,(_tmp23->f1).elt_tq=_tmp2F,
((_tmp23->f1).ptr_atts).rgn=_tmp2E,((_tmp23->f1).ptr_atts).nullable=Cyc_Absyn_true_type,((_tmp23->f1).ptr_atts).bounds=_tmp2C,((_tmp23->f1).ptr_atts).zero_term=_tmp2B,((_tmp23->f1).ptr_atts).ptrloc=0;_tmp23;});
goto _LL1C;};}}else{goto _LL29;}default: _LL29: _LL2A:
# 223
 t=_tmp49 == Cyc_Absyn_Unsigned?Cyc_Absyn_uint_type: Cyc_Absyn_sint_type;
goto _LL1C;}_LL1C:;}
# 226
goto _LL0;case 8U: _LL11: _tmp4A=(_tmp18.String_c).val;_LL12:
# 228
 string_numelts=(int)_get_dyneither_size(_tmp4A,sizeof(char));
_tmp4B=_tmp4A;goto _LL14;case 9U: _LL13: _tmp4B=(_tmp18.Wstring_c).val;_LL14:
# 231
 if(string_numelts == 0){
string_numelts=Cyc_Tcexp_wchar_numelts(_tmp4B);
string_elt_typ=Cyc_Absyn_wchar_type();}{
# 235
struct Cyc_Absyn_Exp*elen=({union Cyc_Absyn_Cnst _tmp56C=Cyc_Absyn_Int_c(Cyc_Absyn_Unsigned,string_numelts);Cyc_Absyn_const_exp(_tmp56C,loc);});
elen->topt=Cyc_Absyn_uint_type;{
# 240
void*_tmp31=Cyc_Absyn_thin_bounds_exp(elen);
t=({void*_tmp570=string_elt_typ;void*_tmp56F=Cyc_Absyn_heap_rgn_type;struct Cyc_Absyn_Tqual _tmp56E=Cyc_Absyn_const_tqual(0U);void*_tmp56D=_tmp31;Cyc_Absyn_atb_type(_tmp570,_tmp56F,_tmp56E,_tmp56D,Cyc_Absyn_true_type);});
# 243
if(topt != 0){
void*_tmp32=Cyc_Tcutil_compress(*topt);void*_tmp33=_tmp32;struct Cyc_Absyn_Tqual _tmp34;switch(*((int*)_tmp33)){case 4U: _LL33: _tmp34=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp33)->f1).tq;_LL34:
# 249
 return({void*_tmp573=string_elt_typ;struct Cyc_Absyn_Tqual _tmp572=_tmp34;struct Cyc_Absyn_Exp*_tmp571=elen;Cyc_Absyn_array_type(_tmp573,_tmp572,_tmp571,Cyc_Tcutil_any_bool(& te),0U);});case 3U: _LL35: _LL36:
# 251
 if(!Cyc_Tcutil_unify(*topt,t) && Cyc_Tcutil_silent_castable(te,loc,t,*topt)){
e->topt=t;
Cyc_Tcutil_unchecked_cast(te,e,*topt,Cyc_Absyn_Other_coercion);
t=*topt;}else{
# 257
t=({void*_tmp577=string_elt_typ;void*_tmp576=Cyc_Absyn_new_evar(& Cyc_Tcutil_rko,Cyc_Tcenv_lookup_opt_type_vars(te));struct Cyc_Absyn_Tqual _tmp575=
Cyc_Absyn_const_tqual(0U);
# 257
void*_tmp574=_tmp31;Cyc_Absyn_atb_type(_tmp577,_tmp576,_tmp575,_tmp574,Cyc_Absyn_true_type);});
# 259
if(!Cyc_Tcutil_unify(*topt,t) && Cyc_Tcutil_silent_castable(te,loc,t,*topt)){
e->topt=t;
Cyc_Tcutil_unchecked_cast(te,e,*topt,Cyc_Absyn_Other_coercion);
t=*topt;}}
# 265
goto _LL32;default: _LL37: _LL38:
 goto _LL32;}_LL32:;}
# 269
return t;};};default: _LL15: _LL16:
# 271
 if(topt != 0){
void*_tmp35=Cyc_Tcutil_compress(*topt);void*_tmp36=_tmp35;void*_tmp40;struct Cyc_Absyn_Tqual _tmp3F;void*_tmp3E;void*_tmp3D;void*_tmp3C;void*_tmp3B;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp36)->tag == 3U){_LL3A: _tmp40=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp36)->f1).elt_type;_tmp3F=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp36)->f1).elt_tq;_tmp3E=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp36)->f1).ptr_atts).rgn;_tmp3D=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp36)->f1).ptr_atts).nullable;_tmp3C=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp36)->f1).ptr_atts).bounds;_tmp3B=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp36)->f1).ptr_atts).zero_term;_LL3B:
# 275
 if(Cyc_Tcutil_force_type2bool(1,_tmp3D))return*topt;
({struct Cyc_String_pa_PrintArg_struct _tmp39=({struct Cyc_String_pa_PrintArg_struct _tmp4D5;_tmp4D5.tag=0U,({
struct _dyneither_ptr _tmp578=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(*topt));_tmp4D5.f1=_tmp578;});_tmp4D5;});void*_tmp37[1U];_tmp37[0]=& _tmp39;({unsigned int _tmp57A=e->loc;struct _dyneither_ptr _tmp579=({const char*_tmp38="Used NULL when expecting a non-NULL value of type %s";_tag_dyneither(_tmp38,sizeof(char),53U);});Cyc_Tcutil_terr(_tmp57A,_tmp579,_tag_dyneither(_tmp37,sizeof(void*),1U));});});
return(void*)({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp3A=_cycalloc(sizeof(*_tmp3A));_tmp3A->tag=3U,(_tmp3A->f1).elt_type=_tmp40,(_tmp3A->f1).elt_tq=_tmp3F,((_tmp3A->f1).ptr_atts).rgn=_tmp3E,((_tmp3A->f1).ptr_atts).nullable=Cyc_Absyn_true_type,((_tmp3A->f1).ptr_atts).bounds=_tmp3C,((_tmp3A->f1).ptr_atts).zero_term=_tmp3B,((_tmp3A->f1).ptr_atts).ptrloc=0;_tmp3A;});}else{_LL3C: _LL3D:
# 280
 goto _LL39;}_LL39:;}{
# 283
struct Cyc_List_List*_tmp41=Cyc_Tcenv_lookup_type_vars(te);
t=(void*)({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp44=_cycalloc(sizeof(*_tmp44));_tmp44->tag=3U,({void*_tmp57F=Cyc_Absyn_new_evar(& Cyc_Tcutil_tako,({struct Cyc_Core_Opt*_tmp42=_cycalloc(sizeof(*_tmp42));_tmp42->v=_tmp41;_tmp42;}));(_tmp44->f1).elt_type=_tmp57F;}),({
struct Cyc_Absyn_Tqual _tmp57E=Cyc_Absyn_empty_tqual(0U);(_tmp44->f1).elt_tq=_tmp57E;}),
({void*_tmp57D=Cyc_Absyn_new_evar(& Cyc_Tcutil_trko,({struct Cyc_Core_Opt*_tmp43=_cycalloc(sizeof(*_tmp43));_tmp43->v=_tmp41;_tmp43;}));((_tmp44->f1).ptr_atts).rgn=_tmp57D;}),((_tmp44->f1).ptr_atts).nullable=Cyc_Absyn_true_type,({
void*_tmp57C=Cyc_Tcutil_any_bounds(& te);((_tmp44->f1).ptr_atts).bounds=_tmp57C;}),({
void*_tmp57B=Cyc_Tcutil_any_bool(& te);((_tmp44->f1).ptr_atts).zero_term=_tmp57B;}),((_tmp44->f1).ptr_atts).ptrloc=0;_tmp44;});
# 290
goto _LL0;};}_LL0:;}
# 292
return t;}
# 296
static void*Cyc_Tcexp_tcDatatype(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e,struct Cyc_List_List*es,struct Cyc_Absyn_Datatypedecl*tud,struct Cyc_Absyn_Datatypefield*tuf);
# 301
static void*Cyc_Tcexp_tcVar(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e,void**b){
void*_tmp4C=*((void**)_check_null(b));void*_tmp4D=_tmp4C;struct Cyc_Absyn_Vardecl*_tmp59;struct Cyc_Absyn_Vardecl*_tmp58;struct Cyc_Absyn_Vardecl*_tmp57;struct Cyc_Absyn_Fndecl*_tmp56;struct Cyc_Absyn_Vardecl*_tmp55;struct _tuple1*_tmp54;switch(*((int*)_tmp4D)){case 0U: _LL1: _tmp54=((struct Cyc_Absyn_Unresolved_b_Absyn_Binding_struct*)_tmp4D)->f1;_LL2:
# 304
(int)_throw((void*)({struct Cyc_Core_Impossible_exn_struct*_tmp4F=_cycalloc(sizeof(*_tmp4F));_tmp4F->tag=Cyc_Core_Impossible,({struct _dyneither_ptr _tmp580=({const char*_tmp4E="unresolved binding in tcVar";_tag_dyneither(_tmp4E,sizeof(char),28U);});_tmp4F->f1=_tmp580;});_tmp4F;}));case 1U: _LL3: _tmp55=((struct Cyc_Absyn_Global_b_Absyn_Binding_struct*)_tmp4D)->f1;_LL4:
# 308
 Cyc_Tcenv_lookup_ordinary_global(te,loc,_tmp55->name,1);
return _tmp55->type;case 2U: _LL5: _tmp56=((struct Cyc_Absyn_Funname_b_Absyn_Binding_struct*)_tmp4D)->f1;_LL6:
# 315
 if(_tmp56->fn_vardecl == 0)
Cyc_Tcenv_lookup_ordinary_global(te,loc,_tmp56->name,1);
return Cyc_Tcutil_fndecl2type(_tmp56);case 5U: _LL7: _tmp57=((struct Cyc_Absyn_Pat_b_Absyn_Binding_struct*)_tmp4D)->f1;_LL8:
 _tmp58=_tmp57;goto _LLA;case 4U: _LL9: _tmp58=((struct Cyc_Absyn_Local_b_Absyn_Binding_struct*)_tmp4D)->f1;_LLA:
 _tmp59=_tmp58;goto _LLC;default: _LLB: _tmp59=((struct Cyc_Absyn_Param_b_Absyn_Binding_struct*)_tmp4D)->f1;_LLC:
# 321
 if(te->allow_valueof){
void*_tmp50=Cyc_Tcutil_compress(_tmp59->type);void*_tmp51=_tmp50;void*_tmp53;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp51)->tag == 0U){if(((struct Cyc_Absyn_TagCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp51)->f1)->tag == 4U){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp51)->f2 != 0){_LLE: _tmp53=(void*)(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp51)->f2)->hd;_LLF:
# 324
({void*_tmp581=(void*)({struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct*_tmp52=_cycalloc(sizeof(*_tmp52));_tmp52->tag=39U,_tmp52->f1=_tmp53;_tmp52;});e->r=_tmp581;});
goto _LLD;}else{goto _LL10;}}else{goto _LL10;}}else{_LL10: _LL11:
 goto _LLD;}_LLD:;}
# 329
return _tmp59->type;}_LL0:;}
# 333
static void Cyc_Tcexp_check_format_args(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*fmt,struct Cyc_Core_Opt*opt_args,int arg_cnt,struct Cyc_List_List**alias_arg_exps,struct Cyc_List_List*(*type_getter)(struct Cyc_Tcenv_Tenv*,struct _dyneither_ptr,unsigned int)){
# 340
struct Cyc_List_List*desc_types;
{void*_tmp5A=fmt->r;void*_tmp5B=_tmp5A;struct _dyneither_ptr _tmp60;struct _dyneither_ptr _tmp5F;switch(*((int*)_tmp5B)){case 0U: if(((((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp5B)->f1).String_c).tag == 8){_LL1: _tmp5F=((((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp5B)->f1).String_c).val;_LL2:
 _tmp60=_tmp5F;goto _LL4;}else{goto _LL5;}case 14U: if(((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)((struct Cyc_Absyn_Exp*)((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp5B)->f2)->r)->tag == 0U){if(((((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)((struct Cyc_Absyn_Exp*)((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp5B)->f2)->r)->f1).String_c).tag == 8){_LL3: _tmp60=((((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)(((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp5B)->f2)->r)->f1).String_c).val;_LL4:
# 344
 desc_types=type_getter(te,(struct _dyneither_ptr)_tmp60,fmt->loc);goto _LL0;}else{goto _LL5;}}else{goto _LL5;}default: _LL5: _LL6:
# 348
 if(opt_args != 0){
struct Cyc_List_List*_tmp5C=(struct Cyc_List_List*)opt_args->v;
for(0;_tmp5C != 0;_tmp5C=_tmp5C->tl){
Cyc_Tcexp_tcExp(te,0,(struct Cyc_Absyn_Exp*)_tmp5C->hd);
if(Cyc_Tcutil_is_noalias_pointer_or_aggr((void*)_check_null(((struct Cyc_Absyn_Exp*)_tmp5C->hd)->topt)) && !
Cyc_Tcutil_is_noalias_path((struct Cyc_Absyn_Exp*)_tmp5C->hd))
({void*_tmp5D=0U;({unsigned int _tmp583=((struct Cyc_Absyn_Exp*)_tmp5C->hd)->loc;struct _dyneither_ptr _tmp582=({const char*_tmp5E="Cannot consume non-unique paths; do swap instead";_tag_dyneither(_tmp5E,sizeof(char),49U);});Cyc_Tcutil_terr(_tmp583,_tmp582,_tag_dyneither(_tmp5D,sizeof(void*),0U));});});}}
# 357
return;}_LL0:;}
# 359
if(opt_args != 0){
struct Cyc_List_List*_tmp61=(struct Cyc_List_List*)opt_args->v;
# 362
for(0;desc_types != 0  && _tmp61 != 0;(
desc_types=desc_types->tl,_tmp61=_tmp61->tl,arg_cnt ++)){
int alias_coercion=0;
void*t=(void*)desc_types->hd;
struct Cyc_Absyn_Exp*e=(struct Cyc_Absyn_Exp*)_tmp61->hd;
Cyc_Tcexp_tcExp(te,& t,e);
if(!Cyc_Tcutil_coerce_arg(te,e,t,& alias_coercion)){
({struct Cyc_String_pa_PrintArg_struct _tmp64=({struct Cyc_String_pa_PrintArg_struct _tmp4D7;_tmp4D7.tag=0U,({
struct _dyneither_ptr _tmp584=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp4D7.f1=_tmp584;});_tmp4D7;});struct Cyc_String_pa_PrintArg_struct _tmp65=({struct Cyc_String_pa_PrintArg_struct _tmp4D6;_tmp4D6.tag=0U,({struct _dyneither_ptr _tmp585=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(e->topt)));_tmp4D6.f1=_tmp585;});_tmp4D6;});void*_tmp62[2U];_tmp62[0]=& _tmp64,_tmp62[1]=& _tmp65;({unsigned int _tmp587=e->loc;struct _dyneither_ptr _tmp586=({const char*_tmp63="descriptor has type %s but argument has type %s";_tag_dyneither(_tmp63,sizeof(char),48U);});Cyc_Tcutil_terr(_tmp587,_tmp586,_tag_dyneither(_tmp62,sizeof(void*),2U));});});
Cyc_Tcutil_explain_failure();}
# 373
if(alias_coercion)
({struct Cyc_List_List*_tmp588=({struct Cyc_List_List*_tmp66=_cycalloc(sizeof(*_tmp66));_tmp66->hd=(void*)arg_cnt,_tmp66->tl=*alias_arg_exps;_tmp66;});*alias_arg_exps=_tmp588;});
if(Cyc_Tcutil_is_noalias_pointer_or_aggr(t) && !
Cyc_Tcutil_is_noalias_path(e))
({void*_tmp67=0U;({unsigned int _tmp58A=((struct Cyc_Absyn_Exp*)_tmp61->hd)->loc;struct _dyneither_ptr _tmp589=({const char*_tmp68="Cannot consume non-unique paths; do swap instead";_tag_dyneither(_tmp68,sizeof(char),49U);});Cyc_Tcutil_terr(_tmp58A,_tmp589,_tag_dyneither(_tmp67,sizeof(void*),0U));});});}
# 380
if(desc_types != 0)
({void*_tmp69=0U;({unsigned int _tmp58C=fmt->loc;struct _dyneither_ptr _tmp58B=({const char*_tmp6A="too few arguments";_tag_dyneither(_tmp6A,sizeof(char),18U);});Cyc_Tcutil_terr(_tmp58C,_tmp58B,_tag_dyneither(_tmp69,sizeof(void*),0U));});});
if(_tmp61 != 0){
({void*_tmp6B=0U;({unsigned int _tmp58E=((struct Cyc_Absyn_Exp*)_tmp61->hd)->loc;struct _dyneither_ptr _tmp58D=({const char*_tmp6C="too many arguments";_tag_dyneither(_tmp6C,sizeof(char),19U);});Cyc_Tcutil_terr(_tmp58E,_tmp58D,_tag_dyneither(_tmp6B,sizeof(void*),0U));});});
# 385
for(0;_tmp61 != 0;_tmp61=_tmp61->tl){
Cyc_Tcexp_tcExp(te,0,(struct Cyc_Absyn_Exp*)_tmp61->hd);}}}}
# 390
static void*Cyc_Tcexp_tcUnPrimop(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,enum Cyc_Absyn_Primop p,struct Cyc_Absyn_Exp*e){
# 392
void*t=Cyc_Tcutil_compress((void*)_check_null(e->topt));
enum Cyc_Absyn_Primop _tmp6D=p;switch(_tmp6D){case Cyc_Absyn_Plus: _LL1: _LL2:
 goto _LL4;case Cyc_Absyn_Minus: _LL3: _LL4:
# 396
 if(!Cyc_Tcutil_is_numeric(e))
({struct Cyc_String_pa_PrintArg_struct _tmp70=({struct Cyc_String_pa_PrintArg_struct _tmp4D8;_tmp4D8.tag=0U,({struct _dyneither_ptr _tmp58F=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp4D8.f1=_tmp58F;});_tmp4D8;});void*_tmp6E[1U];_tmp6E[0]=& _tmp70;({unsigned int _tmp591=loc;struct _dyneither_ptr _tmp590=({const char*_tmp6F="expecting numeric type but found %s";_tag_dyneither(_tmp6F,sizeof(char),36U);});Cyc_Tcutil_terr(_tmp591,_tmp590,_tag_dyneither(_tmp6E,sizeof(void*),1U));});});
return(void*)_check_null(e->topt);case Cyc_Absyn_Not: _LL5: _LL6:
# 400
 Cyc_Tcexp_check_contains_assign(e);
if(!Cyc_Tcutil_coerce_to_bool(te,e))
({struct Cyc_String_pa_PrintArg_struct _tmp73=({struct Cyc_String_pa_PrintArg_struct _tmp4D9;_tmp4D9.tag=0U,({struct _dyneither_ptr _tmp592=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp4D9.f1=_tmp592;});_tmp4D9;});void*_tmp71[1U];_tmp71[0]=& _tmp73;({unsigned int _tmp594=loc;struct _dyneither_ptr _tmp593=({const char*_tmp72="expecting integral or * type but found %s";_tag_dyneither(_tmp72,sizeof(char),42U);});Cyc_Tcutil_terr(_tmp594,_tmp593,_tag_dyneither(_tmp71,sizeof(void*),1U));});});
return Cyc_Absyn_sint_type;case Cyc_Absyn_Bitnot: _LL7: _LL8:
# 405
 if(!Cyc_Tcutil_is_integral(e))
({struct Cyc_String_pa_PrintArg_struct _tmp76=({struct Cyc_String_pa_PrintArg_struct _tmp4DA;_tmp4DA.tag=0U,({struct _dyneither_ptr _tmp595=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp4DA.f1=_tmp595;});_tmp4DA;});void*_tmp74[1U];_tmp74[0]=& _tmp76;({unsigned int _tmp597=loc;struct _dyneither_ptr _tmp596=({const char*_tmp75="expecting integral type but found %s";_tag_dyneither(_tmp75,sizeof(char),37U);});Cyc_Tcutil_terr(_tmp597,_tmp596,_tag_dyneither(_tmp74,sizeof(void*),1U));});});
return(void*)_check_null(e->topt);case Cyc_Absyn_Numelts: _LL9: _LLA:
# 409
{void*_tmp77=t;void*_tmp7F;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp77)->tag == 3U){_LLE: _tmp7F=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp77)->f1).ptr_atts).bounds;_LLF: {
# 411
struct Cyc_Absyn_Exp*_tmp78=Cyc_Tcutil_get_bounds_exp(Cyc_Absyn_fat_bound_type,_tmp7F);
if(_tmp78 != 0){
struct Cyc_Absyn_Exp*_tmp79=_tmp78;
if(!Cyc_Evexp_c_can_eval(_tmp79) && !((unsigned int)Cyc_Tcenv_allow_valueof))
({void*_tmp7A=0U;({unsigned int _tmp599=loc;struct _dyneither_ptr _tmp598=({const char*_tmp7B="cannot apply numelts to a pointer with abstract bounds";_tag_dyneither(_tmp7B,sizeof(char),55U);});Cyc_Tcutil_terr(_tmp599,_tmp598,_tag_dyneither(_tmp7A,sizeof(void*),0U));});});}
# 417
goto _LLD;}}else{_LL10: _LL11:
# 419
({struct Cyc_String_pa_PrintArg_struct _tmp7E=({struct Cyc_String_pa_PrintArg_struct _tmp4DB;_tmp4DB.tag=0U,({struct _dyneither_ptr _tmp59A=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp4DB.f1=_tmp59A;});_tmp4DB;});void*_tmp7C[1U];_tmp7C[0]=& _tmp7E;({unsigned int _tmp59C=loc;struct _dyneither_ptr _tmp59B=({const char*_tmp7D="numelts requires pointer type, not %s";_tag_dyneither(_tmp7D,sizeof(char),38U);});Cyc_Tcutil_terr(_tmp59C,_tmp59B,_tag_dyneither(_tmp7C,sizeof(void*),1U));});});}_LLD:;}
# 421
return Cyc_Absyn_uint_type;default: _LLB: _LLC:
({void*_tmp80=0U;({struct _dyneither_ptr _tmp59D=({const char*_tmp81="Non-unary primop";_tag_dyneither(_tmp81,sizeof(char),17U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp59D,_tag_dyneither(_tmp80,sizeof(void*),0U));});});}_LL0:;}
# 427
static void*Cyc_Tcexp_tcArithBinop(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,int(*checker)(struct Cyc_Absyn_Exp*)){
# 430
if(!checker(e1)){
({struct Cyc_String_pa_PrintArg_struct _tmp84=({struct Cyc_String_pa_PrintArg_struct _tmp4DC;_tmp4DC.tag=0U,({struct _dyneither_ptr _tmp59E=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(e1->topt)));_tmp4DC.f1=_tmp59E;});_tmp4DC;});void*_tmp82[1U];_tmp82[0]=& _tmp84;({unsigned int _tmp5A0=e1->loc;struct _dyneither_ptr _tmp59F=({const char*_tmp83="type %s cannot be used here";_tag_dyneither(_tmp83,sizeof(char),28U);});Cyc_Tcutil_terr(_tmp5A0,_tmp59F,_tag_dyneither(_tmp82,sizeof(void*),1U));});});
return Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te));}
# 434
if(!checker(e2)){
({struct Cyc_String_pa_PrintArg_struct _tmp87=({struct Cyc_String_pa_PrintArg_struct _tmp4DD;_tmp4DD.tag=0U,({struct _dyneither_ptr _tmp5A1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(e2->topt)));_tmp4DD.f1=_tmp5A1;});_tmp4DD;});void*_tmp85[1U];_tmp85[0]=& _tmp87;({unsigned int _tmp5A3=e2->loc;struct _dyneither_ptr _tmp5A2=({const char*_tmp86="type %s cannot be used here";_tag_dyneither(_tmp86,sizeof(char),28U);});Cyc_Tcutil_terr(_tmp5A3,_tmp5A2,_tag_dyneither(_tmp85,sizeof(void*),1U));});});
return Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te));}{
# 438
void*t1=Cyc_Tcutil_compress((void*)_check_null(e1->topt));
void*t2=Cyc_Tcutil_compress((void*)_check_null(e2->topt));
return Cyc_Tcutil_max_arithmetic_type(t1,t2);};}
# 443
static void*Cyc_Tcexp_tcPlus(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){
void*t1=Cyc_Tcutil_compress((void*)_check_null(e1->topt));
void*t2=Cyc_Tcutil_compress((void*)_check_null(e2->topt));
void*_tmp88=t1;void*_tmp9F;struct Cyc_Absyn_Tqual _tmp9E;void*_tmp9D;void*_tmp9C;void*_tmp9B;void*_tmp9A;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp88)->tag == 3U){_LL1: _tmp9F=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp88)->f1).elt_type;_tmp9E=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp88)->f1).elt_tq;_tmp9D=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp88)->f1).ptr_atts).rgn;_tmp9C=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp88)->f1).ptr_atts).nullable;_tmp9B=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp88)->f1).ptr_atts).bounds;_tmp9A=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp88)->f1).ptr_atts).zero_term;_LL2:
# 448
 if(!Cyc_Tcutil_kind_leq(Cyc_Tcutil_type_kind(_tmp9F),& Cyc_Tcutil_tmk))
({void*_tmp89=0U;({unsigned int _tmp5A5=e1->loc;struct _dyneither_ptr _tmp5A4=({const char*_tmp8A="can't perform arithmetic on abstract pointer type";_tag_dyneither(_tmp8A,sizeof(char),50U);});Cyc_Tcutil_terr(_tmp5A5,_tmp5A4,_tag_dyneither(_tmp89,sizeof(void*),0U));});});
if(Cyc_Tcutil_is_noalias_pointer(t1,0))
({void*_tmp8B=0U;({unsigned int _tmp5A7=e1->loc;struct _dyneither_ptr _tmp5A6=({const char*_tmp8C="can't perform arithmetic on non-aliasing pointer type";_tag_dyneither(_tmp8C,sizeof(char),54U);});Cyc_Tcutil_terr(_tmp5A7,_tmp5A6,_tag_dyneither(_tmp8B,sizeof(void*),0U));});});
if(!Cyc_Tcutil_coerce_sint_type(te,e2))
({struct Cyc_String_pa_PrintArg_struct _tmp8F=({struct Cyc_String_pa_PrintArg_struct _tmp4DE;_tmp4DE.tag=0U,({struct _dyneither_ptr _tmp5A8=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t2));_tmp4DE.f1=_tmp5A8;});_tmp4DE;});void*_tmp8D[1U];_tmp8D[0]=& _tmp8F;({unsigned int _tmp5AA=e2->loc;struct _dyneither_ptr _tmp5A9=({const char*_tmp8E="expecting int but found %s";_tag_dyneither(_tmp8E,sizeof(char),27U);});Cyc_Tcutil_terr(_tmp5AA,_tmp5A9,_tag_dyneither(_tmp8D,sizeof(void*),1U));});});{
struct Cyc_Absyn_Exp*_tmp90=Cyc_Tcutil_get_bounds_exp(Cyc_Absyn_fat_bound_type,_tmp9B);
if(_tmp90 == 0)return t1;{
struct Cyc_Absyn_Exp*_tmp91=_tmp90;
# 459
if(Cyc_Tcutil_force_type2bool(0,_tmp9A)){
struct _tuple15 _tmp92=Cyc_Evexp_eval_const_uint_exp(_tmp91);struct _tuple15 _tmp93=_tmp92;unsigned int _tmp97;int _tmp96;_LL6: _tmp97=_tmp93.f1;_tmp96=_tmp93.f2;_LL7:;
if(_tmp96  && _tmp97 == 1)
({void*_tmp94=0U;({unsigned int _tmp5AC=e1->loc;struct _dyneither_ptr _tmp5AB=({const char*_tmp95="pointer arithmetic on thin, zero-terminated pointer may be expensive.";_tag_dyneither(_tmp95,sizeof(char),70U);});Cyc_Tcutil_warn(_tmp5AC,_tmp5AB,_tag_dyneither(_tmp94,sizeof(void*),0U));});});}{
# 470
struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp98=({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp99=_cycalloc(sizeof(*_tmp99));_tmp99->tag=3U,(_tmp99->f1).elt_type=_tmp9F,(_tmp99->f1).elt_tq=_tmp9E,
((_tmp99->f1).ptr_atts).rgn=_tmp9D,((_tmp99->f1).ptr_atts).nullable=Cyc_Absyn_true_type,((_tmp99->f1).ptr_atts).bounds=Cyc_Absyn_fat_bound_type,((_tmp99->f1).ptr_atts).zero_term=_tmp9A,((_tmp99->f1).ptr_atts).ptrloc=0;_tmp99;});
# 474
Cyc_Tcutil_unchecked_cast(te,e1,(void*)_tmp98,Cyc_Absyn_Other_coercion);
return(void*)_tmp98;};};};}else{_LL3: _LL4:
 return Cyc_Tcexp_tcArithBinop(te,e1,e2,Cyc_Tcutil_is_numeric);}_LL0:;}
# 481
static void*Cyc_Tcexp_tcMinus(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){
void*t1=(void*)_check_null(e1->topt);
void*t2=(void*)_check_null(e2->topt);
void*t1_elt=Cyc_Absyn_void_type;
void*t2_elt=Cyc_Absyn_void_type;
if(Cyc_Tcutil_is_tagged_pointer_type_elt(t1,& t1_elt)){
if(Cyc_Tcutil_is_tagged_pointer_type_elt(t2,& t2_elt)){
if(!Cyc_Tcutil_unify(t1_elt,t2_elt)){
({struct Cyc_String_pa_PrintArg_struct _tmpA2=({struct Cyc_String_pa_PrintArg_struct _tmp4E0;_tmp4E0.tag=0U,({
# 491
struct _dyneither_ptr _tmp5AD=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t1));_tmp4E0.f1=_tmp5AD;});_tmp4E0;});struct Cyc_String_pa_PrintArg_struct _tmpA3=({struct Cyc_String_pa_PrintArg_struct _tmp4DF;_tmp4DF.tag=0U,({struct _dyneither_ptr _tmp5AE=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t2));_tmp4DF.f1=_tmp5AE;});_tmp4DF;});void*_tmpA0[2U];_tmpA0[0]=& _tmpA2,_tmpA0[1]=& _tmpA3;({unsigned int _tmp5B0=e1->loc;struct _dyneither_ptr _tmp5AF=({const char*_tmpA1="pointer arithmetic on values of different types (%s != %s)";_tag_dyneither(_tmpA1,sizeof(char),59U);});Cyc_Tcutil_terr(_tmp5B0,_tmp5AF,_tag_dyneither(_tmpA0,sizeof(void*),2U));});});
Cyc_Tcutil_explain_failure();}
# 494
return Cyc_Absyn_sint_type;}else{
if(Cyc_Tcutil_is_pointer_type(t2)){
if(!({void*_tmp5B1=t1_elt;Cyc_Tcutil_unify(_tmp5B1,Cyc_Tcutil_pointer_elt_type(t2));})){
({struct Cyc_String_pa_PrintArg_struct _tmpA6=({struct Cyc_String_pa_PrintArg_struct _tmp4E2;_tmp4E2.tag=0U,({
# 499
struct _dyneither_ptr _tmp5B2=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t1));_tmp4E2.f1=_tmp5B2;});_tmp4E2;});struct Cyc_String_pa_PrintArg_struct _tmpA7=({struct Cyc_String_pa_PrintArg_struct _tmp4E1;_tmp4E1.tag=0U,({struct _dyneither_ptr _tmp5B3=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t2));_tmp4E1.f1=_tmp5B3;});_tmp4E1;});void*_tmpA4[2U];_tmpA4[0]=& _tmpA6,_tmpA4[1]=& _tmpA7;({unsigned int _tmp5B5=e1->loc;struct _dyneither_ptr _tmp5B4=({const char*_tmpA5="pointer arithmetic on values of different types (%s != %s)";_tag_dyneither(_tmpA5,sizeof(char),59U);});Cyc_Tcutil_terr(_tmp5B5,_tmp5B4,_tag_dyneither(_tmpA4,sizeof(void*),2U));});});
Cyc_Tcutil_explain_failure();}
# 503
({void*_tmpA8=0U;({unsigned int _tmp5B7=e1->loc;struct _dyneither_ptr _tmp5B6=({const char*_tmpA9="coercing fat pointer to thin pointer to support subtraction";_tag_dyneither(_tmpA9,sizeof(char),60U);});Cyc_Tcutil_warn(_tmp5B7,_tmp5B6,_tag_dyneither(_tmpA8,sizeof(void*),0U));});});
({struct Cyc_Tcenv_Tenv*_tmp5BC=te;struct Cyc_Absyn_Exp*_tmp5BB=e1;Cyc_Tcutil_unchecked_cast(_tmp5BC,_tmp5BB,({void*_tmp5BA=t1_elt;void*_tmp5B9=Cyc_Tcutil_pointer_region(t1);struct Cyc_Absyn_Tqual _tmp5B8=
Cyc_Absyn_empty_tqual(0U);
# 504
Cyc_Absyn_star_type(_tmp5BA,_tmp5B9,_tmp5B8,Cyc_Absyn_false_type);}),Cyc_Absyn_Other_coercion);});
# 507
return Cyc_Absyn_sint_type;}else{
# 509
if(!Cyc_Tcutil_kind_leq(Cyc_Tcutil_type_kind(t1_elt),& Cyc_Tcutil_tmk))
({void*_tmpAA=0U;({unsigned int _tmp5BE=e1->loc;struct _dyneither_ptr _tmp5BD=({const char*_tmpAB="can't perform arithmetic on abstract pointer type";_tag_dyneither(_tmpAB,sizeof(char),50U);});Cyc_Tcutil_terr(_tmp5BE,_tmp5BD,_tag_dyneither(_tmpAA,sizeof(void*),0U));});});
if(Cyc_Tcutil_is_noalias_pointer(t1,0))
({void*_tmpAC=0U;({unsigned int _tmp5C0=e1->loc;struct _dyneither_ptr _tmp5BF=({const char*_tmpAD="can't perform arithmetic on non-aliasing pointer type";_tag_dyneither(_tmpAD,sizeof(char),54U);});Cyc_Tcutil_terr(_tmp5C0,_tmp5BF,_tag_dyneither(_tmpAC,sizeof(void*),0U));});});
if(!Cyc_Tcutil_coerce_sint_type(te,e2)){
({struct Cyc_String_pa_PrintArg_struct _tmpB0=({struct Cyc_String_pa_PrintArg_struct _tmp4E4;_tmp4E4.tag=0U,({
struct _dyneither_ptr _tmp5C1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t1));_tmp4E4.f1=_tmp5C1;});_tmp4E4;});struct Cyc_String_pa_PrintArg_struct _tmpB1=({struct Cyc_String_pa_PrintArg_struct _tmp4E3;_tmp4E3.tag=0U,({struct _dyneither_ptr _tmp5C2=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t2));_tmp4E3.f1=_tmp5C2;});_tmp4E3;});void*_tmpAE[2U];_tmpAE[0]=& _tmpB0,_tmpAE[1]=& _tmpB1;({unsigned int _tmp5C4=e2->loc;struct _dyneither_ptr _tmp5C3=({const char*_tmpAF="expecting either %s or int but found %s";_tag_dyneither(_tmpAF,sizeof(char),40U);});Cyc_Tcutil_terr(_tmp5C4,_tmp5C3,_tag_dyneither(_tmpAE,sizeof(void*),2U));});});
Cyc_Tcutil_explain_failure();}
# 518
return t1;}}}
# 522
if(Cyc_Tcutil_is_pointer_type(t1)){
if(Cyc_Tcutil_is_pointer_type(t2) && ({void*_tmp5C5=Cyc_Tcutil_pointer_elt_type(t1);Cyc_Tcutil_unify(_tmp5C5,
Cyc_Tcutil_pointer_elt_type(t2));})){
if(Cyc_Tcutil_is_tagged_pointer_type(t2)){
({void*_tmpB2=0U;({unsigned int _tmp5C7=e1->loc;struct _dyneither_ptr _tmp5C6=({const char*_tmpB3="coercing fat pointer to thin pointer to support subtraction";_tag_dyneither(_tmpB3,sizeof(char),60U);});Cyc_Tcutil_warn(_tmp5C7,_tmp5C6,_tag_dyneither(_tmpB2,sizeof(void*),0U));});});
({struct Cyc_Tcenv_Tenv*_tmp5CC=te;struct Cyc_Absyn_Exp*_tmp5CB=e2;Cyc_Tcutil_unchecked_cast(_tmp5CC,_tmp5CB,({void*_tmp5CA=Cyc_Tcutil_pointer_elt_type(t2);void*_tmp5C9=
Cyc_Tcutil_pointer_region(t2);
# 527
struct Cyc_Absyn_Tqual _tmp5C8=
# 529
Cyc_Absyn_empty_tqual(0U);
# 527
Cyc_Absyn_star_type(_tmp5CA,_tmp5C9,_tmp5C8,Cyc_Absyn_false_type);}),Cyc_Absyn_Other_coercion);});}
# 532
({void*_tmpB4=0U;({unsigned int _tmp5CE=e1->loc;struct _dyneither_ptr _tmp5CD=({const char*_tmpB5="thin pointer subtraction!";_tag_dyneither(_tmpB5,sizeof(char),26U);});Cyc_Tcutil_warn(_tmp5CE,_tmp5CD,_tag_dyneither(_tmpB4,sizeof(void*),0U));});});
return Cyc_Absyn_sint_type;}
# 535
({void*_tmpB6=0U;({unsigned int _tmp5D0=e1->loc;struct _dyneither_ptr _tmp5CF=({const char*_tmpB7="coercing thin pointer to integer to support subtraction";_tag_dyneither(_tmpB7,sizeof(char),56U);});Cyc_Tcutil_warn(_tmp5D0,_tmp5CF,_tag_dyneither(_tmpB6,sizeof(void*),0U));});});
Cyc_Tcutil_unchecked_cast(te,e1,Cyc_Absyn_sint_type,Cyc_Absyn_Other_coercion);}
# 538
if(Cyc_Tcutil_is_pointer_type(t2)){
({void*_tmpB8=0U;({unsigned int _tmp5D2=e1->loc;struct _dyneither_ptr _tmp5D1=({const char*_tmpB9="coercing pointer to integer to support subtraction";_tag_dyneither(_tmpB9,sizeof(char),51U);});Cyc_Tcutil_warn(_tmp5D2,_tmp5D1,_tag_dyneither(_tmpB8,sizeof(void*),0U));});});
Cyc_Tcutil_unchecked_cast(te,e2,Cyc_Absyn_sint_type,Cyc_Absyn_Other_coercion);}
# 543
return Cyc_Tcexp_tcArithBinop(te,e1,e2,Cyc_Tcutil_is_numeric);}
# 546
static void*Cyc_Tcexp_tcAnyBinop(struct Cyc_Tcenv_Tenv*te,unsigned int loc,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){
int e1_is_num=Cyc_Tcutil_is_numeric(e1);
int e2_is_num=Cyc_Tcutil_is_numeric(e2);
void*t1=Cyc_Tcutil_compress((void*)_check_null(e1->topt));
void*t2=Cyc_Tcutil_compress((void*)_check_null(e2->topt));
if(e1_is_num  && e2_is_num)
return Cyc_Absyn_sint_type;else{
# 554
if((Cyc_Tcutil_type_kind(t1))->kind == Cyc_Absyn_BoxKind  || ({
void*_tmp5D3=t1;Cyc_Tcutil_unify(_tmp5D3,Cyc_Absyn_new_evar(& Cyc_Tcutil_bko,Cyc_Tcenv_lookup_opt_type_vars(te)));})){
if(Cyc_Tcutil_unify(t1,t2))
return Cyc_Absyn_sint_type;else{
# 559
if(Cyc_Tcutil_silent_castable(te,loc,t2,t1)){
Cyc_Tcutil_unchecked_cast(te,e2,t1,Cyc_Absyn_Other_coercion);
return Cyc_Absyn_sint_type;}else{
if(Cyc_Tcutil_silent_castable(te,loc,t1,t2)){
Cyc_Tcutil_unchecked_cast(te,e1,t2,Cyc_Absyn_Other_coercion);
return Cyc_Absyn_sint_type;}else{
if(Cyc_Tcutil_zero_to_null(te,t2,e1) || Cyc_Tcutil_zero_to_null(te,t1,e2))
return Cyc_Absyn_sint_type;else{
goto pointer_cmp;}}}}}else{
# 572
pointer_cmp: {
struct _tuple0 _tmpBA=({struct _tuple0 _tmp4E5;({void*_tmp5D5=Cyc_Tcutil_compress(t1);_tmp4E5.f1=_tmp5D5;}),({void*_tmp5D4=Cyc_Tcutil_compress(t2);_tmp4E5.f2=_tmp5D4;});_tmp4E5;});struct _tuple0 _tmpBB=_tmpBA;void*_tmpBD;void*_tmpBC;switch(*((int*)_tmpBB.f1)){case 3U: if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmpBB.f2)->tag == 3U){_LL1: _tmpBD=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmpBB.f1)->f1).elt_type;_tmpBC=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmpBB.f2)->f1).elt_type;_LL2:
# 576
 if(Cyc_Tcutil_unify(_tmpBD,_tmpBC))return Cyc_Absyn_sint_type;goto _LL0;}else{goto _LL5;}case 0U: if(((struct Cyc_Absyn_RgnHandleCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmpBB.f1)->f1)->tag == 3U){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmpBB.f2)->tag == 0U){if(((struct Cyc_Absyn_RgnHandleCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmpBB.f2)->f1)->tag == 3U){_LL3: _LL4:
# 578
 return Cyc_Absyn_sint_type;}else{goto _LL5;}}else{goto _LL5;}}else{goto _LL5;}default: _LL5: _LL6:
 goto _LL0;}_LL0:;}
# 581
({struct Cyc_String_pa_PrintArg_struct _tmpC0=({struct Cyc_String_pa_PrintArg_struct _tmp4E7;_tmp4E7.tag=0U,({
struct _dyneither_ptr _tmp5D6=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t1));_tmp4E7.f1=_tmp5D6;});_tmp4E7;});struct Cyc_String_pa_PrintArg_struct _tmpC1=({struct Cyc_String_pa_PrintArg_struct _tmp4E6;_tmp4E6.tag=0U,({struct _dyneither_ptr _tmp5D7=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t2));_tmp4E6.f1=_tmp5D7;});_tmp4E6;});void*_tmpBE[2U];_tmpBE[0]=& _tmpC0,_tmpBE[1]=& _tmpC1;({unsigned int _tmp5D9=loc;struct _dyneither_ptr _tmp5D8=({const char*_tmpBF="comparison not allowed between %s and %s";_tag_dyneither(_tmpBF,sizeof(char),41U);});Cyc_Tcutil_terr(_tmp5D9,_tmp5D8,_tag_dyneither(_tmpBE,sizeof(void*),2U));});});
Cyc_Tcutil_explain_failure();
return Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te));}}}
# 588
static void*Cyc_Tcexp_tcEqPrimop(struct Cyc_Tcenv_Tenv*te,unsigned int loc,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){
# 590
if(({void*_tmp5DA=Cyc_Tcexp_tcAnyBinop(te,loc,e1,e2);_tmp5DA == Cyc_Absyn_sint_type;}))
return Cyc_Absyn_sint_type;
({struct Cyc_String_pa_PrintArg_struct _tmpC4=({struct Cyc_String_pa_PrintArg_struct _tmp4E9;_tmp4E9.tag=0U,({
struct _dyneither_ptr _tmp5DB=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(e1->topt)));_tmp4E9.f1=_tmp5DB;});_tmp4E9;});struct Cyc_String_pa_PrintArg_struct _tmpC5=({struct Cyc_String_pa_PrintArg_struct _tmp4E8;_tmp4E8.tag=0U,({struct _dyneither_ptr _tmp5DC=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(e2->topt)));_tmp4E8.f1=_tmp5DC;});_tmp4E8;});void*_tmpC2[2U];_tmpC2[0]=& _tmpC4,_tmpC2[1]=& _tmpC5;({unsigned int _tmp5DE=loc;struct _dyneither_ptr _tmp5DD=({const char*_tmpC3="comparison not allowed between %s and %s";_tag_dyneither(_tmpC3,sizeof(char),41U);});Cyc_Tcutil_terr(_tmp5DE,_tmp5DD,_tag_dyneither(_tmpC2,sizeof(void*),2U));});});
Cyc_Tcutil_explain_failure();
return Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te));}
# 600
static void*Cyc_Tcexp_tcBinPrimop(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,enum Cyc_Absyn_Primop p,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){
# 602
enum Cyc_Absyn_Primop _tmpC6=p;switch(_tmpC6){case Cyc_Absyn_Plus: _LL1: _LL2:
 return Cyc_Tcexp_tcPlus(te,e1,e2);case Cyc_Absyn_Minus: _LL3: _LL4:
 return Cyc_Tcexp_tcMinus(te,e1,e2);case Cyc_Absyn_Times: _LL5: _LL6:
# 606
 goto _LL8;case Cyc_Absyn_Div: _LL7: _LL8:
 return Cyc_Tcexp_tcArithBinop(te,e1,e2,Cyc_Tcutil_is_numeric);case Cyc_Absyn_Mod: _LL9: _LLA:
# 609
 goto _LLC;case Cyc_Absyn_Bitand: _LLB: _LLC:
 goto _LLE;case Cyc_Absyn_Bitor: _LLD: _LLE:
 goto _LL10;case Cyc_Absyn_Bitxor: _LLF: _LL10:
 goto _LL12;case Cyc_Absyn_Bitlshift: _LL11: _LL12:
 goto _LL14;case Cyc_Absyn_Bitlrshift: _LL13: _LL14:
 goto _LL16;case Cyc_Absyn_Bitarshift: _LL15: _LL16:
 return Cyc_Tcexp_tcArithBinop(te,e1,e2,Cyc_Tcutil_is_integral);case Cyc_Absyn_Eq: _LL17: _LL18:
# 619
 goto _LL1A;case Cyc_Absyn_Neq: _LL19: _LL1A:
 return Cyc_Tcexp_tcEqPrimop(te,loc,e1,e2);case Cyc_Absyn_Gt: _LL1B: _LL1C:
# 622
 goto _LL1E;case Cyc_Absyn_Lt: _LL1D: _LL1E:
 goto _LL20;case Cyc_Absyn_Gte: _LL1F: _LL20:
 goto _LL22;case Cyc_Absyn_Lte: _LL21: _LL22:
 return Cyc_Tcexp_tcAnyBinop(te,loc,e1,e2);default: _LL23: _LL24:
# 627
({void*_tmpC7=0U;({struct _dyneither_ptr _tmp5DF=({const char*_tmpC8="bad binary primop";_tag_dyneither(_tmpC8,sizeof(char),18U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp5DF,_tag_dyneither(_tmpC7,sizeof(void*),0U));});});}_LL0:;}
# 631
static void*Cyc_Tcexp_tcPrimop(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,enum Cyc_Absyn_Primop p,struct Cyc_List_List*es){
# 639
if(p == Cyc_Absyn_Minus  && ((int(*)(struct Cyc_List_List*x))Cyc_List_length)(es)== 1){
struct Cyc_Absyn_Exp*_tmpC9=(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(es))->hd;
void*_tmpCA=Cyc_Tcexp_tcExp(te,topt,_tmpC9);
if(!Cyc_Tcutil_is_numeric(_tmpC9))
({struct Cyc_String_pa_PrintArg_struct _tmpCD=({struct Cyc_String_pa_PrintArg_struct _tmp4EA;_tmp4EA.tag=0U,({struct _dyneither_ptr _tmp5E0=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmpCA));_tmp4EA.f1=_tmp5E0;});_tmp4EA;});void*_tmpCB[1U];_tmpCB[0]=& _tmpCD;({unsigned int _tmp5E2=_tmpC9->loc;struct _dyneither_ptr _tmp5E1=({const char*_tmpCC="expecting numeric type but found %s";_tag_dyneither(_tmpCC,sizeof(char),36U);});Cyc_Tcutil_terr(_tmp5E2,_tmp5E1,_tag_dyneither(_tmpCB,sizeof(void*),1U));});});
return _tmpCA;}
# 646
Cyc_Tcexp_tcExpList(te,es);{
void*t;
{int _tmpCE=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(es);int _tmpCF=_tmpCE;switch(_tmpCF){case 0U: _LL1: _LL2:
 return({void*_tmpD0=0U;({struct Cyc_Tcenv_Tenv*_tmp5E6=te;unsigned int _tmp5E5=loc;void**_tmp5E4=topt;struct _dyneither_ptr _tmp5E3=({const char*_tmpD1="primitive operator has 0 arguments";_tag_dyneither(_tmpD1,sizeof(char),35U);});Cyc_Tcexp_expr_err(_tmp5E6,_tmp5E5,_tmp5E4,_tmp5E3,_tag_dyneither(_tmpD0,sizeof(void*),0U));});});case 1U: _LL3: _LL4:
 t=Cyc_Tcexp_tcUnPrimop(te,loc,topt,p,(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(es))->hd);goto _LL0;case 2U: _LL5: _LL6:
 t=({struct Cyc_Tcenv_Tenv*_tmp5EB=te;unsigned int _tmp5EA=loc;void**_tmp5E9=topt;enum Cyc_Absyn_Primop _tmp5E8=p;struct Cyc_Absyn_Exp*_tmp5E7=(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(es))->hd;Cyc_Tcexp_tcBinPrimop(_tmp5EB,_tmp5EA,_tmp5E9,_tmp5E8,_tmp5E7,(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(es->tl))->hd);});goto _LL0;default: _LL7: _LL8:
 return({void*_tmpD2=0U;({struct Cyc_Tcenv_Tenv*_tmp5EF=te;unsigned int _tmp5EE=loc;void**_tmp5ED=topt;struct _dyneither_ptr _tmp5EC=({const char*_tmpD3="primitive operator has > 2 arguments";_tag_dyneither(_tmpD3,sizeof(char),37U);});Cyc_Tcexp_expr_err(_tmp5EF,_tmp5EE,_tmp5ED,_tmp5EC,_tag_dyneither(_tmpD2,sizeof(void*),0U));});});}_LL0:;}
# 654
return t;};}struct _tuple17{struct Cyc_Absyn_Tqual f1;void*f2;};
# 657
static int Cyc_Tcexp_check_writable_aggr(unsigned int loc,void*t){
void*_tmpD4=Cyc_Tcutil_compress(t);
void*_tmpD5=_tmpD4;struct Cyc_List_List*_tmpF1;void*_tmpF0;struct Cyc_Absyn_Tqual _tmpEF;struct Cyc_List_List*_tmpEE;struct Cyc_Absyn_Datatypefield*_tmpED;struct Cyc_Absyn_Aggrdecl*_tmpEC;switch(*((int*)_tmpD5)){case 0U: switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmpD5)->f1)){case 20U: if(((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmpD5)->f1)->f1).KnownAggr).tag == 2){_LL1: _tmpEC=*((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmpD5)->f1)->f1).KnownAggr).val;_LL2:
# 661
 if(_tmpEC->impl == 0){
({void*_tmpD6=0U;({unsigned int _tmp5F1=loc;struct _dyneither_ptr _tmp5F0=({const char*_tmpD7="attempt to write an abstract aggregate";_tag_dyneither(_tmpD7,sizeof(char),39U);});Cyc_Tcutil_terr(_tmp5F1,_tmp5F0,_tag_dyneither(_tmpD6,sizeof(void*),0U));});});
return 0;}else{
# 665
_tmpEE=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmpEC->impl))->fields;goto _LL4;}}else{goto _LLB;}case 19U: if(((((struct Cyc_Absyn_DatatypeFieldCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmpD5)->f1)->f1).KnownDatatypefield).tag == 2){_LL5: _tmpED=(((((struct Cyc_Absyn_DatatypeFieldCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmpD5)->f1)->f1).KnownDatatypefield).val).f2;_LL6:
# 677
{struct Cyc_List_List*_tmpDC=_tmpED->typs;for(0;_tmpDC != 0;_tmpDC=_tmpDC->tl){
struct _tuple17*_tmpDD=(struct _tuple17*)_tmpDC->hd;struct _tuple17*_tmpDE=_tmpDD;struct Cyc_Absyn_Tqual _tmpE3;void*_tmpE2;_LLE: _tmpE3=_tmpDE->f1;_tmpE2=_tmpDE->f2;_LLF:;
if(_tmpE3.real_const){
({struct Cyc_String_pa_PrintArg_struct _tmpE1=({struct Cyc_String_pa_PrintArg_struct _tmp4EB;_tmp4EB.tag=0U,({struct _dyneither_ptr _tmp5F2=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmpED->name));_tmp4EB.f1=_tmp5F2;});_tmp4EB;});void*_tmpDF[1U];_tmpDF[0]=& _tmpE1;({unsigned int _tmp5F4=loc;struct _dyneither_ptr _tmp5F3=({const char*_tmpE0="attempt to over-write a datatype field (%s) with a const member";_tag_dyneither(_tmpE0,sizeof(char),64U);});Cyc_Tcutil_terr(_tmp5F4,_tmp5F3,_tag_dyneither(_tmpDF,sizeof(void*),1U));});});
return 0;}
# 683
if(!Cyc_Tcexp_check_writable_aggr(loc,_tmpE2))return 0;}}
# 685
return 1;}else{goto _LLB;}default: goto _LLB;}case 7U: _LL3: _tmpEE=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmpD5)->f2;_LL4:
# 667
 for(0;_tmpEE != 0;_tmpEE=_tmpEE->tl){
struct Cyc_Absyn_Aggrfield*_tmpD8=(struct Cyc_Absyn_Aggrfield*)_tmpEE->hd;
if((_tmpD8->tq).real_const){
({struct Cyc_String_pa_PrintArg_struct _tmpDB=({struct Cyc_String_pa_PrintArg_struct _tmp4EC;_tmp4EC.tag=0U,_tmp4EC.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmpD8->name);_tmp4EC;});void*_tmpD9[1U];_tmpD9[0]=& _tmpDB;({unsigned int _tmp5F6=loc;struct _dyneither_ptr _tmp5F5=({const char*_tmpDA="attempt to over-write an aggregate with const member %s";_tag_dyneither(_tmpDA,sizeof(char),56U);});Cyc_Tcutil_terr(_tmp5F6,_tmp5F5,_tag_dyneither(_tmpD9,sizeof(void*),1U));});});
return 0;}
# 673
if(!Cyc_Tcexp_check_writable_aggr(loc,_tmpD8->type))return 0;}
# 675
return 1;case 4U: _LL7: _tmpF0=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmpD5)->f1).elt_type;_tmpEF=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmpD5)->f1).tq;_LL8:
# 687
 if(_tmpEF.real_const){
({void*_tmpE4=0U;({unsigned int _tmp5F8=loc;struct _dyneither_ptr _tmp5F7=({const char*_tmpE5="attempt to over-write a const array";_tag_dyneither(_tmpE5,sizeof(char),36U);});Cyc_Tcutil_terr(_tmp5F8,_tmp5F7,_tag_dyneither(_tmpE4,sizeof(void*),0U));});});
return 0;}
# 691
return Cyc_Tcexp_check_writable_aggr(loc,_tmpF0);case 6U: _LL9: _tmpF1=((struct Cyc_Absyn_TupleType_Absyn_Type_struct*)_tmpD5)->f1;_LLA:
# 693
 for(0;_tmpF1 != 0;_tmpF1=_tmpF1->tl){
struct _tuple17*_tmpE6=(struct _tuple17*)_tmpF1->hd;struct _tuple17*_tmpE7=_tmpE6;struct Cyc_Absyn_Tqual _tmpEB;void*_tmpEA;_LL11: _tmpEB=_tmpE7->f1;_tmpEA=_tmpE7->f2;_LL12:;
if(_tmpEB.real_const){
({void*_tmpE8=0U;({unsigned int _tmp5FA=loc;struct _dyneither_ptr _tmp5F9=({const char*_tmpE9="attempt to over-write a tuple field with a const member";_tag_dyneither(_tmpE9,sizeof(char),56U);});Cyc_Tcutil_terr(_tmp5FA,_tmp5F9,_tag_dyneither(_tmpE8,sizeof(void*),0U));});});
return 0;}
# 699
if(!Cyc_Tcexp_check_writable_aggr(loc,_tmpEA))return 0;}
# 701
return 1;default: _LLB: _LLC:
 return 1;}_LL0:;}
# 709
static void Cyc_Tcexp_check_writable(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e){
# 712
if(!Cyc_Tcexp_check_writable_aggr(e->loc,(void*)_check_null(e->topt)))return;
{void*_tmpF2=e->r;void*_tmpF3=_tmpF2;struct Cyc_Absyn_Exp*_tmp123;struct Cyc_Absyn_Exp*_tmp122;struct Cyc_Absyn_Exp*_tmp121;struct Cyc_Absyn_Exp*_tmp120;struct _dyneither_ptr*_tmp11F;struct Cyc_Absyn_Exp*_tmp11E;struct _dyneither_ptr*_tmp11D;struct Cyc_Absyn_Exp*_tmp11C;struct Cyc_Absyn_Exp*_tmp11B;struct Cyc_Absyn_Vardecl*_tmp11A;struct Cyc_Absyn_Vardecl*_tmp119;struct Cyc_Absyn_Vardecl*_tmp118;struct Cyc_Absyn_Vardecl*_tmp117;switch(*((int*)_tmpF3)){case 1U: switch(*((int*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmpF3)->f1)){case 3U: _LL1: _tmp117=((struct Cyc_Absyn_Param_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmpF3)->f1)->f1;_LL2:
 _tmp118=_tmp117;goto _LL4;case 4U: _LL3: _tmp118=((struct Cyc_Absyn_Local_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmpF3)->f1)->f1;_LL4:
 _tmp119=_tmp118;goto _LL6;case 5U: _LL5: _tmp119=((struct Cyc_Absyn_Pat_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmpF3)->f1)->f1;_LL6:
 _tmp11A=_tmp119;goto _LL8;case 1U: _LL7: _tmp11A=((struct Cyc_Absyn_Global_b_Absyn_Binding_struct*)((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmpF3)->f1)->f1;_LL8:
 if(!(_tmp11A->tq).real_const)return;goto _LL0;default: goto _LL15;}case 23U: _LL9: _tmp11C=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmpF3)->f1;_tmp11B=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmpF3)->f2;_LLA:
# 719
{void*_tmpF4=Cyc_Tcutil_compress((void*)_check_null(_tmp11C->topt));void*_tmpF5=_tmpF4;struct Cyc_List_List*_tmp106;struct Cyc_Absyn_Tqual _tmp105;struct Cyc_Absyn_Tqual _tmp104;switch(*((int*)_tmpF5)){case 3U: _LL18: _tmp104=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmpF5)->f1).elt_tq;_LL19:
 _tmp105=_tmp104;goto _LL1B;case 4U: _LL1A: _tmp105=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmpF5)->f1).tq;_LL1B:
 if(!_tmp105.real_const)return;goto _LL17;case 6U: _LL1C: _tmp106=((struct Cyc_Absyn_TupleType_Absyn_Type_struct*)_tmpF5)->f1;_LL1D: {
# 723
struct _tuple15 _tmpF6=Cyc_Evexp_eval_const_uint_exp(_tmp11B);struct _tuple15 _tmpF7=_tmpF6;unsigned int _tmp103;int _tmp102;_LL21: _tmp103=_tmpF7.f1;_tmp102=_tmpF7.f2;_LL22:;
if(!_tmp102){
({void*_tmpF8=0U;({unsigned int _tmp5FC=e->loc;struct _dyneither_ptr _tmp5FB=({const char*_tmpF9="tuple projection cannot use sizeof or offsetof";_tag_dyneither(_tmpF9,sizeof(char),47U);});Cyc_Tcutil_terr(_tmp5FC,_tmp5FB,_tag_dyneither(_tmpF8,sizeof(void*),0U));});});
return;}
# 728
{struct _handler_cons _tmpFA;_push_handler(& _tmpFA);{int _tmpFC=0;if(setjmp(_tmpFA.handler))_tmpFC=1;if(!_tmpFC){
{struct _tuple17*_tmpFD=((struct _tuple17*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)(_tmp106,(int)_tmp103);struct _tuple17*_tmpFE=_tmpFD;struct Cyc_Absyn_Tqual _tmpFF;_LL24: _tmpFF=_tmpFE->f1;_LL25:;
if(!_tmpFF.real_const){_npop_handler(0U);return;}}
# 729
;_pop_handler();}else{void*_tmpFB=(void*)_exn_thrown;void*_tmp100=_tmpFB;void*_tmp101;if(((struct Cyc_List_Nth_exn_struct*)_tmp100)->tag == Cyc_List_Nth){_LL27: _LL28:
# 731
 return;}else{_LL29: _tmp101=_tmp100;_LL2A:(int)_rethrow(_tmp101);}_LL26:;}};}
goto _LL17;}default: _LL1E: _LL1F:
 goto _LL17;}_LL17:;}
# 735
goto _LL0;case 21U: _LLB: _tmp11E=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmpF3)->f1;_tmp11D=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmpF3)->f2;_LLC:
# 737
{void*_tmp107=Cyc_Tcutil_compress((void*)_check_null(_tmp11E->topt));void*_tmp108=_tmp107;struct Cyc_List_List*_tmp10A;struct Cyc_Absyn_Aggrdecl**_tmp109;switch(*((int*)_tmp108)){case 0U: if(((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp108)->f1)->tag == 20U){if(((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp108)->f1)->f1).KnownAggr).tag == 2){_LL2C: _tmp109=((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp108)->f1)->f1).KnownAggr).val;_LL2D: {
# 739
struct Cyc_Absyn_Aggrfield*sf=
_tmp109 == 0?0: Cyc_Absyn_lookup_decl_field(*_tmp109,_tmp11D);
if(sf == 0  || !(sf->tq).real_const)return;
goto _LL2B;}}else{goto _LL30;}}else{goto _LL30;}case 7U: _LL2E: _tmp10A=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp108)->f2;_LL2F: {
# 744
struct Cyc_Absyn_Aggrfield*sf=Cyc_Absyn_lookup_field(_tmp10A,_tmp11D);
if(sf == 0  || !(sf->tq).real_const)return;
goto _LL2B;}default: _LL30: _LL31:
 goto _LL2B;}_LL2B:;}
# 749
goto _LL0;case 22U: _LLD: _tmp120=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmpF3)->f1;_tmp11F=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmpF3)->f2;_LLE:
# 751
{void*_tmp10B=Cyc_Tcutil_compress((void*)_check_null(_tmp120->topt));void*_tmp10C=_tmp10B;void*_tmp112;struct Cyc_Absyn_Tqual _tmp111;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp10C)->tag == 3U){_LL33: _tmp112=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp10C)->f1).elt_type;_tmp111=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp10C)->f1).elt_tq;_LL34:
# 753
 if(!_tmp111.real_const){
void*_tmp10D=Cyc_Tcutil_compress(_tmp112);void*_tmp10E=_tmp10D;struct Cyc_List_List*_tmp110;struct Cyc_Absyn_Aggrdecl**_tmp10F;switch(*((int*)_tmp10E)){case 0U: if(((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp10E)->f1)->tag == 20U){if(((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp10E)->f1)->f1).KnownAggr).tag == 2){_LL38: _tmp10F=((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp10E)->f1)->f1).KnownAggr).val;_LL39: {
# 756
struct Cyc_Absyn_Aggrfield*sf=
_tmp10F == 0?0: Cyc_Absyn_lookup_decl_field(*_tmp10F,_tmp11F);
if(sf == 0  || !(sf->tq).real_const)return;
goto _LL37;}}else{goto _LL3C;}}else{goto _LL3C;}case 7U: _LL3A: _tmp110=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp10E)->f2;_LL3B: {
# 761
struct Cyc_Absyn_Aggrfield*sf=Cyc_Absyn_lookup_field(_tmp110,_tmp11F);
if(sf == 0  || !(sf->tq).real_const)return;
goto _LL37;}default: _LL3C: _LL3D:
 goto _LL37;}_LL37:;}
# 767
goto _LL32;}else{_LL35: _LL36:
 goto _LL32;}_LL32:;}
# 770
goto _LL0;case 20U: _LLF: _tmp121=((struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmpF3)->f1;_LL10:
# 772
{void*_tmp113=Cyc_Tcutil_compress((void*)_check_null(_tmp121->topt));void*_tmp114=_tmp113;struct Cyc_Absyn_Tqual _tmp116;struct Cyc_Absyn_Tqual _tmp115;switch(*((int*)_tmp114)){case 3U: _LL3F: _tmp115=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp114)->f1).elt_tq;_LL40:
 _tmp116=_tmp115;goto _LL42;case 4U: _LL41: _tmp116=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp114)->f1).tq;_LL42:
 if(!_tmp116.real_const)return;goto _LL3E;default: _LL43: _LL44:
 goto _LL3E;}_LL3E:;}
# 777
goto _LL0;case 12U: _LL11: _tmp122=((struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct*)_tmpF3)->f1;_LL12:
 _tmp123=_tmp122;goto _LL14;case 13U: _LL13: _tmp123=((struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct*)_tmpF3)->f1;_LL14:
 Cyc_Tcexp_check_writable(te,_tmp123);return;default: _LL15: _LL16:
 goto _LL0;}_LL0:;}
# 782
({struct Cyc_String_pa_PrintArg_struct _tmp126=({struct Cyc_String_pa_PrintArg_struct _tmp4ED;_tmp4ED.tag=0U,({struct _dyneither_ptr _tmp5FD=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_exp2string(e));_tmp4ED.f1=_tmp5FD;});_tmp4ED;});void*_tmp124[1U];_tmp124[0]=& _tmp126;({unsigned int _tmp5FF=e->loc;struct _dyneither_ptr _tmp5FE=({const char*_tmp125="attempt to write a const location: %s";_tag_dyneither(_tmp125,sizeof(char),38U);});Cyc_Tcutil_terr(_tmp5FF,_tmp5FE,_tag_dyneither(_tmp124,sizeof(void*),1U));});});}
# 785
static void*Cyc_Tcexp_tcIncrement(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e,enum Cyc_Absyn_Incrementor i){
# 788
({struct Cyc_Tcenv_Tenv*_tmp600=Cyc_Tcenv_enter_lhs(te);Cyc_Tcexp_tcExpNoPromote(_tmp600,0,e);});
if(!Cyc_Absyn_is_lvalue(e))
return({void*_tmp127=0U;({struct Cyc_Tcenv_Tenv*_tmp604=te;unsigned int _tmp603=loc;void**_tmp602=topt;struct _dyneither_ptr _tmp601=({const char*_tmp128="increment/decrement of non-lvalue";_tag_dyneither(_tmp128,sizeof(char),34U);});Cyc_Tcexp_expr_err(_tmp604,_tmp603,_tmp602,_tmp601,_tag_dyneither(_tmp127,sizeof(void*),0U));});});
Cyc_Tcexp_check_writable(te,e);{
void*t=(void*)_check_null(e->topt);
# 794
if(!Cyc_Tcutil_is_numeric(e)){
void*telt=Cyc_Absyn_void_type;
if(Cyc_Tcutil_is_tagged_pointer_type_elt(t,& telt) || 
Cyc_Tcutil_is_zero_pointer_type_elt(t,& telt) && (i == Cyc_Absyn_PreInc  || i == Cyc_Absyn_PostInc)){
if(!Cyc_Tcutil_kind_leq(Cyc_Tcutil_type_kind(telt),& Cyc_Tcutil_tmk))
({void*_tmp129=0U;({unsigned int _tmp606=e->loc;struct _dyneither_ptr _tmp605=({const char*_tmp12A="can't perform arithmetic on abstract pointer type";_tag_dyneither(_tmp12A,sizeof(char),50U);});Cyc_Tcutil_terr(_tmp606,_tmp605,_tag_dyneither(_tmp129,sizeof(void*),0U));});});
if(Cyc_Tcutil_is_noalias_pointer(t,0))
({void*_tmp12B=0U;({unsigned int _tmp608=e->loc;struct _dyneither_ptr _tmp607=({const char*_tmp12C="can't perform arithmetic on non-aliasing pointer type";_tag_dyneither(_tmp12C,sizeof(char),54U);});Cyc_Tcutil_terr(_tmp608,_tmp607,_tag_dyneither(_tmp12B,sizeof(void*),0U));});});}else{
# 803
({struct Cyc_String_pa_PrintArg_struct _tmp12F=({struct Cyc_String_pa_PrintArg_struct _tmp4EE;_tmp4EE.tag=0U,({struct _dyneither_ptr _tmp609=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp4EE.f1=_tmp609;});_tmp4EE;});void*_tmp12D[1U];_tmp12D[0]=& _tmp12F;({unsigned int _tmp60B=e->loc;struct _dyneither_ptr _tmp60A=({const char*_tmp12E="expecting arithmetic or ? type but found %s";_tag_dyneither(_tmp12E,sizeof(char),44U);});Cyc_Tcutil_terr(_tmp60B,_tmp60A,_tag_dyneither(_tmp12D,sizeof(void*),1U));});});}}
# 805
return t;};}
# 809
static void*Cyc_Tcexp_tcConditional(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Absyn_Exp*e3){
# 811
({struct Cyc_Tcenv_Tenv*_tmp60D=Cyc_Tcenv_clear_abstract_val_ok(te);struct Cyc_Absyn_Exp*_tmp60C=e1;Cyc_Tcexp_tcTest(_tmp60D,_tmp60C,({const char*_tmp130="conditional expression";_tag_dyneither(_tmp130,sizeof(char),23U);}));});
Cyc_Tcexp_tcExp(te,topt,e2);
Cyc_Tcexp_tcExp(te,topt,e3);{
void*t;
if(Cyc_Tcenv_abstract_val_ok(te))
t=Cyc_Absyn_new_evar(& Cyc_Tcutil_tako,Cyc_Tcenv_lookup_opt_type_vars(te));else{
# 818
t=Cyc_Absyn_new_evar(& Cyc_Tcutil_tmko,Cyc_Tcenv_lookup_opt_type_vars(te));}{
struct Cyc_List_List _tmp131=({struct Cyc_List_List _tmp4F2;_tmp4F2.hd=e3,_tmp4F2.tl=0;_tmp4F2;});
struct Cyc_List_List _tmp132=({struct Cyc_List_List _tmp4F1;_tmp4F1.hd=e2,_tmp4F1.tl=& _tmp131;_tmp4F1;});
if(!Cyc_Tcutil_coerce_list(te,t,& _tmp132)){
({struct Cyc_String_pa_PrintArg_struct _tmp135=({struct Cyc_String_pa_PrintArg_struct _tmp4F0;_tmp4F0.tag=0U,({
struct _dyneither_ptr _tmp60E=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(e2->topt)));_tmp4F0.f1=_tmp60E;});_tmp4F0;});struct Cyc_String_pa_PrintArg_struct _tmp136=({struct Cyc_String_pa_PrintArg_struct _tmp4EF;_tmp4EF.tag=0U,({struct _dyneither_ptr _tmp60F=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(e3->topt)));_tmp4EF.f1=_tmp60F;});_tmp4EF;});void*_tmp133[2U];_tmp133[0]=& _tmp135,_tmp133[1]=& _tmp136;({unsigned int _tmp611=loc;struct _dyneither_ptr _tmp610=({const char*_tmp134="conditional clause types do not match: %s != %s";_tag_dyneither(_tmp134,sizeof(char),48U);});Cyc_Tcutil_terr(_tmp611,_tmp610,_tag_dyneither(_tmp133,sizeof(void*),2U));});});
Cyc_Tcutil_explain_failure();}
# 826
return t;};};}
# 830
static void*Cyc_Tcexp_tcAnd(struct Cyc_Tcenv_Tenv*te,unsigned int loc,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){
# 832
({struct Cyc_Tcenv_Tenv*_tmp613=te;struct Cyc_Absyn_Exp*_tmp612=e1;Cyc_Tcexp_tcTest(_tmp613,_tmp612,({const char*_tmp137="logical-and expression";_tag_dyneither(_tmp137,sizeof(char),23U);}));});
({struct Cyc_Tcenv_Tenv*_tmp615=te;struct Cyc_Absyn_Exp*_tmp614=e2;Cyc_Tcexp_tcTest(_tmp615,_tmp614,({const char*_tmp138="logical-and expression";_tag_dyneither(_tmp138,sizeof(char),23U);}));});
return Cyc_Absyn_sint_type;}
# 838
static void*Cyc_Tcexp_tcOr(struct Cyc_Tcenv_Tenv*te,unsigned int loc,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){
# 840
({struct Cyc_Tcenv_Tenv*_tmp617=te;struct Cyc_Absyn_Exp*_tmp616=e1;Cyc_Tcexp_tcTest(_tmp617,_tmp616,({const char*_tmp139="logical-or expression";_tag_dyneither(_tmp139,sizeof(char),22U);}));});
({struct Cyc_Tcenv_Tenv*_tmp619=te;struct Cyc_Absyn_Exp*_tmp618=e2;Cyc_Tcexp_tcTest(_tmp619,_tmp618,({const char*_tmp13A="logical-or expression";_tag_dyneither(_tmp13A,sizeof(char),22U);}));});
return Cyc_Absyn_sint_type;}
# 846
static void*Cyc_Tcexp_tcAssignOp(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e1,struct Cyc_Core_Opt*po,struct Cyc_Absyn_Exp*e2){
# 853
({struct Cyc_Tcenv_Tenv*_tmp61A=Cyc_Tcenv_enter_lhs(Cyc_Tcenv_enter_notreadctxt(te));Cyc_Tcexp_tcExpNoPromote(_tmp61A,0,e1);});{
void*t1=(void*)_check_null(e1->topt);
Cyc_Tcexp_tcExp(te,& t1,e2);{
void*t2=(void*)_check_null(e2->topt);
# 858
{void*_tmp13B=Cyc_Tcutil_compress(t1);void*_tmp13C=_tmp13B;if(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp13C)->tag == 4U){_LL1: _LL2:
({void*_tmp13D=0U;({unsigned int _tmp61C=loc;struct _dyneither_ptr _tmp61B=({const char*_tmp13E="cannot assign to an array";_tag_dyneither(_tmp13E,sizeof(char),26U);});Cyc_Tcutil_terr(_tmp61C,_tmp61B,_tag_dyneither(_tmp13D,sizeof(void*),0U));});});goto _LL0;}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 863
if(!Cyc_Tcutil_kind_leq(Cyc_Tcutil_type_kind(t1),& Cyc_Tcutil_tmk))
({void*_tmp13F=0U;({unsigned int _tmp61E=loc;struct _dyneither_ptr _tmp61D=({const char*_tmp140="type is abstract (can't determine size).";_tag_dyneither(_tmp140,sizeof(char),41U);});Cyc_Tcutil_terr(_tmp61E,_tmp61D,_tag_dyneither(_tmp13F,sizeof(void*),0U));});});
# 867
if(!Cyc_Absyn_is_lvalue(e1))
return({void*_tmp141=0U;({struct Cyc_Tcenv_Tenv*_tmp622=te;unsigned int _tmp621=loc;void**_tmp620=topt;struct _dyneither_ptr _tmp61F=({const char*_tmp142="assignment to non-lvalue";_tag_dyneither(_tmp142,sizeof(char),25U);});Cyc_Tcexp_expr_err(_tmp622,_tmp621,_tmp620,_tmp61F,_tag_dyneither(_tmp141,sizeof(void*),0U));});});
Cyc_Tcexp_check_writable(te,e1);
if(po == 0){
if(Cyc_Tcutil_is_noalias_pointer_or_aggr(t2) && !Cyc_Tcutil_is_noalias_path(e2))
({void*_tmp143=0U;({unsigned int _tmp624=e2->loc;struct _dyneither_ptr _tmp623=({const char*_tmp144="Cannot consume non-unique paths; do swap instead";_tag_dyneither(_tmp144,sizeof(char),49U);});Cyc_Tcutil_terr(_tmp624,_tmp623,_tag_dyneither(_tmp143,sizeof(void*),0U));});});
if(!Cyc_Tcutil_coerce_assign(te,e2,t1)){
void*_tmp145=({struct Cyc_String_pa_PrintArg_struct _tmp148=({struct Cyc_String_pa_PrintArg_struct _tmp4F4;_tmp4F4.tag=0U,({
struct _dyneither_ptr _tmp625=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t1));_tmp4F4.f1=_tmp625;});_tmp4F4;});struct Cyc_String_pa_PrintArg_struct _tmp149=({struct Cyc_String_pa_PrintArg_struct _tmp4F3;_tmp4F3.tag=0U,({struct _dyneither_ptr _tmp626=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t2));_tmp4F3.f1=_tmp626;});_tmp4F3;});void*_tmp146[2U];_tmp146[0]=& _tmp148,_tmp146[1]=& _tmp149;({struct Cyc_Tcenv_Tenv*_tmp62A=te;unsigned int _tmp629=loc;void**_tmp628=topt;struct _dyneither_ptr _tmp627=({const char*_tmp147="type mismatch: %s != %s";_tag_dyneither(_tmp147,sizeof(char),24U);});Cyc_Tcexp_expr_err(_tmp62A,_tmp629,_tmp628,_tmp627,_tag_dyneither(_tmp146,sizeof(void*),2U));});});
Cyc_Tcutil_unify(t1,t2);
Cyc_Tcutil_explain_failure();
return _tmp145;}}else{
# 881
enum Cyc_Absyn_Primop _tmp14A=(enum Cyc_Absyn_Primop)po->v;
void*_tmp14B=Cyc_Tcexp_tcBinPrimop(te,loc,0,_tmp14A,e1,e2);
if(!(Cyc_Tcutil_unify(_tmp14B,t1) || Cyc_Tcutil_is_arithmetic_type(_tmp14B) && Cyc_Tcutil_is_arithmetic_type(t1))){
void*_tmp14C=({struct Cyc_String_pa_PrintArg_struct _tmp14F=({struct Cyc_String_pa_PrintArg_struct _tmp4F6;_tmp4F6.tag=0U,({
# 886
struct _dyneither_ptr _tmp62B=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t1));_tmp4F6.f1=_tmp62B;});_tmp4F6;});struct Cyc_String_pa_PrintArg_struct _tmp150=({struct Cyc_String_pa_PrintArg_struct _tmp4F5;_tmp4F5.tag=0U,({
struct _dyneither_ptr _tmp62C=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t2));_tmp4F5.f1=_tmp62C;});_tmp4F5;});void*_tmp14D[2U];_tmp14D[0]=& _tmp14F,_tmp14D[1]=& _tmp150;({struct Cyc_Tcenv_Tenv*_tmp630=te;unsigned int _tmp62F=loc;void**_tmp62E=topt;struct _dyneither_ptr _tmp62D=({const char*_tmp14E="Cannot use this operator in an assignment when the arguments have types %s and %s";_tag_dyneither(_tmp14E,sizeof(char),82U);});Cyc_Tcexp_expr_err(_tmp630,_tmp62F,_tmp62E,_tmp62D,_tag_dyneither(_tmp14D,sizeof(void*),2U));});});
Cyc_Tcutil_unify(_tmp14B,t1);
Cyc_Tcutil_explain_failure();
return _tmp14C;}
# 892
return _tmp14B;}
# 894
return t1;};};}
# 898
static void*Cyc_Tcexp_tcSeqExp(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){
({struct Cyc_Tcenv_Tenv*_tmp631=Cyc_Tcenv_clear_abstract_val_ok(te);Cyc_Tcexp_tcExp(_tmp631,0,e1);});
({struct Cyc_Tcenv_Tenv*_tmp633=Cyc_Tcenv_clear_abstract_val_ok(te);void**_tmp632=topt;Cyc_Tcexp_tcExp(_tmp633,_tmp632,e2);});
return(void*)_check_null(e2->topt);}
# 905
static struct Cyc_Absyn_Datatypefield*Cyc_Tcexp_tcInjection(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e,void*tu,struct Cyc_List_List*inst,struct Cyc_List_List*fs){
# 908
struct Cyc_List_List*fields;
void*t1=(void*)_check_null(e->topt);
# 911
{void*_tmp151=Cyc_Tcutil_compress(t1);void*_tmp152=_tmp151;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp152)->tag == 0U)switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp152)->f1)){case 2U: if(((struct Cyc_Absyn_FloatCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp152)->f1)->f1 == 0){_LL1: _LL2:
 Cyc_Tcutil_unchecked_cast(te,e,Cyc_Absyn_double_type,Cyc_Absyn_No_coercion);t1=Cyc_Absyn_double_type;goto _LL0;}else{goto _LL7;}case 1U: switch(((struct Cyc_Absyn_IntCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp152)->f1)->f2){case Cyc_Absyn_Char_sz: _LL3: _LL4:
 goto _LL6;case Cyc_Absyn_Short_sz: _LL5: _LL6:
 Cyc_Tcutil_unchecked_cast(te,e,Cyc_Absyn_sint_type,Cyc_Absyn_No_coercion);t1=Cyc_Absyn_sint_type;goto _LL0;default: goto _LL7;}default: goto _LL7;}else{_LL7: _LL8:
 goto _LL0;}_LL0:;}
# 918
for(fields=fs;fields != 0;fields=fields->tl){
struct Cyc_Absyn_Datatypefield _tmp153=*((struct Cyc_Absyn_Datatypefield*)fields->hd);struct Cyc_Absyn_Datatypefield _tmp154=_tmp153;struct _tuple1*_tmp158;struct Cyc_List_List*_tmp157;unsigned int _tmp156;enum Cyc_Absyn_Scope _tmp155;_LLA: _tmp158=_tmp154.name;_tmp157=_tmp154.typs;_tmp156=_tmp154.loc;_tmp155=_tmp154.sc;_LLB:;
# 921
if(_tmp157 == 0  || _tmp157->tl != 0)continue;{
void*t2=Cyc_Tcutil_substitute(inst,(*((struct _tuple17*)_tmp157->hd)).f2);
# 924
if(Cyc_Tcutil_unify(t1,t2))
return(struct Cyc_Absyn_Datatypefield*)fields->hd;};}
# 929
for(fields=fs;fields != 0;fields=fields->tl){
struct Cyc_Absyn_Datatypefield _tmp159=*((struct Cyc_Absyn_Datatypefield*)fields->hd);struct Cyc_Absyn_Datatypefield _tmp15A=_tmp159;struct _tuple1*_tmp15E;struct Cyc_List_List*_tmp15D;unsigned int _tmp15C;enum Cyc_Absyn_Scope _tmp15B;_LLD: _tmp15E=_tmp15A.name;_tmp15D=_tmp15A.typs;_tmp15C=_tmp15A.loc;_tmp15B=_tmp15A.sc;_LLE:;
# 932
if(_tmp15D == 0  || _tmp15D->tl != 0)continue;{
void*t2=Cyc_Tcutil_substitute(inst,(*((struct _tuple17*)_tmp15D->hd)).f2);
# 936
int bogus=0;
if(Cyc_Tcutil_coerce_arg(te,e,t2,& bogus))
return(struct Cyc_Absyn_Datatypefield*)fields->hd;};}
# 941
({struct Cyc_String_pa_PrintArg_struct _tmp161=({struct Cyc_String_pa_PrintArg_struct _tmp4F8;_tmp4F8.tag=0U,({
struct _dyneither_ptr _tmp634=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(tu));_tmp4F8.f1=_tmp634;});_tmp4F8;});struct Cyc_String_pa_PrintArg_struct _tmp162=({struct Cyc_String_pa_PrintArg_struct _tmp4F7;_tmp4F7.tag=0U,({struct _dyneither_ptr _tmp635=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t1));_tmp4F7.f1=_tmp635;});_tmp4F7;});void*_tmp15F[2U];_tmp15F[0]=& _tmp161,_tmp15F[1]=& _tmp162;({unsigned int _tmp637=e->loc;struct _dyneither_ptr _tmp636=({const char*_tmp160="can't find a field in %s to inject a value of type %s";_tag_dyneither(_tmp160,sizeof(char),54U);});Cyc_Tcutil_terr(_tmp637,_tmp636,_tag_dyneither(_tmp15F,sizeof(void*),2U));});});
return 0;}
# 947
static void*Cyc_Tcexp_tcFnCall(struct Cyc_Tcenv_Tenv*te_orig,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e,struct Cyc_List_List*args,struct Cyc_Absyn_VarargCallInfo**vararg_call_info,struct Cyc_List_List**alias_arg_exps){
# 953
struct Cyc_List_List*_tmp163=args;
int _tmp164=0;
struct Cyc_Tcenv_Tenv*_tmp165=Cyc_Tcenv_new_block(loc,te_orig);
struct Cyc_Tcenv_Tenv*_tmp166=Cyc_Tcenv_clear_abstract_val_ok(_tmp165);
Cyc_Tcexp_tcExp(_tmp166,0,e);{
void*t=Cyc_Tcutil_compress((void*)_check_null(e->topt));
# 962
void*_tmp167=t;void*_tmp1CA;struct Cyc_Absyn_Tqual _tmp1C9;void*_tmp1C8;void*_tmp1C7;void*_tmp1C6;void*_tmp1C5;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp167)->tag == 3U){_LL1: _tmp1CA=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp167)->f1).elt_type;_tmp1C9=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp167)->f1).elt_tq;_tmp1C8=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp167)->f1).ptr_atts).rgn;_tmp1C7=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp167)->f1).ptr_atts).nullable;_tmp1C6=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp167)->f1).ptr_atts).bounds;_tmp1C5=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp167)->f1).ptr_atts).zero_term;_LL2:
# 967
 Cyc_Tcenv_check_rgn_accessible(_tmp166,loc,_tmp1C8);
# 969
Cyc_Tcutil_check_nonzero_bound(loc,_tmp1C6);{
void*_tmp168=Cyc_Tcutil_compress(_tmp1CA);void*_tmp169=_tmp168;struct Cyc_List_List*_tmp1C1;void*_tmp1C0;struct Cyc_Absyn_Tqual _tmp1BF;void*_tmp1BE;struct Cyc_List_List*_tmp1BD;int _tmp1BC;struct Cyc_Absyn_VarargInfo*_tmp1BB;struct Cyc_List_List*_tmp1BA;struct Cyc_List_List*_tmp1B9;struct Cyc_Absyn_Exp*_tmp1B8;struct Cyc_List_List*_tmp1B7;struct Cyc_Absyn_Exp*_tmp1B6;struct Cyc_List_List*_tmp1B5;if(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->tag == 5U){_LL6: _tmp1C1=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).tvars;_tmp1C0=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).effect;_tmp1BF=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).ret_tqual;_tmp1BE=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).ret_type;_tmp1BD=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).args;_tmp1BC=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).c_varargs;_tmp1BB=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).cyc_varargs;_tmp1BA=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).rgn_po;_tmp1B9=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).attributes;_tmp1B8=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).requires_clause;_tmp1B7=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).requires_relns;_tmp1B6=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).ensures_clause;_tmp1B5=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp169)->f1).ensures_relns;_LL7:
# 974
 if(_tmp1C1 != 0  || _tmp1BA != 0)
({void*_tmp16A=0U;({unsigned int _tmp639=e->loc;struct _dyneither_ptr _tmp638=({const char*_tmp16B="function should have been instantiated prior to use -- probably a compiler bug";_tag_dyneither(_tmp16B,sizeof(char),79U);});Cyc_Tcutil_terr(_tmp639,_tmp638,_tag_dyneither(_tmp16A,sizeof(void*),0U));});});
# 978
if(topt != 0)Cyc_Tcutil_unify(_tmp1BE,*topt);
# 980
while(_tmp163 != 0  && _tmp1BD != 0){
# 982
int alias_coercion=0;
struct Cyc_Absyn_Exp*e1=(struct Cyc_Absyn_Exp*)_tmp163->hd;
void*t2=(*((struct _tuple9*)_tmp1BD->hd)).f3;
Cyc_Tcexp_tcExp(_tmp166,& t2,e1);
if(!Cyc_Tcutil_coerce_arg(_tmp166,e1,t2,& alias_coercion)){
struct _dyneither_ptr s0=({const char*_tmp17B="actual argument has type ";_tag_dyneither(_tmp17B,sizeof(char),26U);});
struct _dyneither_ptr s1=Cyc_Absynpp_typ2string((void*)_check_null(e1->topt));
struct _dyneither_ptr s2=({const char*_tmp17A=" but formal has type ";_tag_dyneither(_tmp17A,sizeof(char),22U);});
struct _dyneither_ptr s3=Cyc_Absynpp_typ2string(t2);
if(({unsigned long _tmp63C=({unsigned long _tmp63B=({unsigned long _tmp63A=Cyc_strlen((struct _dyneither_ptr)s0);_tmp63A + Cyc_strlen((struct _dyneither_ptr)s1);});_tmp63B + Cyc_strlen((struct _dyneither_ptr)s2);});_tmp63C + Cyc_strlen((struct _dyneither_ptr)s3);})>= 70)
({void*_tmp16C=0U;({unsigned int _tmp63F=e1->loc;struct _dyneither_ptr _tmp63E=(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp16F=({struct Cyc_String_pa_PrintArg_struct _tmp4FC;_tmp4FC.tag=0U,_tmp4FC.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s0);_tmp4FC;});struct Cyc_String_pa_PrintArg_struct _tmp170=({struct Cyc_String_pa_PrintArg_struct _tmp4FB;_tmp4FB.tag=0U,_tmp4FB.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s1);_tmp4FB;});struct Cyc_String_pa_PrintArg_struct _tmp171=({struct Cyc_String_pa_PrintArg_struct _tmp4FA;_tmp4FA.tag=0U,_tmp4FA.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s2);_tmp4FA;});struct Cyc_String_pa_PrintArg_struct _tmp172=({struct Cyc_String_pa_PrintArg_struct _tmp4F9;_tmp4F9.tag=0U,_tmp4F9.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s3);_tmp4F9;});void*_tmp16D[4U];_tmp16D[0]=& _tmp16F,_tmp16D[1]=& _tmp170,_tmp16D[2]=& _tmp171,_tmp16D[3]=& _tmp172;({struct _dyneither_ptr _tmp63D=({const char*_tmp16E="%s\n\t%s\n%s\n\t%s.";_tag_dyneither(_tmp16E,sizeof(char),15U);});Cyc_aprintf(_tmp63D,_tag_dyneither(_tmp16D,sizeof(void*),4U));});});Cyc_Tcutil_terr(_tmp63F,_tmp63E,_tag_dyneither(_tmp16C,sizeof(void*),0U));});});else{
# 994
({void*_tmp173=0U;({unsigned int _tmp642=e1->loc;struct _dyneither_ptr _tmp641=(struct _dyneither_ptr)({struct Cyc_String_pa_PrintArg_struct _tmp176=({struct Cyc_String_pa_PrintArg_struct _tmp500;_tmp500.tag=0U,_tmp500.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s0);_tmp500;});struct Cyc_String_pa_PrintArg_struct _tmp177=({struct Cyc_String_pa_PrintArg_struct _tmp4FF;_tmp4FF.tag=0U,_tmp4FF.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s1);_tmp4FF;});struct Cyc_String_pa_PrintArg_struct _tmp178=({struct Cyc_String_pa_PrintArg_struct _tmp4FE;_tmp4FE.tag=0U,_tmp4FE.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s2);_tmp4FE;});struct Cyc_String_pa_PrintArg_struct _tmp179=({struct Cyc_String_pa_PrintArg_struct _tmp4FD;_tmp4FD.tag=0U,_tmp4FD.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)s3);_tmp4FD;});void*_tmp174[4U];_tmp174[0]=& _tmp176,_tmp174[1]=& _tmp177,_tmp174[2]=& _tmp178,_tmp174[3]=& _tmp179;({struct _dyneither_ptr _tmp640=({const char*_tmp175="%s%s%s%s.";_tag_dyneither(_tmp175,sizeof(char),10U);});Cyc_aprintf(_tmp640,_tag_dyneither(_tmp174,sizeof(void*),4U));});});Cyc_Tcutil_terr(_tmp642,_tmp641,_tag_dyneither(_tmp173,sizeof(void*),0U));});});}
Cyc_Tcutil_unify((void*)_check_null(e1->topt),t2);
Cyc_Tcutil_explain_failure();}
# 999
if(alias_coercion)
({struct Cyc_List_List*_tmp643=({struct Cyc_List_List*_tmp17C=_cycalloc(sizeof(*_tmp17C));_tmp17C->hd=(void*)_tmp164,_tmp17C->tl=*alias_arg_exps;_tmp17C;});*alias_arg_exps=_tmp643;});
if(Cyc_Tcutil_is_noalias_pointer_or_aggr(t2) && !Cyc_Tcutil_is_noalias_path(e1))
({void*_tmp17D=0U;({unsigned int _tmp645=e1->loc;struct _dyneither_ptr _tmp644=({const char*_tmp17E="Cannot consume non-unique paths; do swap instead";_tag_dyneither(_tmp17E,sizeof(char),49U);});Cyc_Tcutil_terr(_tmp645,_tmp644,_tag_dyneither(_tmp17D,sizeof(void*),0U));});});
_tmp163=_tmp163->tl;
_tmp1BD=_tmp1BD->tl;
++ _tmp164;}{
# 1010
int args_already_checked=0;
{struct Cyc_List_List*a=_tmp1B9;for(0;a != 0;a=a->tl){
void*_tmp17F=(void*)a->hd;void*_tmp180=_tmp17F;enum Cyc_Absyn_Format_Type _tmp18D;int _tmp18C;int _tmp18B;if(((struct Cyc_Absyn_Format_att_Absyn_Attribute_struct*)_tmp180)->tag == 19U){_LLB: _tmp18D=((struct Cyc_Absyn_Format_att_Absyn_Attribute_struct*)_tmp180)->f1;_tmp18C=((struct Cyc_Absyn_Format_att_Absyn_Attribute_struct*)_tmp180)->f2;_tmp18B=((struct Cyc_Absyn_Format_att_Absyn_Attribute_struct*)_tmp180)->f3;_LLC:
# 1014
{struct _handler_cons _tmp181;_push_handler(& _tmp181);{int _tmp183=0;if(setjmp(_tmp181.handler))_tmp183=1;if(!_tmp183){
# 1016
{struct Cyc_Absyn_Exp*_tmp184=((struct Cyc_Absyn_Exp*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)(args,_tmp18C - 1);
# 1018
struct Cyc_Core_Opt*fmt_args;
if(_tmp18B == 0)
fmt_args=0;else{
# 1022
fmt_args=({struct Cyc_Core_Opt*_tmp185=_cycalloc(sizeof(*_tmp185));({struct Cyc_List_List*_tmp646=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,int i))Cyc_List_nth_tail)(args,_tmp18B - 1);_tmp185->v=_tmp646;});_tmp185;});}
args_already_checked=1;{
enum Cyc_Absyn_Format_Type _tmp186=_tmp18D;if(_tmp186 == Cyc_Absyn_Printf_ft){_LL10: _LL11:
# 1026
 Cyc_Tcexp_check_format_args(_tmp166,_tmp184,fmt_args,_tmp18B - 1,alias_arg_exps,Cyc_Formatstr_get_format_types);
# 1029
goto _LLF;}else{_LL12: _LL13:
# 1031
 Cyc_Tcexp_check_format_args(_tmp166,_tmp184,fmt_args,_tmp18B - 1,alias_arg_exps,Cyc_Formatstr_get_scanf_types);
# 1034
goto _LLF;}_LLF:;};}
# 1016
;_pop_handler();}else{void*_tmp182=(void*)_exn_thrown;void*_tmp187=_tmp182;void*_tmp18A;if(((struct Cyc_List_Nth_exn_struct*)_tmp187)->tag == Cyc_List_Nth){_LL15: _LL16:
# 1036
({void*_tmp188=0U;({unsigned int _tmp648=loc;struct _dyneither_ptr _tmp647=({const char*_tmp189="bad format arguments";_tag_dyneither(_tmp189,sizeof(char),21U);});Cyc_Tcutil_terr(_tmp648,_tmp647,_tag_dyneither(_tmp188,sizeof(void*),0U));});});goto _LL14;}else{_LL17: _tmp18A=_tmp187;_LL18:(int)_rethrow(_tmp18A);}_LL14:;}};}
goto _LLA;}else{_LLD: _LLE:
 goto _LLA;}_LLA:;}}
# 1041
if(_tmp1BD != 0)({void*_tmp18E=0U;({unsigned int _tmp64A=loc;struct _dyneither_ptr _tmp649=({const char*_tmp18F="too few arguments for function";_tag_dyneither(_tmp18F,sizeof(char),31U);});Cyc_Tcutil_terr(_tmp64A,_tmp649,_tag_dyneither(_tmp18E,sizeof(void*),0U));});});else{
# 1043
if((_tmp163 != 0  || _tmp1BC) || _tmp1BB != 0){
if(_tmp1BC)
for(0;_tmp163 != 0;_tmp163=_tmp163->tl){
Cyc_Tcexp_tcExp(_tmp166,0,(struct Cyc_Absyn_Exp*)_tmp163->hd);}else{
if(_tmp1BB == 0)
({void*_tmp190=0U;({unsigned int _tmp64C=loc;struct _dyneither_ptr _tmp64B=({const char*_tmp191="too many arguments for function";_tag_dyneither(_tmp191,sizeof(char),32U);});Cyc_Tcutil_terr(_tmp64C,_tmp64B,_tag_dyneither(_tmp190,sizeof(void*),0U));});});else{
# 1050
struct Cyc_Absyn_VarargInfo _tmp192=*_tmp1BB;struct Cyc_Absyn_VarargInfo _tmp193=_tmp192;void*_tmp1B1;int _tmp1B0;_LL1A: _tmp1B1=_tmp193.type;_tmp1B0=_tmp193.inject;_LL1B:;{
struct Cyc_Absyn_VarargCallInfo*_tmp194=({struct Cyc_Absyn_VarargCallInfo*_tmp1AF=_cycalloc(sizeof(*_tmp1AF));_tmp1AF->num_varargs=0,_tmp1AF->injectors=0,_tmp1AF->vai=_tmp1BB;_tmp1AF;});
# 1054
*vararg_call_info=_tmp194;
# 1056
if(!_tmp1B0)
# 1058
for(0;_tmp163 != 0;(_tmp163=_tmp163->tl,_tmp164 ++)){
int alias_coercion=0;
struct Cyc_Absyn_Exp*e1=(struct Cyc_Absyn_Exp*)_tmp163->hd;
++ _tmp194->num_varargs;
Cyc_Tcexp_tcExp(_tmp166,& _tmp1B1,e1);
if(!Cyc_Tcutil_coerce_arg(_tmp166,e1,_tmp1B1,& alias_coercion)){
({struct Cyc_String_pa_PrintArg_struct _tmp197=({struct Cyc_String_pa_PrintArg_struct _tmp502;_tmp502.tag=0U,({
struct _dyneither_ptr _tmp64D=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp1B1));_tmp502.f1=_tmp64D;});_tmp502;});struct Cyc_String_pa_PrintArg_struct _tmp198=({struct Cyc_String_pa_PrintArg_struct _tmp501;_tmp501.tag=0U,({struct _dyneither_ptr _tmp64E=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(e1->topt)));_tmp501.f1=_tmp64E;});_tmp501;});void*_tmp195[2U];_tmp195[0]=& _tmp197,_tmp195[1]=& _tmp198;({unsigned int _tmp650=loc;struct _dyneither_ptr _tmp64F=({const char*_tmp196="vararg requires type %s but argument has type %s";_tag_dyneither(_tmp196,sizeof(char),49U);});Cyc_Tcutil_terr(_tmp650,_tmp64F,_tag_dyneither(_tmp195,sizeof(void*),2U));});});
Cyc_Tcutil_explain_failure();}
# 1068
if(alias_coercion)
({struct Cyc_List_List*_tmp651=({struct Cyc_List_List*_tmp199=_cycalloc(sizeof(*_tmp199));_tmp199->hd=(void*)_tmp164,_tmp199->tl=*alias_arg_exps;_tmp199;});*alias_arg_exps=_tmp651;});
if(Cyc_Tcutil_is_noalias_pointer_or_aggr(_tmp1B1) && !
Cyc_Tcutil_is_noalias_path(e1))
({void*_tmp19A=0U;({unsigned int _tmp653=e1->loc;struct _dyneither_ptr _tmp652=({const char*_tmp19B="Cannot consume non-unique paths; do swap instead";_tag_dyneither(_tmp19B,sizeof(char),49U);});Cyc_Tcutil_terr(_tmp653,_tmp652,_tag_dyneither(_tmp19A,sizeof(void*),0U));});});}else{
# 1077
void*_tmp19C=Cyc_Tcutil_compress(Cyc_Tcutil_pointer_elt_type(_tmp1B1));void*_tmp19D=_tmp19C;struct Cyc_Absyn_Datatypedecl*_tmp1AE;struct Cyc_List_List*_tmp1AD;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp19D)->tag == 0U){if(((struct Cyc_Absyn_DatatypeCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp19D)->f1)->tag == 18U){if(((((struct Cyc_Absyn_DatatypeCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp19D)->f1)->f1).KnownDatatype).tag == 2){_LL1D: _tmp1AE=*((((struct Cyc_Absyn_DatatypeCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp19D)->f1)->f1).KnownDatatype).val;_tmp1AD=((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp19D)->f2;_LL1E: {
# 1081
struct Cyc_Absyn_Datatypedecl*_tmp19E=*Cyc_Tcenv_lookup_datatypedecl(_tmp166,loc,_tmp1AE->name);
struct Cyc_List_List*fields=0;
if(_tmp19E->fields == 0)
({struct Cyc_String_pa_PrintArg_struct _tmp1A1=({struct Cyc_String_pa_PrintArg_struct _tmp503;_tmp503.tag=0U,({
struct _dyneither_ptr _tmp654=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp1B1));_tmp503.f1=_tmp654;});_tmp503;});void*_tmp19F[1U];_tmp19F[0]=& _tmp1A1;({unsigned int _tmp656=loc;struct _dyneither_ptr _tmp655=({const char*_tmp1A0="can't inject into abstract datatype %s";_tag_dyneither(_tmp1A0,sizeof(char),39U);});Cyc_Tcutil_terr(_tmp656,_tmp655,_tag_dyneither(_tmp19F,sizeof(void*),1U));});});else{
fields=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmp19E->fields))->v;}
# 1092
if(!({void*_tmp657=Cyc_Tcutil_pointer_region(_tmp1B1);Cyc_Tcutil_unify(_tmp657,Cyc_Tcenv_curr_rgn(_tmp166));}))
({struct Cyc_String_pa_PrintArg_struct _tmp1A4=({struct Cyc_String_pa_PrintArg_struct _tmp505;_tmp505.tag=0U,({
struct _dyneither_ptr _tmp658=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp1B1));_tmp505.f1=_tmp658;});_tmp505;});struct Cyc_String_pa_PrintArg_struct _tmp1A5=({struct Cyc_String_pa_PrintArg_struct _tmp504;_tmp504.tag=0U,({struct _dyneither_ptr _tmp659=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(Cyc_Tcenv_curr_rgn(_tmp166)));_tmp504.f1=_tmp659;});_tmp504;});void*_tmp1A2[2U];_tmp1A2[0]=& _tmp1A4,_tmp1A2[1]=& _tmp1A5;({unsigned int _tmp65B=loc;struct _dyneither_ptr _tmp65A=({const char*_tmp1A3="can't unify pointer region for %s to curr_rgn %s";_tag_dyneither(_tmp1A3,sizeof(char),49U);});Cyc_Tcutil_terr(_tmp65B,_tmp65A,_tag_dyneither(_tmp1A2,sizeof(void*),2U));});});{
# 1096
struct Cyc_List_List*_tmp1A6=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_zip)(_tmp19E->tvs,_tmp1AD);
for(0;_tmp163 != 0;_tmp163=_tmp163->tl){
++ _tmp194->num_varargs;{
struct Cyc_Absyn_Exp*e1=(struct Cyc_Absyn_Exp*)_tmp163->hd;
# 1101
if(!args_already_checked){
Cyc_Tcexp_tcExp(_tmp166,0,e1);
if(Cyc_Tcutil_is_noalias_pointer_or_aggr((void*)_check_null(e1->topt)) && !
Cyc_Tcutil_is_noalias_path(e1))
({void*_tmp1A7=0U;({unsigned int _tmp65D=e1->loc;struct _dyneither_ptr _tmp65C=({const char*_tmp1A8="Cannot consume non-unique paths; do swap instead";_tag_dyneither(_tmp1A8,sizeof(char),49U);});Cyc_Tcutil_terr(_tmp65D,_tmp65C,_tag_dyneither(_tmp1A7,sizeof(void*),0U));});});}{
# 1107
struct Cyc_Absyn_Datatypefield*_tmp1A9=({struct Cyc_Tcenv_Tenv*_tmp661=_tmp166;struct Cyc_Absyn_Exp*_tmp660=e1;void*_tmp65F=Cyc_Tcutil_pointer_elt_type(_tmp1B1);struct Cyc_List_List*_tmp65E=_tmp1A6;Cyc_Tcexp_tcInjection(_tmp661,_tmp660,_tmp65F,_tmp65E,fields);});
if(_tmp1A9 != 0)
({struct Cyc_List_List*_tmp663=({
struct Cyc_List_List*_tmp662=_tmp194->injectors;((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(_tmp662,({struct Cyc_List_List*_tmp1AA=_cycalloc(sizeof(*_tmp1AA));
_tmp1AA->hd=_tmp1A9,_tmp1AA->tl=0;_tmp1AA;}));});
# 1109
_tmp194->injectors=_tmp663;});};};}
# 1113
goto _LL1C;};}}else{goto _LL1F;}}else{goto _LL1F;}}else{_LL1F: _LL20:
({void*_tmp1AB=0U;({unsigned int _tmp665=loc;struct _dyneither_ptr _tmp664=({const char*_tmp1AC="bad inject vararg type";_tag_dyneither(_tmp1AC,sizeof(char),23U);});Cyc_Tcutil_terr(_tmp665,_tmp664,_tag_dyneither(_tmp1AB,sizeof(void*),0U));});});goto _LL1C;}_LL1C:;}};}}}}
# 1119
if(*alias_arg_exps == 0)
# 1128 "tcexp.cyc"
Cyc_Tcenv_check_effect_accessible(_tmp166,loc,(void*)_check_null(_tmp1C0));
# 1130
return _tmp1BE;};}else{_LL8: _LL9:
 return({struct Cyc_String_pa_PrintArg_struct _tmp1B4=({struct Cyc_String_pa_PrintArg_struct _tmp506;_tmp506.tag=0U,({struct _dyneither_ptr _tmp666=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp506.f1=_tmp666;});_tmp506;});void*_tmp1B2[1U];_tmp1B2[0]=& _tmp1B4;({struct Cyc_Tcenv_Tenv*_tmp66A=_tmp166;unsigned int _tmp669=loc;void**_tmp668=topt;struct _dyneither_ptr _tmp667=({const char*_tmp1B3="expected pointer to function but found %s";_tag_dyneither(_tmp1B3,sizeof(char),42U);});Cyc_Tcexp_expr_err(_tmp66A,_tmp669,_tmp668,_tmp667,_tag_dyneither(_tmp1B2,sizeof(void*),1U));});});}_LL5:;};}else{_LL3: _LL4:
# 1133
 return({struct Cyc_String_pa_PrintArg_struct _tmp1C4=({struct Cyc_String_pa_PrintArg_struct _tmp507;_tmp507.tag=0U,({struct _dyneither_ptr _tmp66B=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp507.f1=_tmp66B;});_tmp507;});void*_tmp1C2[1U];_tmp1C2[0]=& _tmp1C4;({struct Cyc_Tcenv_Tenv*_tmp66F=_tmp166;unsigned int _tmp66E=loc;void**_tmp66D=topt;struct _dyneither_ptr _tmp66C=({const char*_tmp1C3="expected pointer to function but found %s";_tag_dyneither(_tmp1C3,sizeof(char),42U);});Cyc_Tcexp_expr_err(_tmp66F,_tmp66E,_tmp66D,_tmp66C,_tag_dyneither(_tmp1C2,sizeof(void*),1U));});});}_LL0:;};}
# 1137
static void*Cyc_Tcexp_tcThrow(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e){
int bogus=0;
void*_tmp1CB=Cyc_Absyn_exn_type();
Cyc_Tcexp_tcExp(te,& _tmp1CB,e);
if(!Cyc_Tcutil_coerce_arg(te,e,_tmp1CB,& bogus))
({struct Cyc_String_pa_PrintArg_struct _tmp1CE=({struct Cyc_String_pa_PrintArg_struct _tmp509;_tmp509.tag=0U,({struct _dyneither_ptr _tmp670=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp1CB));_tmp509.f1=_tmp670;});_tmp509;});struct Cyc_String_pa_PrintArg_struct _tmp1CF=({struct Cyc_String_pa_PrintArg_struct _tmp508;_tmp508.tag=0U,({
struct _dyneither_ptr _tmp671=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(e->topt)));_tmp508.f1=_tmp671;});_tmp508;});void*_tmp1CC[2U];_tmp1CC[0]=& _tmp1CE,_tmp1CC[1]=& _tmp1CF;({unsigned int _tmp673=loc;struct _dyneither_ptr _tmp672=({const char*_tmp1CD="expected %s but found %s";_tag_dyneither(_tmp1CD,sizeof(char),25U);});Cyc_Tcutil_terr(_tmp673,_tmp672,_tag_dyneither(_tmp1CC,sizeof(void*),2U));});});
if(topt != 0)return*topt;
return Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te));}
# 1149
static void*Cyc_Tcexp_tcInstantiate(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e,struct Cyc_List_List*ts){
# 1151
Cyc_Tcexp_tcExpNoInst(te,0,e);
# 1153
({void*_tmp674=Cyc_Absyn_pointer_expand((void*)_check_null(e->topt),0);e->topt=_tmp674;});{
void*t1=Cyc_Tcutil_compress((void*)_check_null(e->topt));
{void*_tmp1D0=t1;void*_tmp1EB;struct Cyc_Absyn_Tqual _tmp1EA;void*_tmp1E9;void*_tmp1E8;void*_tmp1E7;void*_tmp1E6;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D0)->tag == 3U){_LL1: _tmp1EB=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D0)->f1).elt_type;_tmp1EA=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D0)->f1).elt_tq;_tmp1E9=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D0)->f1).ptr_atts).rgn;_tmp1E8=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D0)->f1).ptr_atts).nullable;_tmp1E7=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D0)->f1).ptr_atts).bounds;_tmp1E6=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1D0)->f1).ptr_atts).zero_term;_LL2:
# 1157
{void*_tmp1D1=Cyc_Tcutil_compress(_tmp1EB);void*_tmp1D2=_tmp1D1;struct Cyc_List_List*_tmp1E5;void*_tmp1E4;struct Cyc_Absyn_Tqual _tmp1E3;void*_tmp1E2;struct Cyc_List_List*_tmp1E1;int _tmp1E0;struct Cyc_Absyn_VarargInfo*_tmp1DF;struct Cyc_List_List*_tmp1DE;struct Cyc_List_List*_tmp1DD;struct Cyc_Absyn_Exp*_tmp1DC;struct Cyc_List_List*_tmp1DB;struct Cyc_Absyn_Exp*_tmp1DA;struct Cyc_List_List*_tmp1D9;if(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->tag == 5U){_LL6: _tmp1E5=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).tvars;_tmp1E4=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).effect;_tmp1E3=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).ret_tqual;_tmp1E2=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).ret_type;_tmp1E1=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).args;_tmp1E0=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).c_varargs;_tmp1DF=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).cyc_varargs;_tmp1DE=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).rgn_po;_tmp1DD=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).attributes;_tmp1DC=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).requires_clause;_tmp1DB=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).requires_relns;_tmp1DA=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).ensures_clause;_tmp1D9=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp1D2)->f1).ensures_relns;_LL7: {
# 1159
struct Cyc_List_List*instantiation=0;
# 1161
for(0;ts != 0  && _tmp1E5 != 0;(ts=ts->tl,_tmp1E5=_tmp1E5->tl)){
struct Cyc_Absyn_Kind*k=Cyc_Tcutil_tvar_kind((struct Cyc_Absyn_Tvar*)_tmp1E5->hd,& Cyc_Tcutil_bk);
({unsigned int _tmp678=loc;struct Cyc_Tcenv_Tenv*_tmp677=te;struct Cyc_List_List*_tmp676=Cyc_Tcenv_lookup_type_vars(te);struct Cyc_Absyn_Kind*_tmp675=k;Cyc_Tcutil_check_type(_tmp678,_tmp677,_tmp676,_tmp675,1,1,(void*)ts->hd);});
Cyc_Tcutil_check_no_qual(loc,(void*)ts->hd);
instantiation=({struct Cyc_List_List*_tmp1D4=_cycalloc(sizeof(*_tmp1D4));({struct _tuple13*_tmp679=({struct _tuple13*_tmp1D3=_cycalloc(sizeof(*_tmp1D3));_tmp1D3->f1=(struct Cyc_Absyn_Tvar*)_tmp1E5->hd,_tmp1D3->f2=(void*)ts->hd;_tmp1D3;});_tmp1D4->hd=_tmp679;}),_tmp1D4->tl=instantiation;_tmp1D4;});}
# 1167
if(ts != 0)
return({void*_tmp1D5=0U;({struct Cyc_Tcenv_Tenv*_tmp67D=te;unsigned int _tmp67C=loc;void**_tmp67B=topt;struct _dyneither_ptr _tmp67A=({const char*_tmp1D6="too many type variables in instantiation";_tag_dyneither(_tmp1D6,sizeof(char),41U);});Cyc_Tcexp_expr_err(_tmp67D,_tmp67C,_tmp67B,_tmp67A,_tag_dyneither(_tmp1D5,sizeof(void*),0U));});});
# 1172
if(_tmp1E5 == 0){
_tmp1DE=Cyc_Tcutil_rsubst_rgnpo(Cyc_Core_heap_region,instantiation,_tmp1DE);
Cyc_Tcenv_check_rgn_partial_order(te,loc,_tmp1DE);
_tmp1DE=0;}{
# 1177
void*new_fn_typ=({
struct Cyc_List_List*_tmp67E=instantiation;Cyc_Tcutil_substitute(_tmp67E,(void*)({struct Cyc_Absyn_FnType_Absyn_Type_struct*_tmp1D8=_cycalloc(sizeof(*_tmp1D8));
_tmp1D8->tag=5U,(_tmp1D8->f1).tvars=_tmp1E5,(_tmp1D8->f1).effect=_tmp1E4,(_tmp1D8->f1).ret_tqual=_tmp1E3,(_tmp1D8->f1).ret_type=_tmp1E2,(_tmp1D8->f1).args=_tmp1E1,(_tmp1D8->f1).c_varargs=_tmp1E0,(_tmp1D8->f1).cyc_varargs=_tmp1DF,(_tmp1D8->f1).rgn_po=_tmp1DE,(_tmp1D8->f1).attributes=_tmp1DD,(_tmp1D8->f1).requires_clause=_tmp1DC,(_tmp1D8->f1).requires_relns=_tmp1DB,(_tmp1D8->f1).ensures_clause=_tmp1DA,(_tmp1D8->f1).ensures_relns=_tmp1D9;_tmp1D8;}));});
# 1182
return(void*)({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp1D7=_cycalloc(sizeof(*_tmp1D7));_tmp1D7->tag=3U,(_tmp1D7->f1).elt_type=new_fn_typ,(_tmp1D7->f1).elt_tq=_tmp1EA,((_tmp1D7->f1).ptr_atts).rgn=_tmp1E9,((_tmp1D7->f1).ptr_atts).nullable=_tmp1E8,((_tmp1D7->f1).ptr_atts).bounds=_tmp1E7,((_tmp1D7->f1).ptr_atts).zero_term=_tmp1E6,((_tmp1D7->f1).ptr_atts).ptrloc=0;_tmp1D7;});};}}else{_LL8: _LL9:
 goto _LL5;}_LL5:;}
# 1185
goto _LL0;}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 1188
return({struct Cyc_String_pa_PrintArg_struct _tmp1EE=({struct Cyc_String_pa_PrintArg_struct _tmp50A;_tmp50A.tag=0U,({
struct _dyneither_ptr _tmp67F=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t1));_tmp50A.f1=_tmp67F;});_tmp50A;});void*_tmp1EC[1U];_tmp1EC[0]=& _tmp1EE;({struct Cyc_Tcenv_Tenv*_tmp683=te;unsigned int _tmp682=loc;void**_tmp681=topt;struct _dyneither_ptr _tmp680=({const char*_tmp1ED="expecting polymorphic type but found %s";_tag_dyneither(_tmp1ED,sizeof(char),40U);});Cyc_Tcexp_expr_err(_tmp683,_tmp682,_tmp681,_tmp680,_tag_dyneither(_tmp1EC,sizeof(void*),1U));});});};}
# 1193
static void*Cyc_Tcexp_tcCast(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,void*t,struct Cyc_Absyn_Exp*e,enum Cyc_Absyn_Coercion*c){
# 1195
({unsigned int _tmp687=loc;struct Cyc_Tcenv_Tenv*_tmp686=te;struct Cyc_List_List*_tmp685=Cyc_Tcenv_lookup_type_vars(te);struct Cyc_Absyn_Kind*_tmp684=
Cyc_Tcenv_abstract_val_ok(te)?& Cyc_Tcutil_tak:& Cyc_Tcutil_tmk;
# 1195
Cyc_Tcutil_check_type(_tmp687,_tmp686,_tmp685,_tmp684,1,0,t);});
# 1197
Cyc_Tcutil_check_no_qual(loc,t);
# 1199
Cyc_Tcexp_tcExp(te,& t,e);{
void*t2=(void*)_check_null(e->topt);
if(Cyc_Tcutil_silent_castable(te,loc,t2,t))
*((enum Cyc_Absyn_Coercion*)_check_null(c))=Cyc_Absyn_No_coercion;else{
# 1204
enum Cyc_Absyn_Coercion crc=Cyc_Tcutil_castable(te,loc,t2,t);
if(crc != Cyc_Absyn_Unknown_coercion)
*((enum Cyc_Absyn_Coercion*)_check_null(c))=crc;else{
if(Cyc_Tcutil_zero_to_null(te,t,e))
*((enum Cyc_Absyn_Coercion*)_check_null(c))=Cyc_Absyn_No_coercion;else{
# 1211
Cyc_Tcutil_unify(t2,t);{
void*_tmp1EF=({struct Cyc_String_pa_PrintArg_struct _tmp1F2=({struct Cyc_String_pa_PrintArg_struct _tmp50C;_tmp50C.tag=0U,({
struct _dyneither_ptr _tmp688=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t2));_tmp50C.f1=_tmp688;});_tmp50C;});struct Cyc_String_pa_PrintArg_struct _tmp1F3=({struct Cyc_String_pa_PrintArg_struct _tmp50B;_tmp50B.tag=0U,({struct _dyneither_ptr _tmp689=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp50B.f1=_tmp689;});_tmp50B;});void*_tmp1F0[2U];_tmp1F0[0]=& _tmp1F2,_tmp1F0[1]=& _tmp1F3;({struct Cyc_Tcenv_Tenv*_tmp68C=te;unsigned int _tmp68B=loc;struct _dyneither_ptr _tmp68A=({const char*_tmp1F1="cannot cast %s to %s";_tag_dyneither(_tmp1F1,sizeof(char),21U);});Cyc_Tcexp_expr_err(_tmp68C,_tmp68B,& t,_tmp68A,_tag_dyneither(_tmp1F0,sizeof(void*),2U));});});
Cyc_Tcutil_explain_failure();
return _tmp1EF;};}}}
# 1221
{struct _tuple0 _tmp1F4=({struct _tuple0 _tmp50D;_tmp50D.f1=e->r,({void*_tmp68D=Cyc_Tcutil_compress(t);_tmp50D.f2=_tmp68D;});_tmp50D;});struct _tuple0 _tmp1F5=_tmp1F4;int _tmp1FD;void*_tmp1FC;void*_tmp1FB;void*_tmp1FA;if(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp1F5.f1)->tag == 34U){if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1F5.f2)->tag == 3U){_LL1: _tmp1FD=(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp1F5.f1)->f1).fat_result;_tmp1FC=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1F5.f2)->f1).ptr_atts).nullable;_tmp1FB=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1F5.f2)->f1).ptr_atts).bounds;_tmp1FA=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp1F5.f2)->f1).ptr_atts).zero_term;_LL2:
# 1225
 if((_tmp1FD  && !Cyc_Tcutil_force_type2bool(0,_tmp1FA)) && Cyc_Tcutil_force_type2bool(0,_tmp1FC)){
struct Cyc_Absyn_Exp*_tmp1F6=({void*_tmp68E=Cyc_Absyn_bounds_one();Cyc_Tcutil_get_bounds_exp(_tmp68E,_tmp1FB);});
if(_tmp1F6 != 0){
struct Cyc_Absyn_Exp*_tmp1F7=e;
if((Cyc_Evexp_eval_const_uint_exp(_tmp1F7)).f1 == 1)
({void*_tmp1F8=0U;({unsigned int _tmp690=loc;struct _dyneither_ptr _tmp68F=({const char*_tmp1F9="cast from ? pointer to * pointer will lose size information";_tag_dyneither(_tmp1F9,sizeof(char),60U);});Cyc_Tcutil_warn(_tmp690,_tmp68F,_tag_dyneither(_tmp1F8,sizeof(void*),0U));});});}}
# 1233
goto _LL0;}else{goto _LL3;}}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 1236
return t;};}
# 1240
static void*Cyc_Tcexp_tcAddress(struct Cyc_Tcenv_Tenv*te,unsigned int loc,struct Cyc_Absyn_Exp*e0,void**topt,struct Cyc_Absyn_Exp*e){
void**_tmp1FE=0;
struct Cyc_Absyn_Tqual _tmp1FF=Cyc_Absyn_empty_tqual(0U);
int _tmp200=0;
if(topt != 0){
void*_tmp201=Cyc_Tcutil_compress(*topt);void*_tmp202=_tmp201;void**_tmp205;struct Cyc_Absyn_Tqual _tmp204;void*_tmp203;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp202)->tag == 3U){_LL1: _tmp205=(void**)&(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp202)->f1).elt_type;_tmp204=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp202)->f1).elt_tq;_tmp203=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp202)->f1).ptr_atts).nullable;_LL2:
# 1247
 _tmp1FE=_tmp205;
_tmp1FF=_tmp204;
_tmp200=Cyc_Tcutil_force_type2bool(0,_tmp203);
goto _LL0;}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 1261
({struct Cyc_Tcenv_Tenv*_tmp692=Cyc_Tcenv_enter_abstract_val_ok(Cyc_Tcenv_enter_lhs(Cyc_Tcenv_clear_notreadctxt(te)));void**_tmp691=_tmp1FE;Cyc_Tcexp_tcExpNoInst(_tmp692,_tmp691,e);});
# 1264
if(Cyc_Tcutil_is_noalias_pointer_or_aggr((void*)_check_null(e->topt)))
({void*_tmp206=0U;({unsigned int _tmp694=e->loc;struct _dyneither_ptr _tmp693=({const char*_tmp207="Cannot take the address of an alias-free path";_tag_dyneither(_tmp207,sizeof(char),46U);});Cyc_Tcutil_terr(_tmp694,_tmp693,_tag_dyneither(_tmp206,sizeof(void*),0U));});});
# 1270
{void*_tmp208=e->r;void*_tmp209=_tmp208;struct Cyc_Absyn_Exp*_tmp20D;struct Cyc_Absyn_Exp*_tmp20C;if(((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp209)->tag == 23U){_LL6: _tmp20D=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp209)->f1;_tmp20C=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp209)->f2;_LL7:
# 1272
{void*_tmp20A=Cyc_Tcutil_compress((void*)_check_null(_tmp20D->topt));void*_tmp20B=_tmp20A;if(((struct Cyc_Absyn_TupleType_Absyn_Type_struct*)_tmp20B)->tag == 6U){_LLB: _LLC:
 goto _LLA;}else{_LLD: _LLE:
# 1277
({void*_tmp695=(Cyc_Absyn_add_exp(_tmp20D,_tmp20C,0U))->r;e0->r=_tmp695;});
return Cyc_Tcexp_tcPlus(te,_tmp20D,_tmp20C);}_LLA:;}
# 1280
goto _LL5;}else{_LL8: _LL9:
 goto _LL5;}_LL5:;}
# 1285
{void*_tmp20E=e->r;void*_tmp20F=_tmp20E;switch(*((int*)_tmp20F)){case 21U: if(((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp20F)->f3 == 1){_LL10: _LL11:
 goto _LL13;}else{goto _LL14;}case 22U: if(((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp20F)->f3 == 1){_LL12: _LL13:
# 1288
({void*_tmp210=0U;({unsigned int _tmp697=e->loc;struct _dyneither_ptr _tmp696=({const char*_tmp211="cannot take the address of a @tagged union member";_tag_dyneither(_tmp211,sizeof(char),50U);});Cyc_Tcutil_terr(_tmp697,_tmp696,_tag_dyneither(_tmp210,sizeof(void*),0U));});});goto _LLF;}else{goto _LL14;}default: _LL14: _LL15:
 goto _LLF;}_LLF:;}{
# 1293
struct _tuple14 _tmp212=Cyc_Tcutil_addressof_props(te,e);struct _tuple14 _tmp213=_tmp212;int _tmp217;void*_tmp216;_LL17: _tmp217=_tmp213.f1;_tmp216=_tmp213.f2;_LL18:;
# 1295
if(Cyc_Tcutil_is_noalias_region(_tmp216,0))
({void*_tmp214=0U;({unsigned int _tmp699=e->loc;struct _dyneither_ptr _tmp698=({const char*_tmp215="using & would manufacture an alias to an alias-free pointer";_tag_dyneither(_tmp215,sizeof(char),60U);});Cyc_Tcutil_terr(_tmp699,_tmp698,_tag_dyneither(_tmp214,sizeof(void*),0U));});});{
# 1298
struct Cyc_Absyn_Tqual tq=Cyc_Absyn_empty_tqual(0U);
if(_tmp217){
tq.print_const=1;
tq.real_const=1;}{
# 1304
void*t=_tmp200?
Cyc_Absyn_star_type((void*)_check_null(e->topt),_tmp216,tq,Cyc_Absyn_false_type):
 Cyc_Absyn_at_type((void*)_check_null(e->topt),_tmp216,tq,Cyc_Absyn_false_type);
return t;};};};}
# 1311
static void*Cyc_Tcexp_tcSizeof(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,void*t){
if(te->allow_valueof)
# 1315
return Cyc_Absyn_uint_type;
# 1317
({unsigned int _tmp69C=loc;struct Cyc_Tcenv_Tenv*_tmp69B=te;struct Cyc_List_List*_tmp69A=Cyc_Tcenv_lookup_type_vars(te);Cyc_Tcutil_check_type(_tmp69C,_tmp69B,_tmp69A,& Cyc_Tcutil_tmk,1,0,t);});
Cyc_Tcutil_check_no_qual(loc,t);
if(!Cyc_Evexp_okay_szofarg(t))
({struct Cyc_String_pa_PrintArg_struct _tmp21A=({struct Cyc_String_pa_PrintArg_struct _tmp50E;_tmp50E.tag=0U,({
struct _dyneither_ptr _tmp69D=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp50E.f1=_tmp69D;});_tmp50E;});void*_tmp218[1U];_tmp218[0]=& _tmp21A;({unsigned int _tmp69F=loc;struct _dyneither_ptr _tmp69E=({const char*_tmp219="sizeof applied to type %s, which has unknown size here";_tag_dyneither(_tmp219,sizeof(char),55U);});Cyc_Tcutil_terr(_tmp69F,_tmp69E,_tag_dyneither(_tmp218,sizeof(void*),1U));});});
if(topt != 0){
void*_tmp21B=Cyc_Tcutil_compress(*topt);void*_tmp21C=_tmp21B;void*_tmp221;void*_tmp220;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp21C)->tag == 0U){if(((struct Cyc_Absyn_TagCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp21C)->f1)->tag == 4U){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp21C)->f2 != 0){_LL1: _tmp221=_tmp21C;_tmp220=(void*)(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp21C)->f2)->hd;_LL2: {
# 1326
struct Cyc_Absyn_Exp*_tmp21D=Cyc_Absyn_sizeoftype_exp(t,0U);
struct Cyc_Absyn_ValueofType_Absyn_Type_struct*_tmp21E=({struct Cyc_Absyn_ValueofType_Absyn_Type_struct*_tmp21F=_cycalloc(sizeof(*_tmp21F));_tmp21F->tag=9U,_tmp21F->f1=_tmp21D;_tmp21F;});
if(Cyc_Tcutil_unify(_tmp220,(void*)_tmp21E))return _tmp221;
goto _LL0;}}else{goto _LL3;}}else{goto _LL3;}}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 1332
return Cyc_Absyn_uint_type;}
# 1335
void*Cyc_Tcexp_structfield_type(struct _dyneither_ptr*n,struct Cyc_Absyn_Aggrfield*sf){
if(Cyc_strcmp((struct _dyneither_ptr)*n,(struct _dyneither_ptr)*sf->name)== 0)return sf->type;else{
return 0;}}
# 1342
static void*Cyc_Tcexp_tcOffsetof(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,void*t_orig,struct Cyc_List_List*fs){
# 1344
({unsigned int _tmp6A2=loc;struct Cyc_Tcenv_Tenv*_tmp6A1=te;struct Cyc_List_List*_tmp6A0=Cyc_Tcenv_lookup_type_vars(te);Cyc_Tcutil_check_type(_tmp6A2,_tmp6A1,_tmp6A0,& Cyc_Tcutil_tmk,1,0,t_orig);});
Cyc_Tcutil_check_no_qual(loc,t_orig);{
struct Cyc_List_List*_tmp222=fs;
void*_tmp223=t_orig;
for(0;_tmp222 != 0;_tmp222=_tmp222->tl){
void*_tmp224=(void*)_tmp222->hd;
void*_tmp225=_tmp224;unsigned int _tmp25F;struct _dyneither_ptr*_tmp25E;if(((struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct*)_tmp225)->tag == 0U){_LL1: _tmp25E=((struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct*)_tmp225)->f1;_LL2: {
# 1352
int bad_type=1;
{void*_tmp226=Cyc_Tcutil_compress(_tmp223);void*_tmp227=_tmp226;struct Cyc_List_List*_tmp22F;struct Cyc_Absyn_Aggrdecl**_tmp22E;switch(*((int*)_tmp227)){case 0U: if(((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp227)->f1)->tag == 20U){if(((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp227)->f1)->f1).KnownAggr).tag == 2){_LL6: _tmp22E=((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp227)->f1)->f1).KnownAggr).val;_LL7:
# 1355
 if((*_tmp22E)->impl == 0)goto _LL5;{
void*t2=((void*(*)(void*(*pred)(struct _dyneither_ptr*,struct Cyc_Absyn_Aggrfield*),struct _dyneither_ptr*env,struct Cyc_List_List*x))Cyc_List_find_c)(Cyc_Tcexp_structfield_type,_tmp25E,((struct Cyc_Absyn_AggrdeclImpl*)_check_null((*_tmp22E)->impl))->fields);
if(!((unsigned int)t2))
({struct Cyc_String_pa_PrintArg_struct _tmp22A=({struct Cyc_String_pa_PrintArg_struct _tmp50F;_tmp50F.tag=0U,_tmp50F.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp25E);_tmp50F;});void*_tmp228[1U];_tmp228[0]=& _tmp22A;({unsigned int _tmp6A4=loc;struct _dyneither_ptr _tmp6A3=({const char*_tmp229="no field of struct/union has name %s";_tag_dyneither(_tmp229,sizeof(char),37U);});Cyc_Tcutil_terr(_tmp6A4,_tmp6A3,_tag_dyneither(_tmp228,sizeof(void*),1U));});});else{
# 1360
_tmp223=t2;}
bad_type=0;
goto _LL5;};}else{goto _LLA;}}else{goto _LLA;}case 7U: _LL8: _tmp22F=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp227)->f2;_LL9: {
# 1364
void*t2=((void*(*)(void*(*pred)(struct _dyneither_ptr*,struct Cyc_Absyn_Aggrfield*),struct _dyneither_ptr*env,struct Cyc_List_List*x))Cyc_List_find_c)(Cyc_Tcexp_structfield_type,_tmp25E,_tmp22F);
if(!((unsigned int)t2))
({struct Cyc_String_pa_PrintArg_struct _tmp22D=({struct Cyc_String_pa_PrintArg_struct _tmp510;_tmp510.tag=0U,_tmp510.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp25E);_tmp510;});void*_tmp22B[1U];_tmp22B[0]=& _tmp22D;({unsigned int _tmp6A6=loc;struct _dyneither_ptr _tmp6A5=({const char*_tmp22C="no field of struct/union has name %s";_tag_dyneither(_tmp22C,sizeof(char),37U);});Cyc_Tcutil_terr(_tmp6A6,_tmp6A5,_tag_dyneither(_tmp22B,sizeof(void*),1U));});});else{
# 1368
_tmp223=t2;}
bad_type=0;
goto _LL5;}default: _LLA: _LLB:
 goto _LL5;}_LL5:;}
# 1373
if(bad_type){
if(_tmp222 == fs)
({struct Cyc_String_pa_PrintArg_struct _tmp232=({struct Cyc_String_pa_PrintArg_struct _tmp511;_tmp511.tag=0U,({struct _dyneither_ptr _tmp6A7=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp223));_tmp511.f1=_tmp6A7;});_tmp511;});void*_tmp230[1U];_tmp230[0]=& _tmp232;({unsigned int _tmp6A9=loc;struct _dyneither_ptr _tmp6A8=({const char*_tmp231="%s is not a known struct/union type";_tag_dyneither(_tmp231,sizeof(char),36U);});Cyc_Tcutil_terr(_tmp6A9,_tmp6A8,_tag_dyneither(_tmp230,sizeof(void*),1U));});});else{
# 1377
struct _dyneither_ptr _tmp233=({struct Cyc_String_pa_PrintArg_struct _tmp246=({struct Cyc_String_pa_PrintArg_struct _tmp518;_tmp518.tag=0U,({struct _dyneither_ptr _tmp6AA=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t_orig));_tmp518.f1=_tmp6AA;});_tmp518;});void*_tmp244[1U];_tmp244[0]=& _tmp246;({struct _dyneither_ptr _tmp6AB=({const char*_tmp245="(%s)";_tag_dyneither(_tmp245,sizeof(char),5U);});Cyc_aprintf(_tmp6AB,_tag_dyneither(_tmp244,sizeof(void*),1U));});});
struct Cyc_List_List*x;
for(x=fs;x != _tmp222;x=x->tl){
void*_tmp234=(void*)((struct Cyc_List_List*)_check_null(x))->hd;void*_tmp235=_tmp234;unsigned int _tmp23F;struct _dyneither_ptr*_tmp23E;if(((struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct*)_tmp235)->tag == 0U){_LLD: _tmp23E=((struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct*)_tmp235)->f1;_LLE:
# 1382
 _tmp233=({struct Cyc_String_pa_PrintArg_struct _tmp238=({struct Cyc_String_pa_PrintArg_struct _tmp513;_tmp513.tag=0U,_tmp513.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp233);_tmp513;});struct Cyc_String_pa_PrintArg_struct _tmp239=({struct Cyc_String_pa_PrintArg_struct _tmp512;_tmp512.tag=0U,_tmp512.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp23E);_tmp512;});void*_tmp236[2U];_tmp236[0]=& _tmp238,_tmp236[1]=& _tmp239;({struct _dyneither_ptr _tmp6AC=({const char*_tmp237="%s.%s";_tag_dyneither(_tmp237,sizeof(char),6U);});Cyc_aprintf(_tmp6AC,_tag_dyneither(_tmp236,sizeof(void*),2U));});});goto _LLC;}else{_LLF: _tmp23F=((struct Cyc_Absyn_TupleIndex_Absyn_OffsetofField_struct*)_tmp235)->f1;_LL10:
# 1384
 _tmp233=({struct Cyc_String_pa_PrintArg_struct _tmp23C=({struct Cyc_String_pa_PrintArg_struct _tmp515;_tmp515.tag=0U,_tmp515.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp233);_tmp515;});struct Cyc_Int_pa_PrintArg_struct _tmp23D=({struct Cyc_Int_pa_PrintArg_struct _tmp514;_tmp514.tag=1U,_tmp514.f1=(unsigned long)((int)_tmp23F);_tmp514;});void*_tmp23A[2U];_tmp23A[0]=& _tmp23C,_tmp23A[1]=& _tmp23D;({struct _dyneither_ptr _tmp6AD=({const char*_tmp23B="%s.%d";_tag_dyneither(_tmp23B,sizeof(char),6U);});Cyc_aprintf(_tmp6AD,_tag_dyneither(_tmp23A,sizeof(void*),2U));});});goto _LLC;}_LLC:;}
# 1386
({struct Cyc_String_pa_PrintArg_struct _tmp242=({struct Cyc_String_pa_PrintArg_struct _tmp517;_tmp517.tag=0U,_tmp517.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)_tmp233);_tmp517;});struct Cyc_String_pa_PrintArg_struct _tmp243=({struct Cyc_String_pa_PrintArg_struct _tmp516;_tmp516.tag=0U,({struct _dyneither_ptr _tmp6AE=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp223));_tmp516.f1=_tmp6AE;});_tmp516;});void*_tmp240[2U];_tmp240[0]=& _tmp242,_tmp240[1]=& _tmp243;({unsigned int _tmp6B0=loc;struct _dyneither_ptr _tmp6AF=({const char*_tmp241="%s == %s is not a struct/union type";_tag_dyneither(_tmp241,sizeof(char),36U);});Cyc_Tcutil_terr(_tmp6B0,_tmp6AF,_tag_dyneither(_tmp240,sizeof(void*),2U));});});}}
# 1389
goto _LL0;}}else{_LL3: _tmp25F=((struct Cyc_Absyn_TupleIndex_Absyn_OffsetofField_struct*)_tmp225)->f1;_LL4: {
# 1391
int bad_type=1;
{void*_tmp247=Cyc_Tcutil_compress(_tmp223);void*_tmp248=_tmp247;struct Cyc_List_List*_tmp25A;struct Cyc_List_List*_tmp259;struct Cyc_Absyn_Datatypefield*_tmp258;struct Cyc_Absyn_Aggrdecl**_tmp257;switch(*((int*)_tmp248)){case 0U: switch(*((int*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp248)->f1)){case 20U: if(((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp248)->f1)->f1).KnownAggr).tag == 2){_LL12: _tmp257=((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp248)->f1)->f1).KnownAggr).val;_LL13:
# 1394
 if((*_tmp257)->impl == 0)
goto _LL11;
_tmp259=((struct Cyc_Absyn_AggrdeclImpl*)_check_null((*_tmp257)->impl))->fields;goto _LL15;}else{goto _LL1A;}case 19U: if(((((struct Cyc_Absyn_DatatypeFieldCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp248)->f1)->f1).KnownDatatypefield).tag == 2){_LL18: _tmp258=(((((struct Cyc_Absyn_DatatypeFieldCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp248)->f1)->f1).KnownDatatypefield).val).f2;_LL19:
# 1414
 if(({int _tmp6B1=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp258->typs);_tmp6B1 < _tmp25F;}))
({struct Cyc_Int_pa_PrintArg_struct _tmp253=({struct Cyc_Int_pa_PrintArg_struct _tmp51A;_tmp51A.tag=1U,({
unsigned long _tmp6B2=(unsigned long)((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp258->typs);_tmp51A.f1=_tmp6B2;});_tmp51A;});struct Cyc_Int_pa_PrintArg_struct _tmp254=({struct Cyc_Int_pa_PrintArg_struct _tmp519;_tmp519.tag=1U,_tmp519.f1=(unsigned long)((int)_tmp25F);_tmp519;});void*_tmp251[2U];_tmp251[0]=& _tmp253,_tmp251[1]=& _tmp254;({unsigned int _tmp6B4=loc;struct _dyneither_ptr _tmp6B3=({const char*_tmp252="datatype field has too few components: %d < %d";_tag_dyneither(_tmp252,sizeof(char),47U);});Cyc_Tcutil_terr(_tmp6B4,_tmp6B3,_tag_dyneither(_tmp251,sizeof(void*),2U));});});else{
# 1418
if(_tmp25F != 0)
_tmp223=(*((struct _tuple17*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)(_tmp258->typs,(int)(_tmp25F - 1))).f2;else{
if(_tmp222->tl != 0)
({void*_tmp255=0U;({unsigned int _tmp6B6=loc;struct _dyneither_ptr _tmp6B5=({const char*_tmp256="datatype field index 0 refers to the tag; cannot be indexed through";_tag_dyneither(_tmp256,sizeof(char),68U);});Cyc_Tcutil_terr(_tmp6B6,_tmp6B5,_tag_dyneither(_tmp255,sizeof(void*),0U));});});}}
# 1423
bad_type=0;
goto _LL11;}else{goto _LL1A;}default: goto _LL1A;}case 7U: _LL14: _tmp259=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp248)->f2;_LL15:
# 1398
 if(({int _tmp6B7=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp259);_tmp6B7 <= _tmp25F;}))
({struct Cyc_Int_pa_PrintArg_struct _tmp24B=({struct Cyc_Int_pa_PrintArg_struct _tmp51C;_tmp51C.tag=1U,({
unsigned long _tmp6B8=(unsigned long)((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp259);_tmp51C.f1=_tmp6B8;});_tmp51C;});struct Cyc_Int_pa_PrintArg_struct _tmp24C=({struct Cyc_Int_pa_PrintArg_struct _tmp51B;_tmp51B.tag=1U,_tmp51B.f1=(unsigned long)((int)_tmp25F);_tmp51B;});void*_tmp249[2U];_tmp249[0]=& _tmp24B,_tmp249[1]=& _tmp24C;({unsigned int _tmp6BA=loc;struct _dyneither_ptr _tmp6B9=({const char*_tmp24A="struct/union has too few components: %d <= %d";_tag_dyneither(_tmp24A,sizeof(char),46U);});Cyc_Tcutil_terr(_tmp6BA,_tmp6B9,_tag_dyneither(_tmp249,sizeof(void*),2U));});});else{
# 1402
_tmp223=(((struct Cyc_Absyn_Aggrfield*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)(_tmp259,(int)_tmp25F))->type;}
bad_type=0;
goto _LL11;case 6U: _LL16: _tmp25A=((struct Cyc_Absyn_TupleType_Absyn_Type_struct*)_tmp248)->f1;_LL17:
# 1406
 if(({int _tmp6BB=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp25A);_tmp6BB <= _tmp25F;}))
({struct Cyc_Int_pa_PrintArg_struct _tmp24F=({struct Cyc_Int_pa_PrintArg_struct _tmp51E;_tmp51E.tag=1U,({
unsigned long _tmp6BC=(unsigned long)((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp25A);_tmp51E.f1=_tmp6BC;});_tmp51E;});struct Cyc_Int_pa_PrintArg_struct _tmp250=({struct Cyc_Int_pa_PrintArg_struct _tmp51D;_tmp51D.tag=1U,_tmp51D.f1=(unsigned long)((int)_tmp25F);_tmp51D;});void*_tmp24D[2U];_tmp24D[0]=& _tmp24F,_tmp24D[1]=& _tmp250;({unsigned int _tmp6BE=loc;struct _dyneither_ptr _tmp6BD=({const char*_tmp24E="tuple has too few components: %d <= %d";_tag_dyneither(_tmp24E,sizeof(char),39U);});Cyc_Tcutil_terr(_tmp6BE,_tmp6BD,_tag_dyneither(_tmp24D,sizeof(void*),2U));});});else{
# 1410
_tmp223=(*((struct _tuple17*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)(_tmp25A,(int)_tmp25F)).f2;}
bad_type=0;
goto _LL11;default: _LL1A: _LL1B:
# 1425
 goto _LL11;}_LL11:;}
# 1427
if(bad_type)
({struct Cyc_String_pa_PrintArg_struct _tmp25D=({struct Cyc_String_pa_PrintArg_struct _tmp51F;_tmp51F.tag=0U,({struct _dyneither_ptr _tmp6BF=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp223));_tmp51F.f1=_tmp6BF;});_tmp51F;});void*_tmp25B[1U];_tmp25B[0]=& _tmp25D;({unsigned int _tmp6C1=loc;struct _dyneither_ptr _tmp6C0=({const char*_tmp25C="%s is not a known type";_tag_dyneither(_tmp25C,sizeof(char),23U);});Cyc_Tcutil_terr(_tmp6C1,_tmp6C0,_tag_dyneither(_tmp25B,sizeof(void*),1U));});});
goto _LL0;}}_LL0:;}
# 1432
return Cyc_Absyn_uint_type;};}
# 1436
static void*Cyc_Tcexp_tcDeref(struct Cyc_Tcenv_Tenv*te_orig,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e){
struct Cyc_Tcenv_Tenv*_tmp260=Cyc_Tcenv_clear_lhs(Cyc_Tcenv_clear_notreadctxt(te_orig));
Cyc_Tcexp_tcExp(_tmp260,0,e);{
void*t=Cyc_Tcutil_compress((void*)_check_null(e->topt));
void*_tmp261=t;void*_tmp278;void*_tmp277;void*_tmp276;void*_tmp275;switch(*((int*)_tmp261)){case 1U: _LL1: _LL2: {
# 1442
struct Cyc_List_List*_tmp262=Cyc_Tcenv_lookup_type_vars(_tmp260);
void*_tmp263=Cyc_Absyn_new_evar(& Cyc_Tcutil_ako,({struct Cyc_Core_Opt*_tmp26B=_cycalloc(sizeof(*_tmp26B));_tmp26B->v=_tmp262;_tmp26B;}));
void*_tmp264=Cyc_Absyn_new_evar(& Cyc_Tcutil_trko,({struct Cyc_Core_Opt*_tmp26A=_cycalloc(sizeof(*_tmp26A));_tmp26A->v=_tmp262;_tmp26A;}));
void*_tmp265=Cyc_Tcutil_any_bounds(& _tmp260);
void*_tmp266=Cyc_Tcutil_any_bool(& _tmp260);
struct Cyc_Absyn_PtrAtts _tmp267=({struct Cyc_Absyn_PtrAtts _tmp520;_tmp520.rgn=_tmp264,({void*_tmp6C2=Cyc_Tcutil_any_bool(& _tmp260);_tmp520.nullable=_tmp6C2;}),_tmp520.bounds=_tmp265,_tmp520.zero_term=_tmp266,_tmp520.ptrloc=0;_tmp520;});
struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp268=({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp269=_cycalloc(sizeof(*_tmp269));_tmp269->tag=3U,(_tmp269->f1).elt_type=_tmp263,({struct Cyc_Absyn_Tqual _tmp6C3=Cyc_Absyn_empty_tqual(0U);(_tmp269->f1).elt_tq=_tmp6C3;}),(_tmp269->f1).ptr_atts=_tmp267;_tmp269;});
Cyc_Tcutil_unify(t,(void*)_tmp268);
_tmp278=_tmp263;_tmp277=_tmp264;_tmp276=_tmp265;_tmp275=_tmp266;goto _LL4;}case 3U: _LL3: _tmp278=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp261)->f1).elt_type;_tmp277=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp261)->f1).ptr_atts).rgn;_tmp276=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp261)->f1).ptr_atts).bounds;_tmp275=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp261)->f1).ptr_atts).zero_term;_LL4:
# 1452
 Cyc_Tcenv_check_rgn_accessible(_tmp260,loc,_tmp277);
Cyc_Tcutil_check_nonzero_bound(loc,_tmp276);
if(!Cyc_Tcutil_kind_leq(Cyc_Tcutil_type_kind(_tmp278),& Cyc_Tcutil_tmk) && !Cyc_Tcenv_abstract_val_ok(_tmp260)){
void*_tmp26C=Cyc_Tcutil_compress(_tmp278);void*_tmp26D=_tmp26C;if(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp26D)->tag == 5U){_LL8: _LL9:
# 1457
 if(Cyc_Tc_aggressive_warn)
({void*_tmp26E=0U;({unsigned int _tmp6C5=loc;struct _dyneither_ptr _tmp6C4=({const char*_tmp26F="unnecessary dereference for function type";_tag_dyneither(_tmp26F,sizeof(char),42U);});Cyc_Tcutil_warn(_tmp6C5,_tmp6C4,_tag_dyneither(_tmp26E,sizeof(void*),0U));});});
return t;}else{_LLA: _LLB:
# 1461
({void*_tmp270=0U;({unsigned int _tmp6C7=loc;struct _dyneither_ptr _tmp6C6=({const char*_tmp271="can't dereference abstract pointer type";_tag_dyneither(_tmp271,sizeof(char),40U);});Cyc_Tcutil_terr(_tmp6C7,_tmp6C6,_tag_dyneither(_tmp270,sizeof(void*),0U));});});}_LL7:;}
# 1464
return _tmp278;default: _LL5: _LL6:
# 1466
 return({struct Cyc_String_pa_PrintArg_struct _tmp274=({struct Cyc_String_pa_PrintArg_struct _tmp521;_tmp521.tag=0U,({struct _dyneither_ptr _tmp6C8=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp521.f1=_tmp6C8;});_tmp521;});void*_tmp272[1U];_tmp272[0]=& _tmp274;({struct Cyc_Tcenv_Tenv*_tmp6CC=_tmp260;unsigned int _tmp6CB=loc;void**_tmp6CA=topt;struct _dyneither_ptr _tmp6C9=({const char*_tmp273="expecting *, @, or ? type but found %s";_tag_dyneither(_tmp273,sizeof(char),39U);});Cyc_Tcexp_expr_err(_tmp6CC,_tmp6CB,_tmp6CA,_tmp6C9,_tag_dyneither(_tmp272,sizeof(void*),1U));});});}_LL0:;};}
# 1471
static void*Cyc_Tcexp_tcAggrMember(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*outer_e,struct Cyc_Absyn_Exp*e,struct _dyneither_ptr*f,int*is_tagged,int*is_read){
# 1475
({struct Cyc_Tcenv_Tenv*_tmp6CD=Cyc_Tcenv_enter_abstract_val_ok(te);Cyc_Tcexp_tcExpNoPromote(_tmp6CD,0,e);});
# 1477
({int _tmp6CE=!Cyc_Tcenv_in_notreadctxt(te);*is_read=_tmp6CE;});{
void*_tmp279=Cyc_Tcutil_compress((void*)_check_null(e->topt));void*_tmp27A=_tmp279;enum Cyc_Absyn_AggrKind _tmp294;struct Cyc_List_List*_tmp293;struct Cyc_Absyn_Aggrdecl*_tmp292;struct Cyc_List_List*_tmp291;switch(*((int*)_tmp27A)){case 0U: if(((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp27A)->f1)->tag == 20U){if(((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp27A)->f1)->f1).KnownAggr).tag == 2){_LL1: _tmp292=*((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp27A)->f1)->f1).KnownAggr).val;_tmp291=((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp27A)->f2;_LL2: {
# 1480
struct Cyc_Absyn_Aggrfield*_tmp27B=Cyc_Absyn_lookup_decl_field(_tmp292,f);
if(_tmp27B == 0)
return({struct Cyc_String_pa_PrintArg_struct _tmp27E=({struct Cyc_String_pa_PrintArg_struct _tmp523;_tmp523.tag=0U,({
struct _dyneither_ptr _tmp6CF=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp292->name));_tmp523.f1=_tmp6CF;});_tmp523;});struct Cyc_String_pa_PrintArg_struct _tmp27F=({struct Cyc_String_pa_PrintArg_struct _tmp522;_tmp522.tag=0U,_tmp522.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp522;});void*_tmp27C[2U];_tmp27C[0]=& _tmp27E,_tmp27C[1]=& _tmp27F;({struct Cyc_Tcenv_Tenv*_tmp6D3=te;unsigned int _tmp6D2=loc;void**_tmp6D1=topt;struct _dyneither_ptr _tmp6D0=({const char*_tmp27D="%s has no %s member";_tag_dyneither(_tmp27D,sizeof(char),20U);});Cyc_Tcexp_expr_err(_tmp6D3,_tmp6D2,_tmp6D1,_tmp6D0,_tag_dyneither(_tmp27C,sizeof(void*),2U));});});
# 1485
if(((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp292->impl))->tagged)*is_tagged=1;{
void*t2=_tmp27B->type;
if(_tmp291 != 0){
struct Cyc_List_List*_tmp280=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_zip)(_tmp292->tvs,_tmp291);
t2=Cyc_Tcutil_substitute(_tmp280,_tmp27B->type);}
# 1493
if((((_tmp292->kind == Cyc_Absyn_UnionA  && !((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp292->impl))->tagged) && !Cyc_Tcutil_is_bits_only_type(t2)) && !Cyc_Tcenv_in_notreadctxt(te)) && _tmp27B->requires_clause == 0)
return({struct Cyc_String_pa_PrintArg_struct _tmp283=({struct Cyc_String_pa_PrintArg_struct _tmp524;_tmp524.tag=0U,_tmp524.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp524;});void*_tmp281[1U];_tmp281[0]=& _tmp283;({struct Cyc_Tcenv_Tenv*_tmp6D7=te;unsigned int _tmp6D6=loc;void**_tmp6D5=topt;struct _dyneither_ptr _tmp6D4=({const char*_tmp282="cannot read union member %s since it is not `bits-only'";_tag_dyneither(_tmp282,sizeof(char),56U);});Cyc_Tcexp_expr_err(_tmp6D7,_tmp6D6,_tmp6D5,_tmp6D4,_tag_dyneither(_tmp281,sizeof(void*),1U));});});
if(((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp292->impl))->exist_vars != 0){
# 1498
if(!({void*_tmp6D8=t2;Cyc_Tcutil_unify(_tmp6D8,Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te)));}))
return({struct Cyc_String_pa_PrintArg_struct _tmp286=({struct Cyc_String_pa_PrintArg_struct _tmp525;_tmp525.tag=0U,_tmp525.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp525;});void*_tmp284[1U];_tmp284[0]=& _tmp286;({struct Cyc_Tcenv_Tenv*_tmp6DC=te;unsigned int _tmp6DB=loc;void**_tmp6DA=topt;struct _dyneither_ptr _tmp6D9=({const char*_tmp285="must use pattern-matching to access field %s\n\tdue to existential type variables.";_tag_dyneither(_tmp285,sizeof(char),81U);});Cyc_Tcexp_expr_err(_tmp6DC,_tmp6DB,_tmp6DA,_tmp6D9,_tag_dyneither(_tmp284,sizeof(void*),1U));});});}
# 1501
return t2;};}}else{goto _LL5;}}else{goto _LL5;}case 7U: _LL3: _tmp294=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp27A)->f1;_tmp293=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp27A)->f2;_LL4: {
# 1503
struct Cyc_Absyn_Aggrfield*_tmp287=Cyc_Absyn_lookup_field(_tmp293,f);
if(_tmp287 == 0)
return({struct Cyc_String_pa_PrintArg_struct _tmp28A=({struct Cyc_String_pa_PrintArg_struct _tmp526;_tmp526.tag=0U,_tmp526.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp526;});void*_tmp288[1U];_tmp288[0]=& _tmp28A;({struct Cyc_Tcenv_Tenv*_tmp6E0=te;unsigned int _tmp6DF=loc;void**_tmp6DE=topt;struct _dyneither_ptr _tmp6DD=({const char*_tmp289="type has no %s member";_tag_dyneither(_tmp289,sizeof(char),22U);});Cyc_Tcexp_expr_err(_tmp6E0,_tmp6DF,_tmp6DE,_tmp6DD,_tag_dyneither(_tmp288,sizeof(void*),1U));});});
# 1508
if(((_tmp294 == Cyc_Absyn_UnionA  && !Cyc_Tcutil_is_bits_only_type(_tmp287->type)) && !Cyc_Tcenv_in_notreadctxt(te)) && _tmp287->requires_clause == 0)
# 1510
return({struct Cyc_String_pa_PrintArg_struct _tmp28D=({struct Cyc_String_pa_PrintArg_struct _tmp527;_tmp527.tag=0U,_tmp527.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp527;});void*_tmp28B[1U];_tmp28B[0]=& _tmp28D;({struct Cyc_Tcenv_Tenv*_tmp6E4=te;unsigned int _tmp6E3=loc;void**_tmp6E2=topt;struct _dyneither_ptr _tmp6E1=({const char*_tmp28C="cannot read union member %s since it is not `bits-only'";_tag_dyneither(_tmp28C,sizeof(char),56U);});Cyc_Tcexp_expr_err(_tmp6E4,_tmp6E3,_tmp6E2,_tmp6E1,_tag_dyneither(_tmp28B,sizeof(void*),1U));});});
return _tmp287->type;}default: _LL5: _LL6:
# 1513
 return({struct Cyc_String_pa_PrintArg_struct _tmp290=({struct Cyc_String_pa_PrintArg_struct _tmp528;_tmp528.tag=0U,({
struct _dyneither_ptr _tmp6E5=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(e->topt)));_tmp528.f1=_tmp6E5;});_tmp528;});void*_tmp28E[1U];_tmp28E[0]=& _tmp290;({struct Cyc_Tcenv_Tenv*_tmp6E9=te;unsigned int _tmp6E8=loc;void**_tmp6E7=topt;struct _dyneither_ptr _tmp6E6=({const char*_tmp28F="expecting struct or union, found %s";_tag_dyneither(_tmp28F,sizeof(char),36U);});Cyc_Tcexp_expr_err(_tmp6E9,_tmp6E8,_tmp6E7,_tmp6E6,_tag_dyneither(_tmp28E,sizeof(void*),1U));});});}_LL0:;};}
# 1519
static void*Cyc_Tcexp_tcAggrArrow(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e,struct _dyneither_ptr*f,int*is_tagged,int*is_read){
# 1522
({struct Cyc_Tcenv_Tenv*_tmp6EA=Cyc_Tcenv_enter_abstract_val_ok(Cyc_Tcenv_clear_lhs(Cyc_Tcenv_clear_notreadctxt(te)));Cyc_Tcexp_tcExp(_tmp6EA,0,e);});
# 1524
({int _tmp6EB=!Cyc_Tcenv_in_notreadctxt(te);*is_read=_tmp6EB;});
{void*_tmp295=Cyc_Tcutil_compress((void*)_check_null(e->topt));void*_tmp296=_tmp295;void*_tmp2B9;void*_tmp2B8;void*_tmp2B7;void*_tmp2B6;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp296)->tag == 3U){_LL1: _tmp2B9=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp296)->f1).elt_type;_tmp2B8=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp296)->f1).ptr_atts).rgn;_tmp2B7=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp296)->f1).ptr_atts).bounds;_tmp2B6=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp296)->f1).ptr_atts).zero_term;_LL2:
# 1527
 Cyc_Tcutil_check_nonzero_bound(loc,_tmp2B7);
{void*_tmp297=Cyc_Tcutil_compress(_tmp2B9);void*_tmp298=_tmp297;enum Cyc_Absyn_AggrKind _tmp2B5;struct Cyc_List_List*_tmp2B4;struct Cyc_Absyn_Aggrdecl*_tmp2B3;struct Cyc_List_List*_tmp2B2;switch(*((int*)_tmp298)){case 0U: if(((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp298)->f1)->tag == 20U){if(((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp298)->f1)->f1).KnownAggr).tag == 2){_LL6: _tmp2B3=*((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp298)->f1)->f1).KnownAggr).val;_tmp2B2=((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp298)->f2;_LL7: {
# 1530
struct Cyc_Absyn_Aggrfield*_tmp299=Cyc_Absyn_lookup_decl_field(_tmp2B3,f);
if(_tmp299 == 0)
return({struct Cyc_String_pa_PrintArg_struct _tmp29C=({struct Cyc_String_pa_PrintArg_struct _tmp52A;_tmp52A.tag=0U,({
struct _dyneither_ptr _tmp6EC=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp2B3->name));_tmp52A.f1=_tmp6EC;});_tmp52A;});struct Cyc_String_pa_PrintArg_struct _tmp29D=({struct Cyc_String_pa_PrintArg_struct _tmp529;_tmp529.tag=0U,_tmp529.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp529;});void*_tmp29A[2U];_tmp29A[0]=& _tmp29C,_tmp29A[1]=& _tmp29D;({struct Cyc_Tcenv_Tenv*_tmp6F0=te;unsigned int _tmp6EF=loc;void**_tmp6EE=topt;struct _dyneither_ptr _tmp6ED=({const char*_tmp29B="type %s has no %s member";_tag_dyneither(_tmp29B,sizeof(char),25U);});Cyc_Tcexp_expr_err(_tmp6F0,_tmp6EF,_tmp6EE,_tmp6ED,_tag_dyneither(_tmp29A,sizeof(void*),2U));});});
# 1535
if(((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp2B3->impl))->tagged)*is_tagged=1;{
void*t3=_tmp299->type;
if(_tmp2B2 != 0){
struct Cyc_List_List*_tmp29E=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_zip)(_tmp2B3->tvs,_tmp2B2);
t3=Cyc_Tcutil_substitute(_tmp29E,_tmp299->type);}{
# 1541
struct Cyc_Absyn_Kind*_tmp29F=Cyc_Tcutil_type_kind(t3);
# 1544
if(Cyc_Tcutil_kind_leq(& Cyc_Tcutil_ak,_tmp29F) && !Cyc_Tcenv_abstract_val_ok(te)){
void*_tmp2A0=Cyc_Tcutil_compress(t3);void*_tmp2A1=_tmp2A0;if(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp2A1)->tag == 4U){_LLD: _LLE:
 goto _LLC;}else{_LLF: _LL10:
# 1548
 return({struct Cyc_String_pa_PrintArg_struct _tmp2A4=({struct Cyc_String_pa_PrintArg_struct _tmp52B;_tmp52B.tag=0U,_tmp52B.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp52B;});void*_tmp2A2[1U];_tmp2A2[0]=& _tmp2A4;({struct Cyc_Tcenv_Tenv*_tmp6F4=te;unsigned int _tmp6F3=loc;void**_tmp6F2=topt;struct _dyneither_ptr _tmp6F1=({const char*_tmp2A3="cannot get member %s since its type is abstract";_tag_dyneither(_tmp2A3,sizeof(char),48U);});Cyc_Tcexp_expr_err(_tmp6F4,_tmp6F3,_tmp6F2,_tmp6F1,_tag_dyneither(_tmp2A2,sizeof(void*),1U));});});}_LLC:;}
# 1553
if((((_tmp2B3->kind == Cyc_Absyn_UnionA  && !((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp2B3->impl))->tagged) && !
Cyc_Tcutil_is_bits_only_type(t3)) && !Cyc_Tcenv_in_notreadctxt(te)) && _tmp299->requires_clause == 0)
# 1556
return({struct Cyc_String_pa_PrintArg_struct _tmp2A7=({struct Cyc_String_pa_PrintArg_struct _tmp52C;_tmp52C.tag=0U,_tmp52C.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp52C;});void*_tmp2A5[1U];_tmp2A5[0]=& _tmp2A7;({struct Cyc_Tcenv_Tenv*_tmp6F8=te;unsigned int _tmp6F7=loc;void**_tmp6F6=topt;struct _dyneither_ptr _tmp6F5=({const char*_tmp2A6="cannot read union member %s since it is not `bits-only'";_tag_dyneither(_tmp2A6,sizeof(char),56U);});Cyc_Tcexp_expr_err(_tmp6F8,_tmp6F7,_tmp6F6,_tmp6F5,_tag_dyneither(_tmp2A5,sizeof(void*),1U));});});
if(((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp2B3->impl))->exist_vars != 0){
if(!({void*_tmp6F9=t3;Cyc_Tcutil_unify(_tmp6F9,Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te)));}))
return({struct Cyc_String_pa_PrintArg_struct _tmp2AA=({struct Cyc_String_pa_PrintArg_struct _tmp52D;_tmp52D.tag=0U,_tmp52D.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp52D;});void*_tmp2A8[1U];_tmp2A8[0]=& _tmp2AA;({struct Cyc_Tcenv_Tenv*_tmp6FD=te;unsigned int _tmp6FC=loc;void**_tmp6FB=topt;struct _dyneither_ptr _tmp6FA=({const char*_tmp2A9="must use pattern-matching to access field %s\n\tdue to extistential types";_tag_dyneither(_tmp2A9,sizeof(char),72U);});Cyc_Tcexp_expr_err(_tmp6FD,_tmp6FC,_tmp6FB,_tmp6FA,_tag_dyneither(_tmp2A8,sizeof(void*),1U));});});}
# 1563
return t3;};};}}else{goto _LLA;}}else{goto _LLA;}case 7U: _LL8: _tmp2B5=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp298)->f1;_tmp2B4=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp298)->f2;_LL9: {
# 1565
struct Cyc_Absyn_Aggrfield*_tmp2AB=Cyc_Absyn_lookup_field(_tmp2B4,f);
if(_tmp2AB == 0)
return({struct Cyc_String_pa_PrintArg_struct _tmp2AE=({struct Cyc_String_pa_PrintArg_struct _tmp52E;_tmp52E.tag=0U,_tmp52E.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp52E;});void*_tmp2AC[1U];_tmp2AC[0]=& _tmp2AE;({struct Cyc_Tcenv_Tenv*_tmp701=te;unsigned int _tmp700=loc;void**_tmp6FF=topt;struct _dyneither_ptr _tmp6FE=({const char*_tmp2AD="type has no %s field";_tag_dyneither(_tmp2AD,sizeof(char),21U);});Cyc_Tcexp_expr_err(_tmp701,_tmp700,_tmp6FF,_tmp6FE,_tag_dyneither(_tmp2AC,sizeof(void*),1U));});});
# 1570
if((_tmp2B5 == Cyc_Absyn_UnionA  && !Cyc_Tcutil_is_bits_only_type(_tmp2AB->type)) && !Cyc_Tcenv_in_notreadctxt(te))
return({struct Cyc_String_pa_PrintArg_struct _tmp2B1=({struct Cyc_String_pa_PrintArg_struct _tmp52F;_tmp52F.tag=0U,_tmp52F.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*f);_tmp52F;});void*_tmp2AF[1U];_tmp2AF[0]=& _tmp2B1;({struct Cyc_Tcenv_Tenv*_tmp705=te;unsigned int _tmp704=loc;void**_tmp703=topt;struct _dyneither_ptr _tmp702=({const char*_tmp2B0="cannot read union member %s since it is not `bits-only'";_tag_dyneither(_tmp2B0,sizeof(char),56U);});Cyc_Tcexp_expr_err(_tmp705,_tmp704,_tmp703,_tmp702,_tag_dyneither(_tmp2AF,sizeof(void*),1U));});});
return _tmp2AB->type;}default: _LLA: _LLB:
 goto _LL5;}_LL5:;}
# 1575
goto _LL0;}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 1578
return({struct Cyc_String_pa_PrintArg_struct _tmp2BC=({struct Cyc_String_pa_PrintArg_struct _tmp530;_tmp530.tag=0U,({
struct _dyneither_ptr _tmp706=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(e->topt)));_tmp530.f1=_tmp706;});_tmp530;});void*_tmp2BA[1U];_tmp2BA[0]=& _tmp2BC;({struct Cyc_Tcenv_Tenv*_tmp70A=te;unsigned int _tmp709=loc;void**_tmp708=topt;struct _dyneither_ptr _tmp707=({const char*_tmp2BB="expecting struct or union pointer, found %s";_tag_dyneither(_tmp2BB,sizeof(char),44U);});Cyc_Tcexp_expr_err(_tmp70A,_tmp709,_tmp708,_tmp707,_tag_dyneither(_tmp2BA,sizeof(void*),1U));});});}
# 1583
static void*Cyc_Tcexp_ithTupleType(struct Cyc_Tcenv_Tenv*te,unsigned int loc,struct Cyc_List_List*ts,struct Cyc_Absyn_Exp*index){
# 1585
struct _tuple15 _tmp2BD=Cyc_Evexp_eval_const_uint_exp(index);struct _tuple15 _tmp2BE=_tmp2BD;unsigned int _tmp2CC;int _tmp2CB;_LL1: _tmp2CC=_tmp2BE.f1;_tmp2CB=_tmp2BE.f2;_LL2:;
if(!_tmp2CB)
return({void*_tmp2BF=0U;({struct Cyc_Tcenv_Tenv*_tmp70D=te;unsigned int _tmp70C=loc;struct _dyneither_ptr _tmp70B=({const char*_tmp2C0="tuple projection cannot use sizeof or offsetof";_tag_dyneither(_tmp2C0,sizeof(char),47U);});Cyc_Tcexp_expr_err(_tmp70D,_tmp70C,0,_tmp70B,_tag_dyneither(_tmp2BF,sizeof(void*),0U));});});{
# 1589
struct _handler_cons _tmp2C1;_push_handler(& _tmp2C1);{int _tmp2C3=0;if(setjmp(_tmp2C1.handler))_tmp2C3=1;if(!_tmp2C3){
{void*_tmp2C4=(*((struct _tuple17*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)(ts,(int)_tmp2CC)).f2;_npop_handler(0U);return _tmp2C4;};_pop_handler();}else{void*_tmp2C2=(void*)_exn_thrown;void*_tmp2C5=_tmp2C2;void*_tmp2CA;if(((struct Cyc_List_Nth_exn_struct*)_tmp2C5)->tag == Cyc_List_Nth){_LL4: _LL5:
# 1592
 return({struct Cyc_Int_pa_PrintArg_struct _tmp2C8=({struct Cyc_Int_pa_PrintArg_struct _tmp532;_tmp532.tag=1U,_tmp532.f1=(unsigned long)((int)_tmp2CC);_tmp532;});struct Cyc_Int_pa_PrintArg_struct _tmp2C9=({struct Cyc_Int_pa_PrintArg_struct _tmp531;_tmp531.tag=1U,({
unsigned long _tmp70E=(unsigned long)((int(*)(struct Cyc_List_List*x))Cyc_List_length)(ts);_tmp531.f1=_tmp70E;});_tmp531;});void*_tmp2C6[2U];_tmp2C6[0]=& _tmp2C8,_tmp2C6[1]=& _tmp2C9;({struct Cyc_Tcenv_Tenv*_tmp711=te;unsigned int _tmp710=loc;struct _dyneither_ptr _tmp70F=({const char*_tmp2C7="index is %d but tuple has only %d fields";_tag_dyneither(_tmp2C7,sizeof(char),41U);});Cyc_Tcexp_expr_err(_tmp711,_tmp710,0,_tmp70F,_tag_dyneither(_tmp2C6,sizeof(void*),2U));});});}else{_LL6: _tmp2CA=_tmp2C5;_LL7:(int)_rethrow(_tmp2CA);}_LL3:;}};};}
# 1597
static void*Cyc_Tcexp_tcSubscript(struct Cyc_Tcenv_Tenv*te_orig,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){
# 1599
struct Cyc_Tcenv_Tenv*_tmp2CD=Cyc_Tcenv_clear_lhs(Cyc_Tcenv_clear_notreadctxt(te_orig));
({struct Cyc_Tcenv_Tenv*_tmp712=Cyc_Tcenv_clear_abstract_val_ok(_tmp2CD);Cyc_Tcexp_tcExp(_tmp712,0,e1);});
({struct Cyc_Tcenv_Tenv*_tmp713=Cyc_Tcenv_clear_abstract_val_ok(_tmp2CD);Cyc_Tcexp_tcExp(_tmp713,0,e2);});{
void*t1=Cyc_Tcutil_compress((void*)_check_null(e1->topt));
void*t2=Cyc_Tcutil_compress((void*)_check_null(e2->topt));
if(!Cyc_Tcutil_coerce_sint_type(_tmp2CD,e2))
return({struct Cyc_String_pa_PrintArg_struct _tmp2D0=({struct Cyc_String_pa_PrintArg_struct _tmp533;_tmp533.tag=0U,({
struct _dyneither_ptr _tmp714=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t2));_tmp533.f1=_tmp714;});_tmp533;});void*_tmp2CE[1U];_tmp2CE[0]=& _tmp2D0;({struct Cyc_Tcenv_Tenv*_tmp718=_tmp2CD;unsigned int _tmp717=e2->loc;void**_tmp716=topt;struct _dyneither_ptr _tmp715=({const char*_tmp2CF="expecting int subscript, found %s";_tag_dyneither(_tmp2CF,sizeof(char),34U);});Cyc_Tcexp_expr_err(_tmp718,_tmp717,_tmp716,_tmp715,_tag_dyneither(_tmp2CE,sizeof(void*),1U));});});{
# 1609
void*_tmp2D1=t1;struct Cyc_List_List*_tmp2F4;void*_tmp2F3;struct Cyc_Absyn_Tqual _tmp2F2;void*_tmp2F1;void*_tmp2F0;void*_tmp2EF;switch(*((int*)_tmp2D1)){case 3U: _LL1: _tmp2F3=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp2D1)->f1).elt_type;_tmp2F2=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp2D1)->f1).elt_tq;_tmp2F1=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp2D1)->f1).ptr_atts).rgn;_tmp2F0=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp2D1)->f1).ptr_atts).bounds;_tmp2EF=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp2D1)->f1).ptr_atts).zero_term;_LL2:
# 1613
 if(Cyc_Tcutil_force_type2bool(0,_tmp2EF)){
int emit_warning=0;
struct Cyc_Absyn_Exp*_tmp2D2=Cyc_Tcutil_get_bounds_exp(Cyc_Absyn_fat_bound_type,_tmp2F0);
if(_tmp2D2 != 0){
struct Cyc_Absyn_Exp*_tmp2D3=_tmp2D2;
struct _tuple15 _tmp2D4=Cyc_Evexp_eval_const_uint_exp(_tmp2D3);struct _tuple15 _tmp2D5=_tmp2D4;unsigned int _tmp2E1;int _tmp2E0;_LL8: _tmp2E1=_tmp2D5.f1;_tmp2E0=_tmp2D5.f2;_LL9:;
if(_tmp2E0  && _tmp2E1 == 1)emit_warning=1;
if(Cyc_Tcutil_is_const_exp(e2)){
struct _tuple15 _tmp2D6=Cyc_Evexp_eval_const_uint_exp(e2);struct _tuple15 _tmp2D7=_tmp2D6;unsigned int _tmp2DF;int _tmp2DE;_LLB: _tmp2DF=_tmp2D7.f1;_tmp2DE=_tmp2D7.f2;_LLC:;
if(_tmp2DE){
struct _tuple15 _tmp2D8=Cyc_Evexp_eval_const_uint_exp(_tmp2D3);struct _tuple15 _tmp2D9=_tmp2D8;unsigned int _tmp2DD;int _tmp2DC;_LLE: _tmp2DD=_tmp2D9.f1;_tmp2DC=_tmp2D9.f2;_LLF:;
if((_tmp2E0  && _tmp2DD > _tmp2E1) && _tmp2E1 != 1)
({void*_tmp2DA=0U;({unsigned int _tmp71A=loc;struct _dyneither_ptr _tmp719=({const char*_tmp2DB="subscript will fail at run-time";_tag_dyneither(_tmp2DB,sizeof(char),32U);});Cyc_Tcutil_terr(_tmp71A,_tmp719,_tag_dyneither(_tmp2DA,sizeof(void*),0U));});});}}}
# 1629
if(emit_warning)
({void*_tmp2E2=0U;({unsigned int _tmp71C=e2->loc;struct _dyneither_ptr _tmp71B=({const char*_tmp2E3="subscript on thin, zero-terminated pointer could be expensive.";_tag_dyneither(_tmp2E3,sizeof(char),63U);});Cyc_Tcutil_warn(_tmp71C,_tmp71B,_tag_dyneither(_tmp2E2,sizeof(void*),0U));});});}else{
# 1633
if(Cyc_Tcutil_is_const_exp(e2)){
struct _tuple15 _tmp2E4=Cyc_Evexp_eval_const_uint_exp(e2);struct _tuple15 _tmp2E5=_tmp2E4;unsigned int _tmp2E7;int _tmp2E6;_LL11: _tmp2E7=_tmp2E5.f1;_tmp2E6=_tmp2E5.f2;_LL12:;
if(_tmp2E6)
# 1638
({unsigned int _tmp71F=loc;unsigned int _tmp71E=_tmp2E7;void*_tmp71D=_tmp2F0;Cyc_Tcutil_check_bound(_tmp71F,_tmp71E,_tmp71D,Cyc_Tcenv_abstract_val_ok(_tmp2CD));});}else{
# 1641
if(Cyc_Tcutil_is_bound_one(_tmp2F0) && !Cyc_Tcutil_force_type2bool(0,_tmp2EF))
({void*_tmp2E8=0U;({unsigned int _tmp721=e1->loc;struct _dyneither_ptr _tmp720=({const char*_tmp2E9="subscript applied to pointer to one object";_tag_dyneither(_tmp2E9,sizeof(char),43U);});Cyc_Tcutil_warn(_tmp721,_tmp720,_tag_dyneither(_tmp2E8,sizeof(void*),0U));});});
Cyc_Tcutil_check_nonzero_bound(loc,_tmp2F0);}}
# 1646
Cyc_Tcenv_check_rgn_accessible(_tmp2CD,loc,_tmp2F1);
if(!Cyc_Tcutil_kind_leq(Cyc_Tcutil_type_kind(_tmp2F3),& Cyc_Tcutil_tmk) && !Cyc_Tcenv_abstract_val_ok(_tmp2CD))
({void*_tmp2EA=0U;({unsigned int _tmp723=e1->loc;struct _dyneither_ptr _tmp722=({const char*_tmp2EB="can't subscript an abstract pointer";_tag_dyneither(_tmp2EB,sizeof(char),36U);});Cyc_Tcutil_terr(_tmp723,_tmp722,_tag_dyneither(_tmp2EA,sizeof(void*),0U));});});
return _tmp2F3;case 6U: _LL3: _tmp2F4=((struct Cyc_Absyn_TupleType_Absyn_Type_struct*)_tmp2D1)->f1;_LL4:
 return Cyc_Tcexp_ithTupleType(_tmp2CD,loc,_tmp2F4,e2);default: _LL5: _LL6:
 return({struct Cyc_String_pa_PrintArg_struct _tmp2EE=({struct Cyc_String_pa_PrintArg_struct _tmp534;_tmp534.tag=0U,({struct _dyneither_ptr _tmp724=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t1));_tmp534.f1=_tmp724;});_tmp534;});void*_tmp2EC[1U];_tmp2EC[0]=& _tmp2EE;({struct Cyc_Tcenv_Tenv*_tmp728=_tmp2CD;unsigned int _tmp727=loc;void**_tmp726=topt;struct _dyneither_ptr _tmp725=({const char*_tmp2ED="subscript applied to %s";_tag_dyneither(_tmp2ED,sizeof(char),24U);});Cyc_Tcexp_expr_err(_tmp728,_tmp727,_tmp726,_tmp725,_tag_dyneither(_tmp2EC,sizeof(void*),1U));});});}_LL0:;};};}
# 1656
static void*Cyc_Tcexp_tcTuple(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_List_List*es){
int done=0;
struct Cyc_List_List*fields=0;
if(topt != 0){
void*_tmp2F5=Cyc_Tcutil_compress(*topt);void*_tmp2F6=_tmp2F5;struct Cyc_List_List*_tmp2FA;if(((struct Cyc_Absyn_TupleType_Absyn_Type_struct*)_tmp2F6)->tag == 6U){_LL1: _tmp2FA=((struct Cyc_Absyn_TupleType_Absyn_Type_struct*)_tmp2F6)->f1;_LL2:
# 1662
 if(({int _tmp729=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmp2FA);_tmp729 != ((int(*)(struct Cyc_List_List*x))Cyc_List_length)(es);}))
# 1665
goto _LL0;
# 1667
for(0;es != 0;(es=es->tl,_tmp2FA=_tmp2FA->tl)){
int bogus=0;
void*_tmp2F7=(*((struct _tuple17*)((struct Cyc_List_List*)_check_null(_tmp2FA))->hd)).f2;
({struct Cyc_Tcenv_Tenv*_tmp72A=Cyc_Tcenv_clear_abstract_val_ok(te);Cyc_Tcexp_tcExpInitializer(_tmp72A,& _tmp2F7,(struct Cyc_Absyn_Exp*)es->hd);});
# 1672
Cyc_Tcutil_coerce_arg(te,(struct Cyc_Absyn_Exp*)es->hd,(*((struct _tuple17*)_tmp2FA->hd)).f2,& bogus);
fields=({struct Cyc_List_List*_tmp2F9=_cycalloc(sizeof(*_tmp2F9));({struct _tuple17*_tmp72B=({struct _tuple17*_tmp2F8=_cycalloc(sizeof(*_tmp2F8));_tmp2F8->f1=(*((struct _tuple17*)_tmp2FA->hd)).f1,_tmp2F8->f2=(void*)_check_null(((struct Cyc_Absyn_Exp*)es->hd)->topt);_tmp2F8;});_tmp2F9->hd=_tmp72B;}),_tmp2F9->tl=fields;_tmp2F9;});}
# 1675
done=1;
goto _LL0;}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 1679
if(!done)
for(0;es != 0;es=es->tl){
({struct Cyc_Tcenv_Tenv*_tmp72C=Cyc_Tcenv_clear_abstract_val_ok(te);Cyc_Tcexp_tcExpInitializer(_tmp72C,0,(struct Cyc_Absyn_Exp*)es->hd);});
fields=({struct Cyc_List_List*_tmp2FC=_cycalloc(sizeof(*_tmp2FC));({struct _tuple17*_tmp72E=({struct _tuple17*_tmp2FB=_cycalloc(sizeof(*_tmp2FB));({struct Cyc_Absyn_Tqual _tmp72D=Cyc_Absyn_empty_tqual(0U);_tmp2FB->f1=_tmp72D;}),_tmp2FB->f2=(void*)_check_null(((struct Cyc_Absyn_Exp*)es->hd)->topt);_tmp2FB;});_tmp2FC->hd=_tmp72E;}),_tmp2FC->tl=fields;_tmp2FC;});}
# 1684
return(void*)({struct Cyc_Absyn_TupleType_Absyn_Type_struct*_tmp2FD=_cycalloc(sizeof(*_tmp2FD));_tmp2FD->tag=6U,({struct Cyc_List_List*_tmp72F=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(fields);_tmp2FD->f1=_tmp72F;});_tmp2FD;});}
# 1688
static void*Cyc_Tcexp_tcCompoundLit(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct _tuple9*t,struct Cyc_List_List*des){
# 1691
return({void*_tmp2FE=0U;({struct Cyc_Tcenv_Tenv*_tmp733=te;unsigned int _tmp732=loc;void**_tmp731=topt;struct _dyneither_ptr _tmp730=({const char*_tmp2FF="tcCompoundLit";_tag_dyneither(_tmp2FF,sizeof(char),14U);});Cyc_Tcexp_expr_err(_tmp733,_tmp732,_tmp731,_tmp730,_tag_dyneither(_tmp2FE,sizeof(void*),0U));});});}struct _tuple18{struct Cyc_List_List*f1;struct Cyc_Absyn_Exp*f2;};
# 1701 "tcexp.cyc"
static void*Cyc_Tcexp_tcArray(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**elt_topt,struct Cyc_Absyn_Tqual*elt_tqopt,int zero_term,struct Cyc_List_List*des){
# 1704
void*res_t2;
int _tmp300=((int(*)(struct Cyc_List_List*x))Cyc_List_length)(des);
struct Cyc_List_List*es=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(struct _tuple18*),struct Cyc_List_List*x))Cyc_List_map)((struct Cyc_Absyn_Exp*(*)(struct _tuple18*))Cyc_Core_snd,des);
void*res=Cyc_Absyn_new_evar(& Cyc_Tcutil_tmko,Cyc_Tcenv_lookup_opt_type_vars(te));
struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp301=({struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*_tmp317=_cycalloc(sizeof(*_tmp317));_tmp317->tag=0U,({union Cyc_Absyn_Cnst _tmp734=Cyc_Absyn_Int_c(Cyc_Absyn_Unsigned,_tmp300);_tmp317->f1=_tmp734;});_tmp317;});
struct Cyc_Absyn_Exp*sz_exp=Cyc_Absyn_new_exp((void*)_tmp301,loc);
# 1712
if(zero_term){
struct Cyc_Absyn_Exp*_tmp302=((struct Cyc_Absyn_Exp*(*)(struct Cyc_List_List*x,int n))Cyc_List_nth)(es,_tmp300 - 1);
if(!Cyc_Tcutil_is_zero(_tmp302))
({void*_tmp303=0U;({unsigned int _tmp736=_tmp302->loc;struct _dyneither_ptr _tmp735=({const char*_tmp304="zero-terminated array doesn't end with zero.";_tag_dyneither(_tmp304,sizeof(char),45U);});Cyc_Tcutil_terr(_tmp736,_tmp735,_tag_dyneither(_tmp303,sizeof(void*),0U));});});}
# 1717
sz_exp->topt=Cyc_Absyn_uint_type;
res_t2=({void*_tmp739=res;struct Cyc_Absyn_Tqual _tmp738=
(unsigned int)elt_tqopt?*elt_tqopt: Cyc_Absyn_empty_tqual(0U);
# 1718
struct Cyc_Absyn_Exp*_tmp737=sz_exp;Cyc_Absyn_array_type(_tmp739,_tmp738,_tmp737,
# 1720
zero_term?Cyc_Absyn_true_type: Cyc_Absyn_false_type,0U);});
# 1722
{struct Cyc_List_List*es2=es;for(0;es2 != 0;es2=es2->tl){
Cyc_Tcexp_tcExpInitializer(te,elt_topt,(struct Cyc_Absyn_Exp*)es2->hd);}}
# 1725
if(!Cyc_Tcutil_coerce_list(te,res,es))
# 1727
({struct Cyc_String_pa_PrintArg_struct _tmp307=({struct Cyc_String_pa_PrintArg_struct _tmp535;_tmp535.tag=0U,({
struct _dyneither_ptr _tmp73A=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(res));_tmp535.f1=_tmp73A;});_tmp535;});void*_tmp305[1U];_tmp305[0]=& _tmp307;({unsigned int _tmp73C=((struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(es))->hd)->loc;struct _dyneither_ptr _tmp73B=({const char*_tmp306="elements of array do not all have the same type (%s)";_tag_dyneither(_tmp306,sizeof(char),53U);});Cyc_Tcutil_terr(_tmp73C,_tmp73B,_tag_dyneither(_tmp305,sizeof(void*),1U));});});
# 1730
{int offset=0;for(0;des != 0;(offset ++,des=des->tl)){
struct Cyc_List_List*ds=(*((struct _tuple18*)des->hd)).f1;
if(ds != 0){
# 1735
void*_tmp308=(void*)ds->hd;void*_tmp309=_tmp308;struct Cyc_Absyn_Exp*_tmp316;if(((struct Cyc_Absyn_FieldName_Absyn_Designator_struct*)_tmp309)->tag == 1U){_LL1: _LL2:
# 1737
({void*_tmp30A=0U;({unsigned int _tmp73E=loc;struct _dyneither_ptr _tmp73D=({const char*_tmp30B="only array index designators are supported";_tag_dyneither(_tmp30B,sizeof(char),43U);});Cyc_Tcutil_terr(_tmp73E,_tmp73D,_tag_dyneither(_tmp30A,sizeof(void*),0U));});});
goto _LL0;}else{_LL3: _tmp316=((struct Cyc_Absyn_ArrayElement_Absyn_Designator_struct*)_tmp309)->f1;_LL4:
# 1740
 Cyc_Tcexp_tcExpInitializer(te,0,_tmp316);{
struct _tuple15 _tmp30C=Cyc_Evexp_eval_const_uint_exp(_tmp316);struct _tuple15 _tmp30D=_tmp30C;unsigned int _tmp315;int _tmp314;_LL6: _tmp315=_tmp30D.f1;_tmp314=_tmp30D.f2;_LL7:;
if(!_tmp314)
({void*_tmp30E=0U;({unsigned int _tmp740=_tmp316->loc;struct _dyneither_ptr _tmp73F=({const char*_tmp30F="index designator cannot use sizeof or offsetof";_tag_dyneither(_tmp30F,sizeof(char),47U);});Cyc_Tcutil_terr(_tmp740,_tmp73F,_tag_dyneither(_tmp30E,sizeof(void*),0U));});});else{
if(_tmp315 != offset)
({struct Cyc_Int_pa_PrintArg_struct _tmp312=({struct Cyc_Int_pa_PrintArg_struct _tmp537;_tmp537.tag=1U,_tmp537.f1=(unsigned long)offset;_tmp537;});struct Cyc_Int_pa_PrintArg_struct _tmp313=({struct Cyc_Int_pa_PrintArg_struct _tmp536;_tmp536.tag=1U,_tmp536.f1=(unsigned long)((int)_tmp315);_tmp536;});void*_tmp310[2U];_tmp310[0]=& _tmp312,_tmp310[1]=& _tmp313;({unsigned int _tmp742=_tmp316->loc;struct _dyneither_ptr _tmp741=({const char*_tmp311="expecting index designator %d but found %d";_tag_dyneither(_tmp311,sizeof(char),43U);});Cyc_Tcutil_terr(_tmp742,_tmp741,_tag_dyneither(_tmp310,sizeof(void*),2U));});});}
# 1747
goto _LL0;};}_LL0:;}}}
# 1751
return res_t2;}
# 1755
static void*Cyc_Tcexp_tcComprehension(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Vardecl*vd,struct Cyc_Absyn_Exp*bound,struct Cyc_Absyn_Exp*body,int*is_zero_term){
# 1758
Cyc_Tcexp_tcExp(te,0,bound);
{void*_tmp318=Cyc_Tcutil_compress((void*)_check_null(bound->topt));void*_tmp319=_tmp318;void*_tmp31E;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp319)->tag == 0U){if(((struct Cyc_Absyn_TagCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp319)->f1)->tag == 4U){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp319)->f2 != 0){_LL1: _tmp31E=(void*)(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp319)->f2)->hd;_LL2:
# 1763
 if(Cyc_Tcenv_new_status(te)== Cyc_Tcenv_InNewAggr){
struct Cyc_Absyn_Exp*_tmp31A=({void*_tmp743=Cyc_Absyn_uint_type;Cyc_Absyn_cast_exp(_tmp743,Cyc_Absyn_valueof_exp(_tmp31E,0U),0,Cyc_Absyn_No_coercion,0U);});
_tmp31A->topt=bound->topt;
# 1768
bound=_tmp31A;}
# 1770
goto _LL0;}else{goto _LL3;}}else{goto _LL3;}}else{_LL3: _LL4:
# 1772
 if(!Cyc_Tcutil_coerce_uint_type(te,bound))
({struct Cyc_String_pa_PrintArg_struct _tmp31D=({struct Cyc_String_pa_PrintArg_struct _tmp538;_tmp538.tag=0U,({
struct _dyneither_ptr _tmp744=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(bound->topt)));_tmp538.f1=_tmp744;});_tmp538;});void*_tmp31B[1U];_tmp31B[0]=& _tmp31D;({unsigned int _tmp746=bound->loc;struct _dyneither_ptr _tmp745=({const char*_tmp31C="expecting unsigned int, found %s";_tag_dyneither(_tmp31C,sizeof(char),33U);});Cyc_Tcutil_terr(_tmp746,_tmp745,_tag_dyneither(_tmp31B,sizeof(void*),1U));});});}_LL0:;}
# 1777
if(!(vd->tq).real_const)
({void*_tmp31F=0U;({struct _dyneither_ptr _tmp747=({const char*_tmp320="comprehension index variable is not declared const!";_tag_dyneither(_tmp320,sizeof(char),52U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp747,_tag_dyneither(_tmp31F,sizeof(void*),0U));});});
# 1780
if(te->le != 0)
te=Cyc_Tcenv_new_block(loc,te);{
void**_tmp321=0;
struct Cyc_Absyn_Tqual*_tmp322=0;
void**_tmp323=0;
# 1786
if(topt != 0){
void*_tmp324=Cyc_Tcutil_compress(*topt);void*_tmp325=_tmp324;void*_tmp330;struct Cyc_Absyn_Tqual _tmp32F;struct Cyc_Absyn_Exp*_tmp32E;void*_tmp32D;struct Cyc_Absyn_PtrInfo _tmp32C;switch(*((int*)_tmp325)){case 3U: _LL6: _tmp32C=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp325)->f1;_LL7:
# 1789
 _tmp321=({void**_tmp326=_cycalloc(sizeof(*_tmp326));*_tmp326=_tmp32C.elt_type;_tmp326;});
_tmp322=({struct Cyc_Absyn_Tqual*_tmp327=_cycalloc(sizeof(*_tmp327));*_tmp327=_tmp32C.elt_tq;_tmp327;});
_tmp323=({void**_tmp328=_cycalloc(sizeof(*_tmp328));*_tmp328=(_tmp32C.ptr_atts).zero_term;_tmp328;});
goto _LL5;case 4U: _LL8: _tmp330=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp325)->f1).elt_type;_tmp32F=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp325)->f1).tq;_tmp32E=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp325)->f1).num_elts;_tmp32D=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp325)->f1).zero_term;_LL9:
# 1794
 _tmp321=({void**_tmp329=_cycalloc(sizeof(*_tmp329));*_tmp329=_tmp330;_tmp329;});
_tmp322=({struct Cyc_Absyn_Tqual*_tmp32A=_cycalloc(sizeof(*_tmp32A));*_tmp32A=_tmp32F;_tmp32A;});
_tmp323=({void**_tmp32B=_cycalloc(sizeof(*_tmp32B));*_tmp32B=_tmp32D;_tmp32B;});
goto _LL5;default: _LLA: _LLB:
# 1799
 goto _LL5;}_LL5:;}{
# 1802
void*t=Cyc_Tcexp_tcExp(te,_tmp321,body);
# 1804
if(Cyc_Tcutil_is_noalias_pointer_or_aggr(t) && !Cyc_Tcutil_is_noalias_path(body))
({void*_tmp331=0U;({unsigned int _tmp749=body->loc;struct _dyneither_ptr _tmp748=({const char*_tmp332="Cannot consume non-unique paths; do swap instead";_tag_dyneither(_tmp332,sizeof(char),49U);});Cyc_Tcutil_terr(_tmp749,_tmp748,_tag_dyneither(_tmp331,sizeof(void*),0U));});});
if(te->le == 0){
# 1808
if(!Cyc_Tcutil_is_const_exp(bound))
({void*_tmp333=0U;({unsigned int _tmp74B=bound->loc;struct _dyneither_ptr _tmp74A=({const char*_tmp334="bound is not constant";_tag_dyneither(_tmp334,sizeof(char),22U);});Cyc_Tcutil_terr(_tmp74B,_tmp74A,_tag_dyneither(_tmp333,sizeof(void*),0U));});});
if(!Cyc_Tcutil_is_const_exp(body))
({void*_tmp335=0U;({unsigned int _tmp74D=bound->loc;struct _dyneither_ptr _tmp74C=({const char*_tmp336="body is not constant";_tag_dyneither(_tmp336,sizeof(char),21U);});Cyc_Tcutil_terr(_tmp74D,_tmp74C,_tag_dyneither(_tmp335,sizeof(void*),0U));});});}
# 1813
if(_tmp323 != 0  && Cyc_Tcutil_force_type2bool(0,*_tmp323)){
# 1816
struct Cyc_Absyn_Exp*_tmp337=Cyc_Absyn_uint_exp(1U,0U);_tmp337->topt=Cyc_Absyn_uint_type;
bound=Cyc_Absyn_add_exp(bound,_tmp337,0U);bound->topt=Cyc_Absyn_uint_type;
*is_zero_term=1;}
# 1820
if(Cyc_Tcutil_is_noalias_pointer_or_aggr((void*)_check_null(body->topt)) && !
Cyc_Tcutil_is_noalias_path(body))
({void*_tmp338=0U;({unsigned int _tmp74F=body->loc;struct _dyneither_ptr _tmp74E=({const char*_tmp339="Cannot consume non-unique paths; do swap instead";_tag_dyneither(_tmp339,sizeof(char),49U);});Cyc_Tcutil_terr(_tmp74F,_tmp74E,_tag_dyneither(_tmp338,sizeof(void*),0U));});});{
# 1824
void*_tmp33A=({void*_tmp752=t;struct Cyc_Absyn_Tqual _tmp751=_tmp322 == 0?Cyc_Absyn_empty_tqual(0U):*_tmp322;struct Cyc_Absyn_Exp*_tmp750=bound;Cyc_Absyn_array_type(_tmp752,_tmp751,_tmp750,
_tmp323 == 0?Cyc_Absyn_false_type:*_tmp323,0U);});
return _tmp33A;};};};}
# 1830
static void*Cyc_Tcexp_tcComprehensionNoinit(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*bound,void*t,int*is_zero_term){
# 1833
Cyc_Tcexp_tcExp(te,0,bound);
{void*_tmp33B=Cyc_Tcutil_compress((void*)_check_null(bound->topt));void*_tmp33C=_tmp33B;void*_tmp341;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp33C)->tag == 0U){if(((struct Cyc_Absyn_TagCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp33C)->f1)->tag == 4U){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp33C)->f2 != 0){_LL1: _tmp341=(void*)(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp33C)->f2)->hd;_LL2:
# 1838
 if(Cyc_Tcenv_new_status(te)== Cyc_Tcenv_InNewAggr){
struct Cyc_Absyn_Exp*_tmp33D=({void*_tmp753=Cyc_Absyn_uint_type;Cyc_Absyn_cast_exp(_tmp753,Cyc_Absyn_valueof_exp(_tmp341,0U),0,Cyc_Absyn_No_coercion,0U);});
_tmp33D->topt=bound->topt;
# 1843
bound=_tmp33D;}
# 1845
goto _LL0;}else{goto _LL3;}}else{goto _LL3;}}else{_LL3: _LL4:
# 1847
 if(!Cyc_Tcutil_coerce_uint_type(te,bound))
({struct Cyc_String_pa_PrintArg_struct _tmp340=({struct Cyc_String_pa_PrintArg_struct _tmp539;_tmp539.tag=0U,({
struct _dyneither_ptr _tmp754=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(bound->topt)));_tmp539.f1=_tmp754;});_tmp539;});void*_tmp33E[1U];_tmp33E[0]=& _tmp340;({unsigned int _tmp756=bound->loc;struct _dyneither_ptr _tmp755=({const char*_tmp33F="expecting unsigned int, found %s";_tag_dyneither(_tmp33F,sizeof(char),33U);});Cyc_Tcutil_terr(_tmp756,_tmp755,_tag_dyneither(_tmp33E,sizeof(void*),1U));});});}_LL0:;}{
# 1852
void**_tmp342=0;
struct Cyc_Absyn_Tqual*_tmp343=0;
void**_tmp344=0;
# 1856
if(topt != 0){
void*_tmp345=Cyc_Tcutil_compress(*topt);void*_tmp346=_tmp345;void*_tmp351;struct Cyc_Absyn_Tqual _tmp350;struct Cyc_Absyn_Exp*_tmp34F;void*_tmp34E;struct Cyc_Absyn_PtrInfo _tmp34D;switch(*((int*)_tmp346)){case 3U: _LL6: _tmp34D=((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp346)->f1;_LL7:
# 1859
 _tmp342=({void**_tmp347=_cycalloc(sizeof(*_tmp347));*_tmp347=_tmp34D.elt_type;_tmp347;});
_tmp343=({struct Cyc_Absyn_Tqual*_tmp348=_cycalloc(sizeof(*_tmp348));*_tmp348=_tmp34D.elt_tq;_tmp348;});
_tmp344=({void**_tmp349=_cycalloc(sizeof(*_tmp349));*_tmp349=(_tmp34D.ptr_atts).zero_term;_tmp349;});
goto _LL5;case 4U: _LL8: _tmp351=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp346)->f1).elt_type;_tmp350=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp346)->f1).tq;_tmp34F=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp346)->f1).num_elts;_tmp34E=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp346)->f1).zero_term;_LL9:
# 1864
 _tmp342=({void**_tmp34A=_cycalloc(sizeof(*_tmp34A));*_tmp34A=_tmp351;_tmp34A;});
_tmp343=({struct Cyc_Absyn_Tqual*_tmp34B=_cycalloc(sizeof(*_tmp34B));*_tmp34B=_tmp350;_tmp34B;});
_tmp344=({void**_tmp34C=_cycalloc(sizeof(*_tmp34C));*_tmp34C=_tmp34E;_tmp34C;});
goto _LL5;default: _LLA: _LLB:
# 1869
 goto _LL5;}_LL5:;}
# 1872
({unsigned int _tmp759=loc;struct Cyc_Tcenv_Tenv*_tmp758=te;struct Cyc_List_List*_tmp757=Cyc_Tcenv_lookup_type_vars(te);Cyc_Tcutil_check_type(_tmp759,_tmp758,_tmp757,& Cyc_Tcutil_tmk,1,1,t);});
if(_tmp342 != 0)Cyc_Tcutil_unify(*_tmp342,t);
# 1875
if(te->le == 0){
if(!Cyc_Tcutil_is_const_exp(bound))
({void*_tmp352=0U;({unsigned int _tmp75B=bound->loc;struct _dyneither_ptr _tmp75A=({const char*_tmp353="bound is not constant";_tag_dyneither(_tmp353,sizeof(char),22U);});Cyc_Tcutil_terr(_tmp75B,_tmp75A,_tag_dyneither(_tmp352,sizeof(void*),0U));});});}
# 1879
if(_tmp344 != 0  && Cyc_Tcutil_force_type2bool(0,*_tmp344)){
# 1882
struct Cyc_Absyn_Exp*_tmp354=Cyc_Absyn_uint_exp(1U,0U);_tmp354->topt=Cyc_Absyn_uint_type;
bound=Cyc_Absyn_add_exp(bound,_tmp354,0U);bound->topt=Cyc_Absyn_uint_type;
*is_zero_term=1;
# 1886
({void*_tmp355=0U;({unsigned int _tmp75D=loc;struct _dyneither_ptr _tmp75C=({const char*_tmp356="non-initializing comprehensions do not currently support @zeroterm arrays";_tag_dyneither(_tmp356,sizeof(char),74U);});Cyc_Tcutil_terr(_tmp75D,_tmp75C,_tag_dyneither(_tmp355,sizeof(void*),0U));});});}{
# 1889
void*_tmp357=({void*_tmp760=t;struct Cyc_Absyn_Tqual _tmp75F=_tmp343 == 0?Cyc_Absyn_empty_tqual(0U):*_tmp343;struct Cyc_Absyn_Exp*_tmp75E=bound;Cyc_Absyn_array_type(_tmp760,_tmp75F,_tmp75E,
_tmp344 == 0?Cyc_Absyn_false_type:*_tmp344,0U);});
return _tmp357;};};}struct _tuple19{struct Cyc_Absyn_Aggrfield*f1;struct Cyc_Absyn_Exp*f2;};
# 1904 "tcexp.cyc"
static void*Cyc_Tcexp_tcAggregate(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct _tuple1**tn,struct Cyc_List_List**ts,struct Cyc_List_List*args,struct Cyc_Absyn_Aggrdecl**ad_opt){
# 1908
struct Cyc_Absyn_Aggrdecl**adptr;
struct Cyc_Absyn_Aggrdecl*ad;
if(*ad_opt != 0){
ad=(struct Cyc_Absyn_Aggrdecl*)_check_null(*ad_opt);
adptr=({struct Cyc_Absyn_Aggrdecl**_tmp358=_cycalloc(sizeof(*_tmp358));*_tmp358=ad;_tmp358;});}else{
# 1914
{struct _handler_cons _tmp359;_push_handler(& _tmp359);{int _tmp35B=0;if(setjmp(_tmp359.handler))_tmp35B=1;if(!_tmp35B){adptr=Cyc_Tcenv_lookup_aggrdecl(te,loc,*tn);
ad=*adptr;
# 1914
;_pop_handler();}else{void*_tmp35A=(void*)_exn_thrown;void*_tmp35C=_tmp35A;void*_tmp360;if(((struct Cyc_Dict_Absent_exn_struct*)_tmp35C)->tag == Cyc_Dict_Absent){_LL1: _LL2:
# 1917
({struct Cyc_String_pa_PrintArg_struct _tmp35F=({struct Cyc_String_pa_PrintArg_struct _tmp53A;_tmp53A.tag=0U,({struct _dyneither_ptr _tmp761=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(*tn));_tmp53A.f1=_tmp761;});_tmp53A;});void*_tmp35D[1U];_tmp35D[0]=& _tmp35F;({unsigned int _tmp763=loc;struct _dyneither_ptr _tmp762=({const char*_tmp35E="unbound struct/union name %s";_tag_dyneither(_tmp35E,sizeof(char),29U);});Cyc_Tcutil_terr(_tmp763,_tmp762,_tag_dyneither(_tmp35D,sizeof(void*),1U));});});
return topt != 0?*topt: Cyc_Absyn_void_type;}else{_LL3: _tmp360=_tmp35C;_LL4:(int)_rethrow(_tmp360);}_LL0:;}};}
# 1920
*ad_opt=ad;
*tn=ad->name;}
# 1923
if(ad->impl == 0){
({struct Cyc_String_pa_PrintArg_struct _tmp363=({struct Cyc_String_pa_PrintArg_struct _tmp53B;_tmp53B.tag=0U,({struct _dyneither_ptr _tmp764=(struct _dyneither_ptr)(ad->kind == Cyc_Absyn_StructA?({const char*_tmp364="struct";_tag_dyneither(_tmp364,sizeof(char),7U);}):({const char*_tmp365="union";_tag_dyneither(_tmp365,sizeof(char),6U);}));_tmp53B.f1=_tmp764;});_tmp53B;});void*_tmp361[1U];_tmp361[0]=& _tmp363;({unsigned int _tmp766=loc;struct _dyneither_ptr _tmp765=({const char*_tmp362="can't construct abstract %s";_tag_dyneither(_tmp362,sizeof(char),28U);});Cyc_Tcutil_terr(_tmp766,_tmp765,_tag_dyneither(_tmp361,sizeof(void*),1U));});});
return Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te));}{
# 1931
struct Cyc_Tcenv_Tenv*te2;
enum Cyc_Tcenv_NewStatus _tmp366=Cyc_Tcenv_new_status(te);
if(_tmp366 == Cyc_Tcenv_InNew)
te2=Cyc_Tcenv_set_new_status(Cyc_Tcenv_InNewAggr,te);else{
# 1940
te2=Cyc_Tcenv_set_new_status(_tmp366,te);}{
# 1942
struct _tuple12 _tmp367=({struct _tuple12 _tmp541;({struct Cyc_List_List*_tmp767=Cyc_Tcenv_lookup_type_vars(te2);_tmp541.f1=_tmp767;}),_tmp541.f2=Cyc_Core_heap_region;_tmp541;});
struct Cyc_List_List*_tmp368=((struct Cyc_List_List*(*)(struct _tuple13*(*f)(struct _tuple12*,struct Cyc_Absyn_Tvar*),struct _tuple12*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcutil_r_make_inst_var,& _tmp367,ad->tvs);
struct Cyc_List_List*_tmp369=((struct Cyc_List_List*(*)(struct _tuple13*(*f)(struct _tuple12*,struct Cyc_Absyn_Tvar*),struct _tuple12*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcutil_r_make_inst_var,& _tmp367,((struct Cyc_Absyn_AggrdeclImpl*)_check_null(ad->impl))->exist_vars);
struct Cyc_List_List*_tmp36A=((struct Cyc_List_List*(*)(void*(*f)(struct _tuple13*),struct Cyc_List_List*x))Cyc_List_map)((void*(*)(struct _tuple13*))Cyc_Core_snd,_tmp368);
struct Cyc_List_List*_tmp36B=((struct Cyc_List_List*(*)(void*(*f)(struct _tuple13*),struct Cyc_List_List*x))Cyc_List_map)((void*(*)(struct _tuple13*))Cyc_Core_snd,_tmp369);
struct Cyc_List_List*_tmp36C=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(_tmp368,_tmp369);
void*res_typ;
# 1953
if(topt != 0){
void*_tmp36D=Cyc_Tcutil_compress(*topt);void*_tmp36E=_tmp36D;struct Cyc_Absyn_Aggrdecl**_tmp371;struct Cyc_List_List*_tmp370;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp36E)->tag == 0U){if(((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp36E)->f1)->tag == 20U){if(((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp36E)->f1)->f1).KnownAggr).tag == 2){_LL6: _tmp371=((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp36E)->f1)->f1).KnownAggr).val;_tmp370=((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp36E)->f2;_LL7:
# 1956
 if(*_tmp371 == *adptr){
{struct Cyc_List_List*_tmp36F=_tmp36A;for(0;_tmp36F != 0  && _tmp370 != 0;(
_tmp36F=_tmp36F->tl,_tmp370=_tmp370->tl)){
Cyc_Tcutil_unify((void*)_tmp36F->hd,(void*)_tmp370->hd);}}
# 1961
res_typ=*topt;
goto _LL5;}
# 1964
goto _LL9;}else{goto _LL8;}}else{goto _LL8;}}else{_LL8: _LL9:
# 1966
 res_typ=({union Cyc_Absyn_AggrInfo _tmp768=Cyc_Absyn_KnownAggr(adptr);Cyc_Absyn_aggr_type(_tmp768,_tmp36A);});}_LL5:;}else{
# 1969
res_typ=({union Cyc_Absyn_AggrInfo _tmp769=Cyc_Absyn_KnownAggr(adptr);Cyc_Absyn_aggr_type(_tmp769,_tmp36A);});}{
# 1972
struct Cyc_List_List*_tmp372=*ts;
struct Cyc_List_List*_tmp373=_tmp36B;
while(_tmp372 != 0  && _tmp373 != 0){
# 1976
({unsigned int _tmp76C=loc;struct Cyc_Tcenv_Tenv*_tmp76B=te2;struct Cyc_List_List*_tmp76A=Cyc_Tcenv_lookup_type_vars(te2);Cyc_Tcutil_check_type(_tmp76C,_tmp76B,_tmp76A,& Cyc_Tcutil_ak,1,0,(void*)_tmp372->hd);});
Cyc_Tcutil_check_no_qual(loc,(void*)_tmp372->hd);
Cyc_Tcutil_unify((void*)_tmp372->hd,(void*)_tmp373->hd);
_tmp372=_tmp372->tl;
_tmp373=_tmp373->tl;}
# 1982
if(_tmp372 != 0)
({void*_tmp374=0U;({unsigned int _tmp76E=loc;struct _dyneither_ptr _tmp76D=({const char*_tmp375="too many explicit witness types";_tag_dyneither(_tmp375,sizeof(char),32U);});Cyc_Tcutil_terr(_tmp76E,_tmp76D,_tag_dyneither(_tmp374,sizeof(void*),0U));});});
# 1985
*ts=_tmp36B;{
# 1988
struct Cyc_List_List*fields=
((struct Cyc_List_List*(*)(struct _RegionHandle*rgn,unsigned int loc,struct Cyc_List_List*des,enum Cyc_Absyn_AggrKind,struct Cyc_List_List*fields))Cyc_Tcutil_resolve_aggregate_designators)(Cyc_Core_heap_region,loc,args,ad->kind,((struct Cyc_Absyn_AggrdeclImpl*)_check_null(ad->impl))->fields);
for(0;fields != 0;fields=fields->tl){
int bogus=0;
struct _tuple19*_tmp376=(struct _tuple19*)fields->hd;struct _tuple19*_tmp377=_tmp376;struct Cyc_Absyn_Aggrfield*_tmp383;struct Cyc_Absyn_Exp*_tmp382;_LLB: _tmp383=_tmp377->f1;_tmp382=_tmp377->f2;_LLC:;{
void*_tmp378=Cyc_Tcutil_substitute(_tmp36C,_tmp383->type);
({struct Cyc_Tcenv_Tenv*_tmp76F=Cyc_Tcenv_clear_abstract_val_ok(te2);Cyc_Tcexp_tcExpInitializer(_tmp76F,& _tmp378,_tmp382);});
# 1999
if(!Cyc_Tcutil_coerce_arg(te2,_tmp382,_tmp378,& bogus)){
({struct Cyc_String_pa_PrintArg_struct _tmp37B=({struct Cyc_String_pa_PrintArg_struct _tmp540;_tmp540.tag=0U,_tmp540.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp383->name);_tmp540;});struct Cyc_String_pa_PrintArg_struct _tmp37C=({struct Cyc_String_pa_PrintArg_struct _tmp53F;_tmp53F.tag=0U,({
struct _dyneither_ptr _tmp770=(struct _dyneither_ptr)(ad->kind == Cyc_Absyn_StructA?({const char*_tmp380="struct";_tag_dyneither(_tmp380,sizeof(char),7U);}):({const char*_tmp381="union";_tag_dyneither(_tmp381,sizeof(char),6U);}));_tmp53F.f1=_tmp770;});_tmp53F;});struct Cyc_String_pa_PrintArg_struct _tmp37D=({struct Cyc_String_pa_PrintArg_struct _tmp53E;_tmp53E.tag=0U,({
struct _dyneither_ptr _tmp771=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(*tn));_tmp53E.f1=_tmp771;});_tmp53E;});struct Cyc_String_pa_PrintArg_struct _tmp37E=({struct Cyc_String_pa_PrintArg_struct _tmp53D;_tmp53D.tag=0U,({struct _dyneither_ptr _tmp772=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp378));_tmp53D.f1=_tmp772;});_tmp53D;});struct Cyc_String_pa_PrintArg_struct _tmp37F=({struct Cyc_String_pa_PrintArg_struct _tmp53C;_tmp53C.tag=0U,({
struct _dyneither_ptr _tmp773=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(_tmp382->topt)));_tmp53C.f1=_tmp773;});_tmp53C;});void*_tmp379[5U];_tmp379[0]=& _tmp37B,_tmp379[1]=& _tmp37C,_tmp379[2]=& _tmp37D,_tmp379[3]=& _tmp37E,_tmp379[4]=& _tmp37F;({unsigned int _tmp775=_tmp382->loc;struct _dyneither_ptr _tmp774=({const char*_tmp37A="field %s of %s %s expects type %s != %s";_tag_dyneither(_tmp37A,sizeof(char),40U);});Cyc_Tcutil_terr(_tmp775,_tmp774,_tag_dyneither(_tmp379,sizeof(void*),5U));});});
Cyc_Tcutil_explain_failure();}};}{
# 2008
struct Cyc_List_List*_tmp384=0;
{struct Cyc_List_List*_tmp385=((struct Cyc_Absyn_AggrdeclImpl*)_check_null(ad->impl))->rgn_po;for(0;_tmp385 != 0;_tmp385=_tmp385->tl){
_tmp384=({struct Cyc_List_List*_tmp387=_cycalloc(sizeof(*_tmp387));({struct _tuple0*_tmp778=({struct _tuple0*_tmp386=_cycalloc(sizeof(*_tmp386));({void*_tmp777=Cyc_Tcutil_substitute(_tmp36C,(*((struct _tuple0*)_tmp385->hd)).f1);_tmp386->f1=_tmp777;}),({
void*_tmp776=Cyc_Tcutil_substitute(_tmp36C,(*((struct _tuple0*)_tmp385->hd)).f2);_tmp386->f2=_tmp776;});_tmp386;});
# 2010
_tmp387->hd=_tmp778;}),_tmp387->tl=_tmp384;_tmp387;});}}
# 2013
_tmp384=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(_tmp384);
Cyc_Tcenv_check_rgn_partial_order(te2,loc,_tmp384);
return res_typ;};};};};};}
# 2019
static void*Cyc_Tcexp_tcAnonStruct(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void*ts,struct Cyc_List_List*args){
# 2021
{void*_tmp388=Cyc_Tcutil_compress(ts);void*_tmp389=_tmp388;enum Cyc_Absyn_AggrKind _tmp398;struct Cyc_List_List*_tmp397;if(((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp389)->tag == 7U){_LL1: _tmp398=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp389)->f1;_tmp397=((struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_tmp389)->f2;_LL2:
# 2023
 if(_tmp398 == Cyc_Absyn_UnionA)
({void*_tmp38A=0U;({unsigned int _tmp77A=loc;struct _dyneither_ptr _tmp779=({const char*_tmp38B="expecting struct but found union";_tag_dyneither(_tmp38B,sizeof(char),33U);});Cyc_Tcutil_terr(_tmp77A,_tmp779,_tag_dyneither(_tmp38A,sizeof(void*),0U));});});{
struct Cyc_List_List*fields=
((struct Cyc_List_List*(*)(struct _RegionHandle*rgn,unsigned int loc,struct Cyc_List_List*des,enum Cyc_Absyn_AggrKind,struct Cyc_List_List*fields))Cyc_Tcutil_resolve_aggregate_designators)(Cyc_Core_heap_region,loc,args,Cyc_Absyn_StructA,_tmp397);
for(0;fields != 0;fields=fields->tl){
int bogus=0;
struct _tuple19*_tmp38C=(struct _tuple19*)fields->hd;struct _tuple19*_tmp38D=_tmp38C;struct Cyc_Absyn_Aggrfield*_tmp394;struct Cyc_Absyn_Exp*_tmp393;_LL6: _tmp394=_tmp38D->f1;_tmp393=_tmp38D->f2;_LL7:;
({struct Cyc_Tcenv_Tenv*_tmp77C=Cyc_Tcenv_clear_abstract_val_ok(te);void**_tmp77B=& _tmp394->type;Cyc_Tcexp_tcExpInitializer(_tmp77C,_tmp77B,_tmp393);});
# 2032
if(!Cyc_Tcutil_coerce_arg(te,_tmp393,_tmp394->type,& bogus)){
({struct Cyc_String_pa_PrintArg_struct _tmp390=({struct Cyc_String_pa_PrintArg_struct _tmp544;_tmp544.tag=0U,_tmp544.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmp394->name);_tmp544;});struct Cyc_String_pa_PrintArg_struct _tmp391=({struct Cyc_String_pa_PrintArg_struct _tmp543;_tmp543.tag=0U,({
struct _dyneither_ptr _tmp77D=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp394->type));_tmp543.f1=_tmp77D;});_tmp543;});struct Cyc_String_pa_PrintArg_struct _tmp392=({struct Cyc_String_pa_PrintArg_struct _tmp542;_tmp542.tag=0U,({
struct _dyneither_ptr _tmp77E=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)_check_null(_tmp393->topt)));_tmp542.f1=_tmp77E;});_tmp542;});void*_tmp38E[3U];_tmp38E[0]=& _tmp390,_tmp38E[1]=& _tmp391,_tmp38E[2]=& _tmp392;({unsigned int _tmp780=_tmp393->loc;struct _dyneither_ptr _tmp77F=({const char*_tmp38F="field %s of struct expects type %s != %s";_tag_dyneither(_tmp38F,sizeof(char),41U);});Cyc_Tcutil_terr(_tmp780,_tmp77F,_tag_dyneither(_tmp38E,sizeof(void*),3U));});});
Cyc_Tcutil_explain_failure();}}
# 2039
goto _LL0;};}else{_LL3: _LL4:
({void*_tmp395=0U;({struct _dyneither_ptr _tmp781=({const char*_tmp396="tcAnonStruct: wrong type";_tag_dyneither(_tmp396,sizeof(char),25U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp781,_tag_dyneither(_tmp395,sizeof(void*),0U));});});}_LL0:;}
# 2042
return ts;}
# 2046
static void*Cyc_Tcexp_tcDatatype(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e,struct Cyc_List_List*es,struct Cyc_Absyn_Datatypedecl*tud,struct Cyc_Absyn_Datatypefield*tuf){
# 2049
struct _tuple12 _tmp399=({struct _tuple12 _tmp54A;({struct Cyc_List_List*_tmp782=Cyc_Tcenv_lookup_type_vars(te);_tmp54A.f1=_tmp782;}),_tmp54A.f2=Cyc_Core_heap_region;_tmp54A;});
struct Cyc_List_List*_tmp39A=((struct Cyc_List_List*(*)(struct _tuple13*(*f)(struct _tuple12*,struct Cyc_Absyn_Tvar*),struct _tuple12*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcutil_r_make_inst_var,& _tmp399,tud->tvs);
struct Cyc_List_List*_tmp39B=((struct Cyc_List_List*(*)(void*(*f)(struct _tuple13*),struct Cyc_List_List*x))Cyc_List_map)((void*(*)(struct _tuple13*))Cyc_Core_snd,_tmp39A);
void*res=({union Cyc_Absyn_DatatypeFieldInfo _tmp783=Cyc_Absyn_KnownDatatypefield(tud,tuf);Cyc_Absyn_datatype_field_type(_tmp783,_tmp39B);});
# 2054
if(topt != 0){
void*_tmp39C=Cyc_Tcutil_compress(*topt);void*_tmp39D=_tmp39C;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp39D)->tag == 0U){if(((struct Cyc_Absyn_DatatypeFieldCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp39D)->f1)->tag == 19U){_LL1: _LL2:
 Cyc_Tcutil_unify(*topt,res);goto _LL0;}else{goto _LL3;}}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}{
# 2060
struct Cyc_List_List*ts=tuf->typs;
for(0;es != 0  && ts != 0;(es=es->tl,ts=ts->tl)){
int bogus=0;
struct Cyc_Absyn_Exp*e=(struct Cyc_Absyn_Exp*)es->hd;
void*t=(*((struct _tuple17*)ts->hd)).f2;
if(_tmp39A != 0)t=Cyc_Tcutil_substitute(_tmp39A,t);
Cyc_Tcexp_tcExpInitializer(te,& t,e);
if(!Cyc_Tcutil_coerce_arg(te,e,t,& bogus)){
({struct Cyc_String_pa_PrintArg_struct _tmp3A0=({struct Cyc_String_pa_PrintArg_struct _tmp547;_tmp547.tag=0U,({
# 2070
struct _dyneither_ptr _tmp784=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(tuf->name));_tmp547.f1=_tmp784;});_tmp547;});struct Cyc_String_pa_PrintArg_struct _tmp3A1=({struct Cyc_String_pa_PrintArg_struct _tmp546;_tmp546.tag=0U,({struct _dyneither_ptr _tmp785=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp546.f1=_tmp785;});_tmp546;});struct Cyc_String_pa_PrintArg_struct _tmp3A2=({struct Cyc_String_pa_PrintArg_struct _tmp545;_tmp545.tag=0U,({
struct _dyneither_ptr _tmp786=(struct _dyneither_ptr)((struct _dyneither_ptr)(e->topt == 0?(struct _dyneither_ptr)({const char*_tmp3A3="?";_tag_dyneither(_tmp3A3,sizeof(char),2U);}): Cyc_Absynpp_typ2string((void*)_check_null(e->topt))));_tmp545.f1=_tmp786;});_tmp545;});void*_tmp39E[3U];_tmp39E[0]=& _tmp3A0,_tmp39E[1]=& _tmp3A1,_tmp39E[2]=& _tmp3A2;({unsigned int _tmp788=e->loc;struct _dyneither_ptr _tmp787=({const char*_tmp39F="datatype constructor %s expects argument of type %s but this argument has type %s";_tag_dyneither(_tmp39F,sizeof(char),82U);});Cyc_Tcutil_terr(_tmp788,_tmp787,_tag_dyneither(_tmp39E,sizeof(void*),3U));});});
Cyc_Tcutil_explain_failure();}}
# 2075
if(es != 0)
return({struct Cyc_String_pa_PrintArg_struct _tmp3A6=({struct Cyc_String_pa_PrintArg_struct _tmp548;_tmp548.tag=0U,({
# 2078
struct _dyneither_ptr _tmp789=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(tuf->name));_tmp548.f1=_tmp789;});_tmp548;});void*_tmp3A4[1U];_tmp3A4[0]=& _tmp3A6;({struct Cyc_Tcenv_Tenv*_tmp78D=te;unsigned int _tmp78C=((struct Cyc_Absyn_Exp*)es->hd)->loc;void**_tmp78B=topt;struct _dyneither_ptr _tmp78A=({const char*_tmp3A5="too many arguments for datatype constructor %s";_tag_dyneither(_tmp3A5,sizeof(char),47U);});Cyc_Tcexp_expr_err(_tmp78D,_tmp78C,_tmp78B,_tmp78A,_tag_dyneither(_tmp3A4,sizeof(void*),1U));});});
if(ts != 0)
return({struct Cyc_String_pa_PrintArg_struct _tmp3A9=({struct Cyc_String_pa_PrintArg_struct _tmp549;_tmp549.tag=0U,({
struct _dyneither_ptr _tmp78E=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(tuf->name));_tmp549.f1=_tmp78E;});_tmp549;});void*_tmp3A7[1U];_tmp3A7[0]=& _tmp3A9;({struct Cyc_Tcenv_Tenv*_tmp792=te;unsigned int _tmp791=loc;void**_tmp790=topt;struct _dyneither_ptr _tmp78F=({const char*_tmp3A8="too few arguments for datatype constructor %s";_tag_dyneither(_tmp3A8,sizeof(char),46U);});Cyc_Tcexp_expr_err(_tmp792,_tmp791,_tmp790,_tmp78F,_tag_dyneither(_tmp3A7,sizeof(void*),1U));});});
return res;};}
# 2085
static void Cyc_Tcexp_check_malloc_type(int allow_zero,unsigned int loc,void**topt,void*t){
# 2087
if(Cyc_Tcutil_is_bits_only_type(t) || allow_zero  && Cyc_Tcutil_zeroable_type(t))return;
# 2089
if(topt != 0){
void*_tmp3AA=Cyc_Tcutil_compress(*topt);void*_tmp3AB=_tmp3AA;void*_tmp3AC;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp3AB)->tag == 3U){_LL1: _tmp3AC=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp3AB)->f1).elt_type;_LL2:
# 2092
 Cyc_Tcutil_unify(_tmp3AC,t);
if(Cyc_Tcutil_is_bits_only_type(t) || allow_zero  && Cyc_Tcutil_zeroable_type(t))return;
goto _LL0;}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 2098
({struct Cyc_String_pa_PrintArg_struct _tmp3AF=({struct Cyc_String_pa_PrintArg_struct _tmp54C;_tmp54C.tag=0U,({
struct _dyneither_ptr _tmp793=(struct _dyneither_ptr)((struct _dyneither_ptr)(allow_zero?(struct _dyneither_ptr)({const char*_tmp3B1="calloc";_tag_dyneither(_tmp3B1,sizeof(char),7U);}):(struct _dyneither_ptr)({const char*_tmp3B2="malloc";_tag_dyneither(_tmp3B2,sizeof(char),7U);})));_tmp54C.f1=_tmp793;});_tmp54C;});struct Cyc_String_pa_PrintArg_struct _tmp3B0=({struct Cyc_String_pa_PrintArg_struct _tmp54B;_tmp54B.tag=0U,({struct _dyneither_ptr _tmp794=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp54B.f1=_tmp794;});_tmp54B;});void*_tmp3AD[2U];_tmp3AD[0]=& _tmp3AF,_tmp3AD[1]=& _tmp3B0;({unsigned int _tmp796=loc;struct _dyneither_ptr _tmp795=({const char*_tmp3AE="%s cannot be used with type %s\n\t(type needs initialization)";_tag_dyneither(_tmp3AE,sizeof(char),60U);});Cyc_Tcutil_terr(_tmp796,_tmp795,_tag_dyneither(_tmp3AD,sizeof(void*),2U));});});}
# 2102
static void*Cyc_Tcexp_mallocRgn(void*rgn){
# 2104
enum Cyc_Absyn_AliasQual _tmp3B3=(Cyc_Tcutil_type_kind(Cyc_Tcutil_compress(rgn)))->aliasqual;enum Cyc_Absyn_AliasQual _tmp3B4=_tmp3B3;if(_tmp3B4 == Cyc_Absyn_Unique){_LL1: _LL2:
 return Cyc_Absyn_unique_rgn_type;}else{_LL3: _LL4:
 return Cyc_Absyn_heap_rgn_type;}_LL0:;}
# 2110
static void*Cyc_Tcexp_tcMalloc(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp**ropt,void***t,struct Cyc_Absyn_Exp**e,int*is_calloc,int*is_fat){
# 2115
void*rgn=Cyc_Absyn_heap_rgn_type;
if(*ropt != 0){
# 2119
void*expected_type=
Cyc_Absyn_rgn_handle_type(Cyc_Absyn_new_evar(& Cyc_Tcutil_trko,Cyc_Tcenv_lookup_opt_type_vars(te)));
void*handle_type=Cyc_Tcexp_tcExp(te,& expected_type,(struct Cyc_Absyn_Exp*)_check_null(*ropt));
void*_tmp3B5=Cyc_Tcutil_compress(handle_type);void*_tmp3B6=_tmp3B5;void*_tmp3BA;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B6)->tag == 0U){if(((struct Cyc_Absyn_RgnHandleCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B6)->f1)->tag == 3U){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B6)->f2 != 0){_LL1: _tmp3BA=(void*)(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3B6)->f2)->hd;_LL2:
# 2124
 rgn=_tmp3BA;
Cyc_Tcenv_check_rgn_accessible(te,loc,rgn);
goto _LL0;}else{goto _LL3;}}else{goto _LL3;}}else{_LL3: _LL4:
# 2128
({struct Cyc_String_pa_PrintArg_struct _tmp3B9=({struct Cyc_String_pa_PrintArg_struct _tmp54D;_tmp54D.tag=0U,({
struct _dyneither_ptr _tmp797=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(handle_type));_tmp54D.f1=_tmp797;});_tmp54D;});void*_tmp3B7[1U];_tmp3B7[0]=& _tmp3B9;({unsigned int _tmp799=((struct Cyc_Absyn_Exp*)_check_null(*ropt))->loc;struct _dyneither_ptr _tmp798=({const char*_tmp3B8="expecting region_t type but found %s";_tag_dyneither(_tmp3B8,sizeof(char),37U);});Cyc_Tcutil_terr(_tmp799,_tmp798,_tag_dyneither(_tmp3B7,sizeof(void*),1U));});});
goto _LL0;}_LL0:;}else{
# 2135
if(topt != 0){
void*optrgn=Cyc_Absyn_void_type;
if(Cyc_Tcutil_rgn_of_pointer(*topt,& optrgn)){
rgn=Cyc_Tcexp_mallocRgn(optrgn);
if(rgn == Cyc_Absyn_unique_rgn_type)*ropt=Cyc_Absyn_uniquergn_exp;}}}
# 2142
({struct Cyc_Tcenv_Tenv*_tmp79A=Cyc_Tcenv_clear_abstract_val_ok(te);Cyc_Tcexp_tcExp(_tmp79A,& Cyc_Absyn_uint_type,*e);});{
# 2151 "tcexp.cyc"
void*elt_type;
struct Cyc_Absyn_Exp*num_elts;
int one_elt;
if(*is_calloc){
if(*t == 0)({void*_tmp3BB=0U;({struct _dyneither_ptr _tmp79B=({const char*_tmp3BC="calloc with empty type";_tag_dyneither(_tmp3BC,sizeof(char),23U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp79B,_tag_dyneither(_tmp3BB,sizeof(void*),0U));});});
elt_type=*((void**)_check_null(*t));
({unsigned int _tmp79E=loc;struct Cyc_Tcenv_Tenv*_tmp79D=te;struct Cyc_List_List*_tmp79C=Cyc_Tcenv_lookup_type_vars(te);Cyc_Tcutil_check_type(_tmp79E,_tmp79D,_tmp79C,& Cyc_Tcutil_tmk,1,0,elt_type);});
Cyc_Tcutil_check_no_qual(loc,elt_type);
Cyc_Tcexp_check_malloc_type(1,loc,topt,elt_type);
num_elts=*e;
one_elt=0;}else{
# 2163
void*er=(*e)->r;
retry_sizeof: {
void*_tmp3BD=er;struct Cyc_Absyn_Exp*_tmp3D2;struct Cyc_Absyn_Exp*_tmp3D1;void*_tmp3D0;switch(*((int*)_tmp3BD)){case 17U: _LL6: _tmp3D0=(void*)((struct Cyc_Absyn_Sizeoftype_e_Absyn_Raw_exp_struct*)_tmp3BD)->f1;_LL7:
# 2167
 elt_type=_tmp3D0;
({void**_tmp79F=({void**_tmp3BE=_cycalloc(sizeof(*_tmp3BE));*_tmp3BE=elt_type;_tmp3BE;});*t=_tmp79F;});
num_elts=Cyc_Absyn_uint_exp(1U,0U);
Cyc_Tcexp_tcExp(te,& Cyc_Absyn_uint_type,num_elts);
one_elt=1;
goto _LL5;case 3U: if(((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp3BD)->f1 == Cyc_Absyn_Times){if(((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp3BD)->f2 != 0){if(((struct Cyc_List_List*)((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp3BD)->f2)->tl != 0){if(((struct Cyc_List_List*)((struct Cyc_List_List*)((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp3BD)->f2)->tl)->tl == 0){_LL8: _tmp3D2=(struct Cyc_Absyn_Exp*)(((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp3BD)->f2)->hd;_tmp3D1=(struct Cyc_Absyn_Exp*)((((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp3BD)->f2)->tl)->hd;_LL9:
# 2174
{struct _tuple0 _tmp3BF=({struct _tuple0 _tmp54E;_tmp54E.f1=_tmp3D2->r,_tmp54E.f2=_tmp3D1->r;_tmp54E;});struct _tuple0 _tmp3C0=_tmp3BF;void*_tmp3C4;void*_tmp3C3;if(((struct Cyc_Absyn_Sizeoftype_e_Absyn_Raw_exp_struct*)_tmp3C0.f1)->tag == 17U){_LLD: _tmp3C3=(void*)((struct Cyc_Absyn_Sizeoftype_e_Absyn_Raw_exp_struct*)_tmp3C0.f1)->f1;_LLE:
# 2176
 Cyc_Tcexp_check_malloc_type(0,loc,topt,_tmp3C3);
elt_type=_tmp3C3;
({void**_tmp7A0=({void**_tmp3C1=_cycalloc(sizeof(*_tmp3C1));*_tmp3C1=elt_type;_tmp3C1;});*t=_tmp7A0;});
num_elts=_tmp3D1;
one_elt=0;
goto _LLC;}else{if(((struct Cyc_Absyn_Sizeoftype_e_Absyn_Raw_exp_struct*)_tmp3C0.f2)->tag == 17U){_LLF: _tmp3C4=(void*)((struct Cyc_Absyn_Sizeoftype_e_Absyn_Raw_exp_struct*)_tmp3C0.f2)->f1;_LL10:
# 2183
 Cyc_Tcexp_check_malloc_type(0,loc,topt,_tmp3C4);
elt_type=_tmp3C4;
({void**_tmp7A1=({void**_tmp3C2=_cycalloc(sizeof(*_tmp3C2));*_tmp3C2=elt_type;_tmp3C2;});*t=_tmp7A1;});
num_elts=_tmp3D2;
one_elt=0;
goto _LLC;}else{_LL11: _LL12:
 goto No_sizeof;}}_LLC:;}
# 2191
goto _LL5;}else{goto _LLA;}}else{goto _LLA;}}else{goto _LLA;}}else{goto _LLA;}default: _LLA: _LLB:
# 2193
 No_sizeof: {
# 2196
struct Cyc_Absyn_Exp*_tmp3C5=*e;
{void*_tmp3C6=_tmp3C5->r;void*_tmp3C7=_tmp3C6;struct Cyc_Absyn_Exp*_tmp3C8;if(((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp3C7)->tag == 14U){_LL14: _tmp3C8=((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp3C7)->f2;_LL15:
 _tmp3C5=_tmp3C8;goto _LL13;}else{_LL16: _LL17:
 goto _LL13;}_LL13:;}
# 2201
{void*_tmp3C9=Cyc_Tcutil_compress((void*)_check_null(_tmp3C5->topt));void*_tmp3CA=_tmp3C9;void*_tmp3CE;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3CA)->tag == 0U){if(((struct Cyc_Absyn_TagCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3CA)->f1)->tag == 4U){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3CA)->f2 != 0){_LL19: _tmp3CE=(void*)(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3CA)->f2)->hd;_LL1A:
# 2203
{void*_tmp3CB=Cyc_Tcutil_compress(_tmp3CE);void*_tmp3CC=_tmp3CB;struct Cyc_Absyn_Exp*_tmp3CD;if(((struct Cyc_Absyn_ValueofType_Absyn_Type_struct*)_tmp3CC)->tag == 9U){_LL1E: _tmp3CD=((struct Cyc_Absyn_ValueofType_Absyn_Type_struct*)_tmp3CC)->f1;_LL1F:
# 2205
 er=_tmp3CD->r;goto retry_sizeof;}else{_LL20: _LL21:
 goto _LL1D;}_LL1D:;}
# 2208
goto _LL18;}else{goto _LL1B;}}else{goto _LL1B;}}else{_LL1B: _LL1C:
 goto _LL18;}_LL18:;}
# 2211
elt_type=Cyc_Absyn_char_type;
({void**_tmp7A2=({void**_tmp3CF=_cycalloc(sizeof(*_tmp3CF));*_tmp3CF=elt_type;_tmp3CF;});*t=_tmp7A2;});
num_elts=*e;
one_elt=0;}
# 2216
goto _LL5;}_LL5:;}}
# 2220
*is_fat=!one_elt;
# 2223
{void*_tmp3D3=elt_type;struct Cyc_Absyn_Aggrdecl*_tmp3D6;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3D3)->tag == 0U){if(((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3D3)->f1)->tag == 20U){if(((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3D3)->f1)->f1).KnownAggr).tag == 2){_LL23: _tmp3D6=*((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3D3)->f1)->f1).KnownAggr).val;_LL24:
# 2225
 if(_tmp3D6->impl != 0  && ((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp3D6->impl))->exist_vars != 0)
({void*_tmp3D4=0U;({unsigned int _tmp7A4=loc;struct _dyneither_ptr _tmp7A3=({const char*_tmp3D5="malloc with existential types not yet implemented";_tag_dyneither(_tmp3D5,sizeof(char),50U);});Cyc_Tcutil_terr(_tmp7A4,_tmp7A3,_tag_dyneither(_tmp3D4,sizeof(void*),0U));});});
goto _LL22;}else{goto _LL25;}}else{goto _LL25;}}else{_LL25: _LL26:
 goto _LL22;}_LL22:;}{
# 2232
void*(*_tmp3D7)(void*t,void*rgn,struct Cyc_Absyn_Tqual tq,void*zero_term)=Cyc_Absyn_at_type;
void*_tmp3D8=Cyc_Absyn_false_type;
if(topt != 0){
void*_tmp3D9=Cyc_Tcutil_compress(*topt);void*_tmp3DA=_tmp3D9;void*_tmp3E6;void*_tmp3E5;void*_tmp3E4;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp3DA)->tag == 3U){_LL28: _tmp3E6=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp3DA)->f1).ptr_atts).nullable;_tmp3E5=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp3DA)->f1).ptr_atts).bounds;_tmp3E4=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp3DA)->f1).ptr_atts).zero_term;_LL29:
# 2237
 _tmp3D8=_tmp3E4;
if(Cyc_Tcutil_force_type2bool(0,_tmp3E6))
_tmp3D7=Cyc_Absyn_star_type;
# 2242
if(Cyc_Tcutil_force_type2bool(0,_tmp3E4) && !(*is_calloc)){
({void*_tmp3DB=0U;({unsigned int _tmp7A6=loc;struct _dyneither_ptr _tmp7A5=({const char*_tmp3DC="converting malloc to calloc to ensure zero-termination";_tag_dyneither(_tmp3DC,sizeof(char),55U);});Cyc_Tcutil_warn(_tmp7A6,_tmp7A5,_tag_dyneither(_tmp3DB,sizeof(void*),0U));});});
*is_calloc=1;}{
# 2248
struct Cyc_Absyn_Exp*_tmp3DD=({void*_tmp7A7=Cyc_Absyn_bounds_one();Cyc_Tcutil_get_bounds_exp(_tmp7A7,_tmp3E5);});
if(_tmp3DD != 0  && !one_elt){
struct Cyc_Absyn_Exp*_tmp3DE=_tmp3DD;
int _tmp3DF=Cyc_Evexp_c_can_eval(num_elts);
if(_tmp3DF  && Cyc_Evexp_same_const_exp(_tmp3DE,num_elts)){
*is_fat=0;
return({void*_tmp7AB=elt_type;void*_tmp7AA=rgn;struct Cyc_Absyn_Tqual _tmp7A9=Cyc_Absyn_empty_tqual(0U);void*_tmp7A8=_tmp3E5;Cyc_Absyn_atb_type(_tmp7AB,_tmp7AA,_tmp7A9,_tmp7A8,_tmp3D8);});}{
# 2256
void*_tmp3E0=Cyc_Tcutil_compress((void*)_check_null(num_elts->topt));void*_tmp3E1=_tmp3E0;void*_tmp3E3;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3E1)->tag == 0U){if(((struct Cyc_Absyn_TagCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3E1)->f1)->tag == 4U){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3E1)->f2 != 0){_LL2D: _tmp3E3=(void*)(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp3E1)->f2)->hd;_LL2E: {
# 2258
struct Cyc_Absyn_Exp*_tmp3E2=({void*_tmp7AC=Cyc_Absyn_uint_type;Cyc_Absyn_cast_exp(_tmp7AC,Cyc_Absyn_valueof_exp(_tmp3E3,0U),0,Cyc_Absyn_No_coercion,0U);});
# 2260
if(Cyc_Evexp_same_const_exp(_tmp3E2,_tmp3DE)){
*is_fat=0;
return({void*_tmp7B0=elt_type;void*_tmp7AF=rgn;struct Cyc_Absyn_Tqual _tmp7AE=Cyc_Absyn_empty_tqual(0U);void*_tmp7AD=_tmp3E5;Cyc_Absyn_atb_type(_tmp7B0,_tmp7AF,_tmp7AE,_tmp7AD,_tmp3D8);});}
# 2264
goto _LL2C;}}else{goto _LL2F;}}else{goto _LL2F;}}else{_LL2F: _LL30:
 goto _LL2C;}_LL2C:;};}
# 2268
goto _LL27;};}else{_LL2A: _LL2B:
 goto _LL27;}_LL27:;}
# 2271
if(!one_elt)_tmp3D7=Cyc_Absyn_fatptr_type;
return({void*(*_tmp7B4)(void*t,void*rgn,struct Cyc_Absyn_Tqual tq,void*zero_term)=_tmp3D7;void*_tmp7B3=elt_type;void*_tmp7B2=rgn;struct Cyc_Absyn_Tqual _tmp7B1=Cyc_Absyn_empty_tqual(0U);_tmp7B4(_tmp7B3,_tmp7B2,_tmp7B1,_tmp3D8);});};};}
# 2276
static void*Cyc_Tcexp_tcSwap(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2){
# 2278
struct Cyc_Tcenv_Tenv*_tmp3E7=Cyc_Tcenv_enter_lhs(te);
Cyc_Tcexp_tcExpNoPromote(_tmp3E7,0,e1);{
void*_tmp3E8=(void*)_check_null(e1->topt);
Cyc_Tcexp_tcExpNoPromote(_tmp3E7,& _tmp3E8,e2);{
void*t1=(void*)_check_null(e1->topt);
void*t2=(void*)_check_null(e2->topt);
# 2285
{void*_tmp3E9=Cyc_Tcutil_compress(t1);void*_tmp3EA=_tmp3E9;if(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp3EA)->tag == 4U){_LL1: _LL2:
({void*_tmp3EB=0U;({unsigned int _tmp7B6=loc;struct _dyneither_ptr _tmp7B5=({const char*_tmp3EC="cannot assign to an array";_tag_dyneither(_tmp3EC,sizeof(char),26U);});Cyc_Tcutil_terr(_tmp7B6,_tmp7B5,_tag_dyneither(_tmp3EB,sizeof(void*),0U));});});goto _LL0;}else{_LL3: _LL4:
 goto _LL0;}_LL0:;}
# 2290
if(!Cyc_Tcutil_is_boxed(t1) && !Cyc_Tcutil_is_pointer_type(t1))
({void*_tmp3ED=0U;({unsigned int _tmp7B8=loc;struct _dyneither_ptr _tmp7B7=({const char*_tmp3EE="Swap not allowed for non-pointer or non-word-sized types.";_tag_dyneither(_tmp3EE,sizeof(char),58U);});Cyc_Tcutil_terr(_tmp7B8,_tmp7B7,_tag_dyneither(_tmp3ED,sizeof(void*),0U));});});
# 2294
if(!Cyc_Absyn_is_lvalue(e1))
return({void*_tmp3EF=0U;({struct Cyc_Tcenv_Tenv*_tmp7BC=te;unsigned int _tmp7BB=e1->loc;void**_tmp7BA=topt;struct _dyneither_ptr _tmp7B9=({const char*_tmp3F0="swap non-lvalue";_tag_dyneither(_tmp3F0,sizeof(char),16U);});Cyc_Tcexp_expr_err(_tmp7BC,_tmp7BB,_tmp7BA,_tmp7B9,_tag_dyneither(_tmp3EF,sizeof(void*),0U));});});
if(!Cyc_Absyn_is_lvalue(e2))
return({void*_tmp3F1=0U;({struct Cyc_Tcenv_Tenv*_tmp7C0=te;unsigned int _tmp7BF=e2->loc;void**_tmp7BE=topt;struct _dyneither_ptr _tmp7BD=({const char*_tmp3F2="swap non-lvalue";_tag_dyneither(_tmp3F2,sizeof(char),16U);});Cyc_Tcexp_expr_err(_tmp7C0,_tmp7BF,_tmp7BE,_tmp7BD,_tag_dyneither(_tmp3F1,sizeof(void*),0U));});});
# 2299
Cyc_Tcexp_check_writable(te,e1);
Cyc_Tcexp_check_writable(te,e2);
if(!Cyc_Tcutil_unify(t1,t2)){
void*_tmp3F3=({struct Cyc_String_pa_PrintArg_struct _tmp3F6=({struct Cyc_String_pa_PrintArg_struct _tmp550;_tmp550.tag=0U,({
struct _dyneither_ptr _tmp7C1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t1));_tmp550.f1=_tmp7C1;});_tmp550;});struct Cyc_String_pa_PrintArg_struct _tmp3F7=({struct Cyc_String_pa_PrintArg_struct _tmp54F;_tmp54F.tag=0U,({struct _dyneither_ptr _tmp7C2=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t2));_tmp54F.f1=_tmp7C2;});_tmp54F;});void*_tmp3F4[2U];_tmp3F4[0]=& _tmp3F6,_tmp3F4[1]=& _tmp3F7;({struct Cyc_Tcenv_Tenv*_tmp7C6=te;unsigned int _tmp7C5=loc;void**_tmp7C4=topt;struct _dyneither_ptr _tmp7C3=({const char*_tmp3F5="type mismatch: %s != %s";_tag_dyneither(_tmp3F5,sizeof(char),24U);});Cyc_Tcexp_expr_err(_tmp7C6,_tmp7C5,_tmp7C4,_tmp7C3,_tag_dyneither(_tmp3F4,sizeof(void*),2U));});});
return _tmp3F3;}
# 2306
return Cyc_Absyn_void_type;};};}
# 2311
static void*Cyc_Tcexp_tcStmtExp(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Stmt*s){
({struct Cyc_Tcenv_Tenv*_tmp7C7=Cyc_Tcenv_enter_stmt_exp(Cyc_Tcenv_clear_abstract_val_ok(te));Cyc_Tcstmt_tcStmt(_tmp7C7,s,1);});
# 2314
while(1){
void*_tmp3F8=s->r;void*_tmp3F9=_tmp3F8;struct Cyc_Absyn_Decl*_tmp404;struct Cyc_Absyn_Stmt*_tmp403;struct Cyc_Absyn_Stmt*_tmp402;struct Cyc_Absyn_Stmt*_tmp401;struct Cyc_Absyn_Exp*_tmp400;switch(*((int*)_tmp3F9)){case 1U: _LL1: _tmp400=((struct Cyc_Absyn_Exp_s_Absyn_Raw_stmt_struct*)_tmp3F9)->f1;_LL2: {
# 2318
void*_tmp3FA=(void*)_check_null(_tmp400->topt);
if(!({void*_tmp7C8=_tmp3FA;Cyc_Tcutil_unify(_tmp7C8,Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te)));})){
({struct Cyc_String_pa_PrintArg_struct _tmp3FD=({struct Cyc_String_pa_PrintArg_struct _tmp551;_tmp551.tag=0U,({
struct _dyneither_ptr _tmp7C9=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp3FA));_tmp551.f1=_tmp7C9;});_tmp551;});void*_tmp3FB[1U];_tmp3FB[0]=& _tmp3FD;({unsigned int _tmp7CB=loc;struct _dyneither_ptr _tmp7CA=({const char*_tmp3FC="statement expression returns type %s";_tag_dyneither(_tmp3FC,sizeof(char),37U);});Cyc_Tcutil_terr(_tmp7CB,_tmp7CA,_tag_dyneither(_tmp3FB,sizeof(void*),1U));});});
Cyc_Tcutil_explain_failure();}
# 2324
return _tmp3FA;}case 2U: _LL3: _tmp402=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp3F9)->f1;_tmp401=((struct Cyc_Absyn_Seq_s_Absyn_Raw_stmt_struct*)_tmp3F9)->f2;_LL4:
 s=_tmp401;continue;case 12U: _LL5: _tmp404=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp3F9)->f1;_tmp403=((struct Cyc_Absyn_Decl_s_Absyn_Raw_stmt_struct*)_tmp3F9)->f2;_LL6:
 s=_tmp403;continue;default: _LL7: _LL8:
# 2328
 return({void*_tmp3FE=0U;({struct Cyc_Tcenv_Tenv*_tmp7CF=te;unsigned int _tmp7CE=loc;void**_tmp7CD=topt;struct _dyneither_ptr _tmp7CC=({const char*_tmp3FF="statement expression must end with expression";_tag_dyneither(_tmp3FF,sizeof(char),46U);});Cyc_Tcexp_expr_err(_tmp7CF,_tmp7CE,_tmp7CD,_tmp7CC,_tag_dyneither(_tmp3FE,sizeof(void*),0U));});});}_LL0:;}}
# 2333
static void*Cyc_Tcexp_tcTagcheck(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp*e,struct _dyneither_ptr*f){
void*t=Cyc_Tcutil_compress(({struct Cyc_Tcenv_Tenv*_tmp7D0=Cyc_Tcenv_enter_abstract_val_ok(te);Cyc_Tcexp_tcExp(_tmp7D0,0,e);}));
{void*_tmp405=t;struct Cyc_Absyn_Aggrdecl*_tmp409;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp405)->tag == 0U){if(((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp405)->f1)->tag == 20U){if(((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp405)->f1)->f1).KnownAggr).tag == 2){_LL1: _tmp409=*((((struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp405)->f1)->f1).KnownAggr).val;_LL2:
# 2337
 if((_tmp409->kind == Cyc_Absyn_UnionA  && _tmp409->impl != 0) && ((struct Cyc_Absyn_AggrdeclImpl*)_check_null(_tmp409->impl))->tagged)goto _LL0;
goto _LL4;}else{goto _LL3;}}else{goto _LL3;}}else{_LL3: _LL4:
# 2340
({struct Cyc_String_pa_PrintArg_struct _tmp408=({struct Cyc_String_pa_PrintArg_struct _tmp552;_tmp552.tag=0U,({struct _dyneither_ptr _tmp7D1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(t));_tmp552.f1=_tmp7D1;});_tmp552;});void*_tmp406[1U];_tmp406[0]=& _tmp408;({unsigned int _tmp7D3=loc;struct _dyneither_ptr _tmp7D2=({const char*_tmp407="expecting @tagged union but found %s";_tag_dyneither(_tmp407,sizeof(char),37U);});Cyc_Tcutil_terr(_tmp7D3,_tmp7D2,_tag_dyneither(_tmp406,sizeof(void*),1U));});});
goto _LL0;}_LL0:;}
# 2343
return Cyc_Absyn_uint_type;}
# 2347
static void*Cyc_Tcexp_tcNew(struct Cyc_Tcenv_Tenv*te,unsigned int loc,void**topt,struct Cyc_Absyn_Exp**rgn_handle,struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Exp*e1){
# 2351
void*rgn=Cyc_Absyn_heap_rgn_type;
struct Cyc_Tcenv_Tenv*_tmp40A=Cyc_Tcenv_clear_abstract_val_ok(Cyc_Tcenv_set_new_status(Cyc_Tcenv_InNew,te));
if(*rgn_handle != 0){
# 2356
void*expected_type=
Cyc_Absyn_rgn_handle_type(Cyc_Absyn_new_evar(& Cyc_Tcutil_trko,Cyc_Tcenv_lookup_opt_type_vars(_tmp40A)));
void*handle_type=Cyc_Tcexp_tcExp(_tmp40A,& expected_type,(struct Cyc_Absyn_Exp*)_check_null(*rgn_handle));
void*_tmp40B=Cyc_Tcutil_compress(handle_type);void*_tmp40C=_tmp40B;void*_tmp410;if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp40C)->tag == 0U){if(((struct Cyc_Absyn_RgnHandleCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp40C)->f1)->tag == 3U){if(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp40C)->f2 != 0){_LL1: _tmp410=(void*)(((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp40C)->f2)->hd;_LL2:
# 2361
 rgn=_tmp410;
Cyc_Tcenv_check_rgn_accessible(_tmp40A,loc,rgn);
goto _LL0;}else{goto _LL3;}}else{goto _LL3;}}else{_LL3: _LL4:
# 2365
({struct Cyc_String_pa_PrintArg_struct _tmp40F=({struct Cyc_String_pa_PrintArg_struct _tmp553;_tmp553.tag=0U,({
struct _dyneither_ptr _tmp7D4=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(handle_type));_tmp553.f1=_tmp7D4;});_tmp553;});void*_tmp40D[1U];_tmp40D[0]=& _tmp40F;({unsigned int _tmp7D6=((struct Cyc_Absyn_Exp*)_check_null(*rgn_handle))->loc;struct _dyneither_ptr _tmp7D5=({const char*_tmp40E="expecting region_t type but found %s";_tag_dyneither(_tmp40E,sizeof(char),37U);});Cyc_Tcutil_terr(_tmp7D6,_tmp7D5,_tag_dyneither(_tmp40D,sizeof(void*),1U));});});
goto _LL0;}_LL0:;}else{
# 2372
if(topt != 0){
void*optrgn=Cyc_Absyn_void_type;
if(Cyc_Tcutil_rgn_of_pointer(*topt,& optrgn)){
rgn=Cyc_Tcexp_mallocRgn(optrgn);
if(rgn == Cyc_Absyn_unique_rgn_type)*rgn_handle=Cyc_Absyn_uniquergn_exp;}}}{
# 2380
void*_tmp411=e1->r;void*_tmp412=_tmp411;struct Cyc_List_List*_tmp42A;struct Cyc_Core_Opt*_tmp429;struct Cyc_List_List*_tmp428;switch(*((int*)_tmp412)){case 27U: _LL6: _LL7:
 goto _LL9;case 28U: _LL8: _LL9: {
# 2385
void*res_typ=Cyc_Tcexp_tcExpNoPromote(_tmp40A,topt,e1);
if(!Cyc_Tcutil_is_array_type(res_typ))
({void*_tmp413=0U;({struct _dyneither_ptr _tmp7D7=({const char*_tmp414="tcNew: comprehension returned non-array type";_tag_dyneither(_tmp414,sizeof(char),45U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp7D7,_tag_dyneither(_tmp413,sizeof(void*),0U));});});
res_typ=Cyc_Tcutil_promote_array(res_typ,rgn,1);
if(topt != 0){
if(!Cyc_Tcutil_unify(*topt,res_typ) && Cyc_Tcutil_silent_castable(_tmp40A,loc,res_typ,*topt)){
e->topt=res_typ;
Cyc_Tcutil_unchecked_cast(_tmp40A,e,*topt,Cyc_Absyn_Other_coercion);
res_typ=*topt;}}
# 2396
return res_typ;}case 36U: _LLA: _tmp429=((struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmp412)->f1;_tmp428=((struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmp412)->f2;_LLB:
# 2398
({void*_tmp7D8=(void*)({struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*_tmp415=_cycalloc(sizeof(*_tmp415));_tmp415->tag=26U,_tmp415->f1=_tmp428;_tmp415;});e1->r=_tmp7D8;});
_tmp42A=_tmp428;goto _LLD;case 26U: _LLC: _tmp42A=((struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*)_tmp412)->f1;_LLD: {
# 2401
void**elt_typ_opt=0;
int zero_term=0;
if(topt != 0){
void*_tmp416=Cyc_Tcutil_compress(*topt);void*_tmp417=_tmp416;void**_tmp41A;struct Cyc_Absyn_Tqual _tmp419;void*_tmp418;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp417)->tag == 3U){_LL15: _tmp41A=(void**)&(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp417)->f1).elt_type;_tmp419=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp417)->f1).elt_tq;_tmp418=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp417)->f1).ptr_atts).zero_term;_LL16:
# 2407
 elt_typ_opt=_tmp41A;
zero_term=Cyc_Tcutil_force_type2bool(0,_tmp418);
goto _LL14;}else{_LL17: _LL18:
 goto _LL14;}_LL14:;}{
# 2413
void*res_typ=Cyc_Tcexp_tcArray(_tmp40A,e1->loc,elt_typ_opt,0,zero_term,_tmp42A);
e1->topt=res_typ;
if(!Cyc_Tcutil_is_array_type(res_typ))
({void*_tmp41B=0U;({struct _dyneither_ptr _tmp7D9=({const char*_tmp41C="tcExpNoPromote on Array_e returned non-array type";_tag_dyneither(_tmp41C,sizeof(char),50U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp7D9,_tag_dyneither(_tmp41B,sizeof(void*),0U));});});
res_typ=Cyc_Tcutil_promote_array(res_typ,rgn,0);
if(topt != 0){
# 2422
if(!Cyc_Tcutil_unify(*topt,res_typ) && Cyc_Tcutil_silent_castable(_tmp40A,loc,res_typ,*topt)){
e->topt=res_typ;
Cyc_Tcutil_unchecked_cast(_tmp40A,e,*topt,Cyc_Absyn_Other_coercion);
res_typ=*topt;}}
# 2428
return res_typ;};}case 0U: switch(((((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp412)->f1).Wstring_c).tag){case 8U: _LLE: _LLF: {
# 2433
void*_tmp41D=({void*_tmp7DD=Cyc_Absyn_char_type;void*_tmp7DC=rgn;struct Cyc_Absyn_Tqual _tmp7DB=Cyc_Absyn_const_tqual(0U);void*_tmp7DA=Cyc_Absyn_fat_bound_type;Cyc_Absyn_atb_type(_tmp7DD,_tmp7DC,_tmp7DB,_tmp7DA,Cyc_Absyn_true_type);});
# 2435
void*_tmp41E=Cyc_Tcexp_tcExp(_tmp40A,& _tmp41D,e1);
return({void*_tmp7E1=_tmp41E;void*_tmp7E0=rgn;struct Cyc_Absyn_Tqual _tmp7DF=Cyc_Absyn_empty_tqual(0U);void*_tmp7DE=Cyc_Absyn_thin_bounds_exp(Cyc_Absyn_uint_exp(1U,0U));Cyc_Absyn_atb_type(_tmp7E1,_tmp7E0,_tmp7DF,_tmp7DE,Cyc_Absyn_false_type);});}case 9U: _LL10: _LL11: {
# 2440
void*_tmp41F=({void*_tmp7E5=Cyc_Absyn_wchar_type();void*_tmp7E4=rgn;struct Cyc_Absyn_Tqual _tmp7E3=Cyc_Absyn_const_tqual(0U);void*_tmp7E2=Cyc_Absyn_fat_bound_type;Cyc_Absyn_atb_type(_tmp7E5,_tmp7E4,_tmp7E3,_tmp7E2,Cyc_Absyn_true_type);});
# 2442
void*_tmp420=Cyc_Tcexp_tcExp(_tmp40A,& _tmp41F,e1);
return({void*_tmp7E9=_tmp420;void*_tmp7E8=rgn;struct Cyc_Absyn_Tqual _tmp7E7=Cyc_Absyn_empty_tqual(0U);void*_tmp7E6=Cyc_Absyn_thin_bounds_exp(Cyc_Absyn_uint_exp(1U,0U));Cyc_Absyn_atb_type(_tmp7E9,_tmp7E8,_tmp7E7,_tmp7E6,Cyc_Absyn_false_type);});}default: goto _LL12;}default: _LL12: _LL13:
# 2449
 RG: {
void*bogus=Cyc_Absyn_void_type;
void**topt2=0;
if(topt != 0){
void*_tmp421=Cyc_Tcutil_compress(*topt);void*_tmp422=_tmp421;void**_tmp424;struct Cyc_Absyn_Tqual _tmp423;switch(*((int*)_tmp422)){case 3U: _LL1A: _tmp424=(void**)&(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp422)->f1).elt_type;_tmp423=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp422)->f1).elt_tq;_LL1B:
# 2455
 topt2=_tmp424;goto _LL19;case 0U: if(((struct Cyc_Absyn_DatatypeCon_Absyn_TyCon_struct*)((struct Cyc_Absyn_AppType_Absyn_Type_struct*)_tmp422)->f1)->tag == 18U){_LL1C: _LL1D:
# 2459
 bogus=*topt;
topt2=& bogus;
goto _LL19;}else{goto _LL1E;}default: _LL1E: _LL1F:
 goto _LL19;}_LL19:;}{
# 2465
void*telt=Cyc_Tcexp_tcExp(_tmp40A,topt2,e1);
# 2467
if(Cyc_Tcutil_is_noalias_pointer_or_aggr(telt) && !Cyc_Tcutil_is_noalias_path(e1))
({void*_tmp425=0U;({unsigned int _tmp7EB=e1->loc;struct _dyneither_ptr _tmp7EA=({const char*_tmp426="Cannot consume non-unique paths; do swap instead";_tag_dyneither(_tmp426,sizeof(char),49U);});Cyc_Tcutil_terr(_tmp7EB,_tmp7EA,_tag_dyneither(_tmp425,sizeof(void*),0U));});});{
void*res_typ=(void*)({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp427=_cycalloc(sizeof(*_tmp427));
_tmp427->tag=3U,(_tmp427->f1).elt_type=telt,({struct Cyc_Absyn_Tqual _tmp7EE=Cyc_Absyn_empty_tqual(0U);(_tmp427->f1).elt_tq=_tmp7EE;}),
((_tmp427->f1).ptr_atts).rgn=rgn,({void*_tmp7ED=Cyc_Tcutil_any_bool(& _tmp40A);((_tmp427->f1).ptr_atts).nullable=_tmp7ED;}),({
void*_tmp7EC=Cyc_Absyn_bounds_one();((_tmp427->f1).ptr_atts).bounds=_tmp7EC;}),((_tmp427->f1).ptr_atts).zero_term=Cyc_Absyn_false_type,((_tmp427->f1).ptr_atts).ptrloc=0;_tmp427;});
# 2474
if(topt != 0){
if(!Cyc_Tcutil_unify(*topt,res_typ) && Cyc_Tcutil_silent_castable(_tmp40A,loc,res_typ,*topt)){
e->topt=res_typ;
Cyc_Tcutil_unchecked_cast(_tmp40A,e,*topt,Cyc_Absyn_Other_coercion);
res_typ=*topt;}}
# 2481
return res_typ;};};}}_LL5:;};}
# 2487
void*Cyc_Tcexp_tcExp(struct Cyc_Tcenv_Tenv*te,void**topt,struct Cyc_Absyn_Exp*e){
void*t=Cyc_Tcutil_compress(Cyc_Tcexp_tcExpNoPromote(te,topt,e));
if(Cyc_Tcutil_is_array_type(t))
({void*_tmp7F0=t=({void*_tmp7EF=t;Cyc_Tcutil_promote_array(_tmp7EF,(Cyc_Tcutil_addressof_props(te,e)).f2,0);});e->topt=_tmp7F0;});
return t;}
# 2497
void*Cyc_Tcexp_tcExpInitializer(struct Cyc_Tcenv_Tenv*te,void**topt,struct Cyc_Absyn_Exp*e){
void*t=Cyc_Tcexp_tcExpNoPromote(te,topt,e);
# 2501
if(Cyc_Tcutil_is_noalias_pointer_or_aggr(t) && !Cyc_Tcutil_is_noalias_path(e))
# 2506
({void*_tmp42B=0U;({unsigned int _tmp7F2=e->loc;struct _dyneither_ptr _tmp7F1=({const char*_tmp42C="Cannot consume non-unique paths; do swap instead";_tag_dyneither(_tmp42C,sizeof(char),49U);});Cyc_Tcutil_terr(_tmp7F2,_tmp7F1,_tag_dyneither(_tmp42B,sizeof(void*),0U));});});{
# 2508
void*_tmp42D=e->r;void*_tmp42E=_tmp42D;switch(*((int*)_tmp42E)){case 26U: _LL1: _LL2:
 goto _LL4;case 27U: _LL3: _LL4:
 goto _LL6;case 28U: _LL5: _LL6:
 goto _LL8;case 0U: switch(((((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp42E)->f1).String_c).tag){case 9U: _LL7: _LL8:
 goto _LLA;case 8U: _LL9: _LLA:
 return t;default: goto _LLB;}default: _LLB: _LLC:
# 2515
 t=Cyc_Tcutil_compress(t);
if(Cyc_Tcutil_is_array_type(t)){
t=({void*_tmp7F3=t;Cyc_Tcutil_promote_array(_tmp7F3,(Cyc_Tcutil_addressof_props(te,e)).f2,0);});
Cyc_Tcutil_unchecked_cast(te,e,t,Cyc_Absyn_Other_coercion);}
# 2520
return t;}_LL0:;};}
# 2531 "tcexp.cyc"
static void*Cyc_Tcexp_tcExpNoPromote(struct Cyc_Tcenv_Tenv*te,void**topt,struct Cyc_Absyn_Exp*e){
{void*_tmp42F=e->r;void*_tmp430=_tmp42F;struct Cyc_Absyn_Exp*_tmp44E;if(((struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct*)_tmp430)->tag == 12U){_LL1: _tmp44E=((struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct*)_tmp430)->f1;_LL2:
# 2535
 Cyc_Tcexp_tcExpNoInst(te,topt,_tmp44E);
({void*_tmp7F4=Cyc_Absyn_pointer_expand((void*)_check_null(_tmp44E->topt),0);_tmp44E->topt=_tmp7F4;});
e->topt=_tmp44E->topt;
goto _LL0;}else{_LL3: _LL4:
# 2541
 Cyc_Tcexp_tcExpNoInst(te,topt,e);
({void*_tmp7F5=Cyc_Absyn_pointer_expand((void*)_check_null(e->topt),0);e->topt=_tmp7F5;});
# 2544
{void*_tmp431=Cyc_Tcutil_compress((void*)_check_null(e->topt));void*_tmp432=_tmp431;void*_tmp44D;struct Cyc_Absyn_Tqual _tmp44C;void*_tmp44B;void*_tmp44A;void*_tmp449;void*_tmp448;if(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp432)->tag == 3U){_LL6: _tmp44D=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp432)->f1).elt_type;_tmp44C=(((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp432)->f1).elt_tq;_tmp44B=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp432)->f1).ptr_atts).rgn;_tmp44A=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp432)->f1).ptr_atts).nullable;_tmp449=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp432)->f1).ptr_atts).bounds;_tmp448=((((struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_tmp432)->f1).ptr_atts).zero_term;_LL7:
# 2546
{void*_tmp433=Cyc_Tcutil_compress(_tmp44D);void*_tmp434=_tmp433;struct Cyc_List_List*_tmp447;void*_tmp446;struct Cyc_Absyn_Tqual _tmp445;void*_tmp444;struct Cyc_List_List*_tmp443;int _tmp442;struct Cyc_Absyn_VarargInfo*_tmp441;struct Cyc_List_List*_tmp440;struct Cyc_List_List*_tmp43F;struct Cyc_Absyn_Exp*_tmp43E;struct Cyc_List_List*_tmp43D;struct Cyc_Absyn_Exp*_tmp43C;struct Cyc_List_List*_tmp43B;if(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->tag == 5U){_LLB: _tmp447=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).tvars;_tmp446=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).effect;_tmp445=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).ret_tqual;_tmp444=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).ret_type;_tmp443=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).args;_tmp442=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).c_varargs;_tmp441=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).cyc_varargs;_tmp440=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).rgn_po;_tmp43F=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).attributes;_tmp43E=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).requires_clause;_tmp43D=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).requires_relns;_tmp43C=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).ensures_clause;_tmp43B=(((struct Cyc_Absyn_FnType_Absyn_Type_struct*)_tmp434)->f1).ensures_relns;_LLC:
# 2548
 if(_tmp447 != 0){
struct _tuple12 _tmp435=({struct _tuple12 _tmp554;({struct Cyc_List_List*_tmp7F6=Cyc_Tcenv_lookup_type_vars(te);_tmp554.f1=_tmp7F6;}),_tmp554.f2=Cyc_Core_heap_region;_tmp554;});
struct Cyc_List_List*inst=
((struct Cyc_List_List*(*)(struct _tuple13*(*f)(struct _tuple12*,struct Cyc_Absyn_Tvar*),struct _tuple12*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcutil_r_make_inst_var,& _tmp435,_tmp447);
struct Cyc_List_List*ts=((struct Cyc_List_List*(*)(void*(*f)(struct _tuple13*),struct Cyc_List_List*x))Cyc_List_map)((void*(*)(struct _tuple13*))Cyc_Core_snd,inst);
# 2556
_tmp440=Cyc_Tcutil_rsubst_rgnpo(Cyc_Core_heap_region,inst,_tmp440);
Cyc_Tcenv_check_rgn_partial_order(te,e->loc,_tmp440);{
void*ftyp=({struct Cyc_List_List*_tmp7F7=inst;Cyc_Tcutil_substitute(_tmp7F7,(void*)({struct Cyc_Absyn_FnType_Absyn_Type_struct*_tmp43A=_cycalloc(sizeof(*_tmp43A));
_tmp43A->tag=5U,(_tmp43A->f1).tvars=0,(_tmp43A->f1).effect=_tmp446,(_tmp43A->f1).ret_tqual=_tmp445,(_tmp43A->f1).ret_type=_tmp444,(_tmp43A->f1).args=_tmp443,(_tmp43A->f1).c_varargs=_tmp442,(_tmp43A->f1).cyc_varargs=_tmp441,(_tmp43A->f1).rgn_po=0,(_tmp43A->f1).attributes=_tmp43F,(_tmp43A->f1).requires_clause=_tmp43E,(_tmp43A->f1).requires_relns=_tmp43D,(_tmp43A->f1).ensures_clause=_tmp43C,(_tmp43A->f1).ensures_relns=_tmp43B;_tmp43A;}));});
# 2563
struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp436=({struct Cyc_Absyn_PointerType_Absyn_Type_struct*_tmp439=_cycalloc(sizeof(*_tmp439));_tmp439->tag=3U,(_tmp439->f1).elt_type=ftyp,(_tmp439->f1).elt_tq=_tmp44C,((_tmp439->f1).ptr_atts).rgn=_tmp44B,((_tmp439->f1).ptr_atts).nullable=_tmp44A,((_tmp439->f1).ptr_atts).bounds=_tmp449,((_tmp439->f1).ptr_atts).zero_term=_tmp448,((_tmp439->f1).ptr_atts).ptrloc=0;_tmp439;});
# 2565
struct Cyc_Absyn_Exp*_tmp437=Cyc_Absyn_copy_exp(e);
({void*_tmp7F8=(void*)({struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct*_tmp438=_cycalloc(sizeof(*_tmp438));_tmp438->tag=13U,_tmp438->f1=_tmp437,_tmp438->f2=ts;_tmp438;});e->r=_tmp7F8;});
e->topt=(void*)_tmp436;};}
# 2569
goto _LLA;}else{_LLD: _LLE:
 goto _LLA;}_LLA:;}
# 2572
goto _LL5;}else{_LL8: _LL9:
 goto _LL5;}_LL5:;}
# 2575
goto _LL0;}_LL0:;}
# 2577
return(void*)_check_null(e->topt);}
# 2585
static void Cyc_Tcexp_insert_alias_stmts(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Exp*fn_exp,struct Cyc_List_List*alias_arg_exps){
# 2587
struct Cyc_List_List*_tmp44F=0;
# 2589
{void*_tmp450=fn_exp->r;void*_tmp451=_tmp450;struct Cyc_List_List*_tmp465;if(((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp451)->tag == 10U){_LL1: _tmp465=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp451)->f2;_LL2:
# 2591
{void*_tmp452=e->r;void*_tmp453=_tmp452;struct Cyc_List_List*_tmp462;if(((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp453)->tag == 10U){_LL6: _tmp462=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp453)->f2;_LL7: {
# 2593
struct Cyc_List_List*_tmp454=alias_arg_exps;
int _tmp455=0;
while(_tmp454 != 0){
while(_tmp455 != (int)_tmp454->hd){
if(_tmp465 == 0)
({void*_tmp456=0U;({struct _dyneither_ptr _tmp7FA=(struct _dyneither_ptr)({struct Cyc_Int_pa_PrintArg_struct _tmp459=({struct Cyc_Int_pa_PrintArg_struct _tmp556;_tmp556.tag=1U,_tmp556.f1=(unsigned long)_tmp455;_tmp556;});struct Cyc_Int_pa_PrintArg_struct _tmp45A=({struct Cyc_Int_pa_PrintArg_struct _tmp555;_tmp555.tag=1U,_tmp555.f1=(unsigned long)((int)_tmp454->hd);_tmp555;});void*_tmp457[2U];_tmp457[0]=& _tmp459,_tmp457[1]=& _tmp45A;({struct _dyneither_ptr _tmp7F9=({const char*_tmp458="bad count %d/%d for alias coercion!";_tag_dyneither(_tmp458,sizeof(char),36U);});Cyc_aprintf(_tmp7F9,_tag_dyneither(_tmp457,sizeof(void*),2U));});});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp7FA,_tag_dyneither(_tmp456,sizeof(void*),0U));});});
# 2600
++ _tmp455;
_tmp465=_tmp465->tl;
_tmp462=((struct Cyc_List_List*)_check_null(_tmp462))->tl;}{
# 2605
struct _tuple11 _tmp45B=({struct Cyc_Absyn_Exp*_tmp7FB=(struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(_tmp465))->hd;Cyc_Tcutil_insert_alias(_tmp7FB,Cyc_Tcutil_copy_type((void*)_check_null(((struct Cyc_Absyn_Exp*)((struct Cyc_List_List*)_check_null(_tmp462))->hd)->topt)));});struct _tuple11 _tmp45C=_tmp45B;struct Cyc_Absyn_Decl*_tmp45F;struct Cyc_Absyn_Exp*_tmp45E;_LLB: _tmp45F=_tmp45C.f1;_tmp45E=_tmp45C.f2;_LLC:;
_tmp465->hd=(void*)_tmp45E;
_tmp44F=({struct Cyc_List_List*_tmp45D=_cycalloc(sizeof(*_tmp45D));_tmp45D->hd=_tmp45F,_tmp45D->tl=_tmp44F;_tmp45D;});
_tmp454=_tmp454->tl;};}
# 2610
goto _LL5;}}else{_LL8: _LL9:
({void*_tmp460=0U;({struct _dyneither_ptr _tmp7FC=({const char*_tmp461="not a function call!";_tag_dyneither(_tmp461,sizeof(char),21U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp7FC,_tag_dyneither(_tmp460,sizeof(void*),0U));});});}_LL5:;}
# 2613
goto _LL0;}else{_LL3: _LL4:
({void*_tmp463=0U;({struct _dyneither_ptr _tmp7FD=({const char*_tmp464="not a function call!";_tag_dyneither(_tmp464,sizeof(char),21U);});((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(_tmp7FD,_tag_dyneither(_tmp463,sizeof(void*),0U));});});}_LL0:;}
# 2618
while(_tmp44F != 0){
struct Cyc_Absyn_Decl*_tmp466=(struct Cyc_Absyn_Decl*)_tmp44F->hd;
fn_exp=({struct Cyc_Absyn_Stmt*_tmp800=({struct Cyc_Absyn_Decl*_tmp7FF=_tmp466;struct Cyc_Absyn_Stmt*_tmp7FE=Cyc_Absyn_exp_stmt(fn_exp,e->loc);Cyc_Absyn_decl_stmt(_tmp7FF,_tmp7FE,e->loc);});Cyc_Absyn_stmt_exp(_tmp800,e->loc);});
_tmp44F=_tmp44F->tl;}
# 2625
e->topt=0;
e->r=fn_exp->r;}
# 2631
static void Cyc_Tcexp_tcExpNoInst(struct Cyc_Tcenv_Tenv*te,void**topt,struct Cyc_Absyn_Exp*e){
unsigned int loc=e->loc;
void*t;
# 2635
{void*_tmp467=e->r;void*_tmp468=_tmp467;void*_tmp4D1;struct Cyc_Absyn_Exp*_tmp4D0;struct Cyc_Absyn_Exp*_tmp4CF;struct _dyneither_ptr*_tmp4CE;struct Cyc_Absyn_Exp*_tmp4CD;struct Cyc_Absyn_Exp*_tmp4CC;int*_tmp4CB;struct Cyc_Absyn_Exp**_tmp4CA;void***_tmp4C9;struct Cyc_Absyn_Exp**_tmp4C8;int*_tmp4C7;void*_tmp4C6;struct Cyc_Absyn_Enumfield*_tmp4C5;struct Cyc_Absyn_Enumdecl*_tmp4C4;struct Cyc_Absyn_Enumfield*_tmp4C3;struct Cyc_List_List*_tmp4C2;struct Cyc_Absyn_Datatypedecl*_tmp4C1;struct Cyc_Absyn_Datatypefield*_tmp4C0;void*_tmp4BF;struct Cyc_List_List*_tmp4BE;struct _tuple1**_tmp4BD;struct Cyc_List_List**_tmp4BC;struct Cyc_List_List*_tmp4BB;struct Cyc_Absyn_Aggrdecl**_tmp4BA;struct Cyc_Absyn_Exp*_tmp4B9;void*_tmp4B8;int*_tmp4B7;struct Cyc_Absyn_Vardecl*_tmp4B6;struct Cyc_Absyn_Exp*_tmp4B5;struct Cyc_Absyn_Exp*_tmp4B4;int*_tmp4B3;struct Cyc_Absyn_Stmt*_tmp4B2;struct Cyc_List_List*_tmp4B1;struct _tuple9*_tmp4B0;struct Cyc_List_List*_tmp4AF;struct Cyc_List_List*_tmp4AE;struct Cyc_Absyn_Exp*_tmp4AD;struct Cyc_Absyn_Exp*_tmp4AC;struct Cyc_Absyn_Exp*_tmp4AB;struct _dyneither_ptr*_tmp4AA;int*_tmp4A9;int*_tmp4A8;struct Cyc_Absyn_Exp*_tmp4A7;struct _dyneither_ptr*_tmp4A6;int*_tmp4A5;int*_tmp4A4;struct Cyc_Absyn_Exp*_tmp4A3;void*_tmp4A2;struct Cyc_List_List*_tmp4A1;void*_tmp4A0;struct Cyc_Absyn_Exp*_tmp49F;struct Cyc_Absyn_Exp**_tmp49E;struct Cyc_Absyn_Exp*_tmp49D;struct Cyc_Absyn_Exp*_tmp49C;void*_tmp49B;struct Cyc_Absyn_Exp*_tmp49A;enum Cyc_Absyn_Coercion*_tmp499;struct Cyc_Absyn_Exp*_tmp498;struct Cyc_List_List*_tmp497;struct Cyc_Absyn_Exp*_tmp496;struct Cyc_Absyn_Exp*_tmp495;struct Cyc_Absyn_Exp*_tmp494;struct Cyc_Absyn_Exp*_tmp493;struct Cyc_Absyn_Exp*_tmp492;struct Cyc_Absyn_Exp*_tmp491;struct Cyc_Absyn_Exp*_tmp490;struct Cyc_Absyn_Exp*_tmp48F;struct Cyc_Absyn_Exp*_tmp48E;struct Cyc_Absyn_Exp*_tmp48D;struct Cyc_Absyn_Exp*_tmp48C;struct Cyc_Core_Opt*_tmp48B;struct Cyc_Absyn_Exp*_tmp48A;struct Cyc_Absyn_Exp*_tmp489;enum Cyc_Absyn_Incrementor _tmp488;enum Cyc_Absyn_Primop _tmp487;struct Cyc_List_List*_tmp486;void**_tmp485;union Cyc_Absyn_Cnst*_tmp484;struct Cyc_Core_Opt*_tmp483;struct Cyc_List_List*_tmp482;struct Cyc_Absyn_Exp*_tmp481;struct Cyc_List_List*_tmp480;struct Cyc_Absyn_VarargCallInfo**_tmp47F;struct Cyc_Absyn_Exp*_tmp47E;struct Cyc_List_List*_tmp47D;struct Cyc_Absyn_VarargCallInfo**_tmp47C;int*_tmp47B;struct Cyc_Absyn_Exp*_tmp47A;switch(*((int*)_tmp468)){case 12U: _LL1: _tmp47A=((struct Cyc_Absyn_NoInstantiate_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LL2:
# 2640
 Cyc_Tcexp_tcExpNoInst(te,0,_tmp47A);
return;case 10U: _LL3: _tmp47E=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp47D=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_tmp47C=(struct Cyc_Absyn_VarargCallInfo**)&((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp468)->f3;_tmp47B=(int*)&((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp468)->f4;if(!(*_tmp47B)){_LL4:
# 2644
(int)_throw((void*)({struct Cyc_Core_Impossible_exn_struct*_tmp46A=_cycalloc(sizeof(*_tmp46A));_tmp46A->tag=Cyc_Core_Impossible,({struct _dyneither_ptr _tmp801=({const char*_tmp469="unresolved function in tcExpNoInst";_tag_dyneither(_tmp469,sizeof(char),35U);});_tmp46A->f1=_tmp801;});_tmp46A;}));}else{_LL1B: _tmp481=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp480=((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_tmp47F=(struct Cyc_Absyn_VarargCallInfo**)&((struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_tmp468)->f3;_LL1C: {
# 2677
struct Cyc_List_List*alias_arg_exps=0;
int ok=1;
struct Cyc_Absyn_Exp*fn_exp;
{struct _handler_cons _tmp46B;_push_handler(& _tmp46B);{int _tmp46D=0;if(setjmp(_tmp46B.handler))_tmp46D=1;if(!_tmp46D){
fn_exp=Cyc_Tcutil_deep_copy_exp(0,e);;_pop_handler();}else{void*_tmp46C=(void*)_exn_thrown;void*_tmp46E=_tmp46C;void*_tmp46F;if(((struct Cyc_Core_Failure_exn_struct*)_tmp46E)->tag == Cyc_Core_Failure){_LL58: _LL59:
# 2684
 ok=0;
fn_exp=e;
goto _LL57;}else{_LL5A: _tmp46F=_tmp46E;_LL5B:(int)_rethrow(_tmp46F);}_LL57:;}};}
# 2688
t=Cyc_Tcexp_tcFnCall(te,loc,topt,_tmp481,_tmp480,_tmp47F,& alias_arg_exps);
if(alias_arg_exps != 0  && ok){
alias_arg_exps=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(alias_arg_exps);
Cyc_Tcexp_insert_alias_stmts(te,e,fn_exp,alias_arg_exps);
Cyc_Tcexp_tcExpNoInst(te,topt,e);
return;}
# 2695
goto _LL0;}}case 36U: _LL5: _tmp483=((struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp482=((struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL6:
# 2648
 Cyc_Tcexp_resolve_unresolved_mem(te,loc,topt,e,_tmp482);
Cyc_Tcexp_tcExpNoInst(te,topt,e);
return;case 0U: _LL7: _tmp484=(union Cyc_Absyn_Cnst*)&((struct Cyc_Absyn_Const_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LL8:
# 2653
 t=Cyc_Tcexp_tcConst(te,loc,topt,_tmp484,e);goto _LL0;case 1U: _LL9: _tmp485=(void**)&((struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LLA:
# 2655
 t=Cyc_Tcexp_tcVar(te,loc,topt,e,_tmp485);goto _LL0;case 2U: _LLB: _LLC:
# 2657
 t=Cyc_Absyn_sint_type;goto _LL0;case 3U: _LLD: _tmp487=((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp486=((struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LLE:
# 2659
 t=Cyc_Tcexp_tcPrimop(te,loc,topt,_tmp487,_tmp486);goto _LL0;case 5U: _LLF: _tmp489=((struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp488=((struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL10:
# 2661
 t=Cyc_Tcexp_tcIncrement(te,loc,topt,_tmp489,_tmp488);goto _LL0;case 4U: _LL11: _tmp48C=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp48B=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_tmp48A=((struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_tmp468)->f3;_LL12:
# 2663
 t=Cyc_Tcexp_tcAssignOp(te,loc,topt,_tmp48C,_tmp48B,_tmp48A);goto _LL0;case 6U: _LL13: _tmp48F=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp48E=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_tmp48D=((struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_tmp468)->f3;_LL14:
# 2665
 t=Cyc_Tcexp_tcConditional(te,loc,topt,_tmp48F,_tmp48E,_tmp48D);goto _LL0;case 7U: _LL15: _tmp491=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp490=((struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL16:
# 2667
 t=Cyc_Tcexp_tcAnd(te,loc,_tmp491,_tmp490);goto _LL0;case 8U: _LL17: _tmp493=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp492=((struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL18:
# 2669
 t=Cyc_Tcexp_tcOr(te,loc,_tmp493,_tmp492);goto _LL0;case 9U: _LL19: _tmp495=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp494=((struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL1A:
# 2671
 t=Cyc_Tcexp_tcSeqExp(te,loc,topt,_tmp495,_tmp494);goto _LL0;case 11U: _LL1D: _tmp496=((struct Cyc_Absyn_Throw_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LL1E:
# 2697
 t=Cyc_Tcexp_tcThrow(te,loc,topt,_tmp496);goto _LL0;case 13U: _LL1F: _tmp498=((struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp497=((struct Cyc_Absyn_Instantiate_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL20:
# 2699
 t=Cyc_Tcexp_tcInstantiate(te,loc,topt,_tmp498,_tmp497);goto _LL0;case 14U: _LL21: _tmp49B=(void*)((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp49A=((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_tmp499=(enum Cyc_Absyn_Coercion*)&((struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_tmp468)->f4;_LL22:
# 2701
 t=Cyc_Tcexp_tcCast(te,loc,topt,_tmp49B,_tmp49A,_tmp499);goto _LL0;case 15U: _LL23: _tmp49C=((struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LL24:
# 2703
 t=Cyc_Tcexp_tcAddress(te,loc,e,topt,_tmp49C);goto _LL0;case 16U: _LL25: _tmp49E=(struct Cyc_Absyn_Exp**)&((struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp49D=((struct Cyc_Absyn_New_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL26:
# 2705
 t=Cyc_Tcexp_tcNew(te,loc,topt,_tmp49E,e,_tmp49D);goto _LL0;case 18U: _LL27: _tmp49F=((struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LL28: {
# 2707
void*_tmp470=Cyc_Tcexp_tcExpNoPromote(te,0,_tmp49F);
t=Cyc_Tcexp_tcSizeof(te,loc,topt,_tmp470);goto _LL0;}case 17U: _LL29: _tmp4A0=(void*)((struct Cyc_Absyn_Sizeoftype_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LL2A:
# 2710
 t=Cyc_Tcexp_tcSizeof(te,loc,topt,_tmp4A0);goto _LL0;case 19U: _LL2B: _tmp4A2=(void*)((struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4A1=((struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL2C:
# 2712
 t=Cyc_Tcexp_tcOffsetof(te,loc,topt,_tmp4A2,_tmp4A1);goto _LL0;case 20U: _LL2D: _tmp4A3=((struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LL2E:
# 2714
 t=Cyc_Tcexp_tcDeref(te,loc,topt,_tmp4A3);goto _LL0;case 21U: _LL2F: _tmp4A7=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4A6=((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_tmp4A5=(int*)&((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp468)->f3;_tmp4A4=(int*)&((struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_tmp468)->f4;_LL30:
# 2716
 t=Cyc_Tcexp_tcAggrMember(te,loc,topt,e,_tmp4A7,_tmp4A6,_tmp4A5,_tmp4A4);goto _LL0;case 22U: _LL31: _tmp4AB=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4AA=((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_tmp4A9=(int*)&((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp468)->f3;_tmp4A8=(int*)&((struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_tmp468)->f4;_LL32:
# 2718
 t=Cyc_Tcexp_tcAggrArrow(te,loc,topt,_tmp4AB,_tmp4AA,_tmp4A9,_tmp4A8);goto _LL0;case 23U: _LL33: _tmp4AD=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4AC=((struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL34:
# 2720
 t=Cyc_Tcexp_tcSubscript(te,loc,topt,_tmp4AD,_tmp4AC);goto _LL0;case 24U: _LL35: _tmp4AE=((struct Cyc_Absyn_Tuple_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LL36:
# 2722
 t=Cyc_Tcexp_tcTuple(te,loc,topt,_tmp4AE);goto _LL0;case 25U: _LL37: _tmp4B0=((struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4AF=((struct Cyc_Absyn_CompoundLit_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL38:
# 2724
 t=Cyc_Tcexp_tcCompoundLit(te,loc,topt,_tmp4B0,_tmp4AF);goto _LL0;case 26U: _LL39: _tmp4B1=((struct Cyc_Absyn_Array_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LL3A: {
# 2728
void**elt_topt=0;
struct Cyc_Absyn_Tqual*elt_tqopt=0;
int zero_term=0;
if(topt != 0){
void*_tmp471=Cyc_Tcutil_compress(*topt);void*_tmp472=_tmp471;void**_tmp475;struct Cyc_Absyn_Tqual*_tmp474;void*_tmp473;if(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp472)->tag == 4U){_LL5D: _tmp475=(void**)&(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp472)->f1).elt_type;_tmp474=(struct Cyc_Absyn_Tqual*)&(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp472)->f1).tq;_tmp473=(((struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_tmp472)->f1).zero_term;_LL5E:
# 2734
 elt_topt=_tmp475;
elt_tqopt=_tmp474;
zero_term=Cyc_Tcutil_force_type2bool(0,_tmp473);
goto _LL5C;}else{_LL5F: _LL60:
 goto _LL5C;}_LL5C:;}
# 2741
t=Cyc_Tcexp_tcArray(te,loc,elt_topt,elt_tqopt,zero_term,_tmp4B1);goto _LL0;}case 37U: _LL3B: _tmp4B2=((struct Cyc_Absyn_StmtExp_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LL3C:
# 2743
 t=Cyc_Tcexp_tcStmtExp(te,loc,topt,_tmp4B2);goto _LL0;case 27U: _LL3D: _tmp4B6=((struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4B5=((struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_tmp4B4=((struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct*)_tmp468)->f3;_tmp4B3=(int*)&((struct Cyc_Absyn_Comprehension_e_Absyn_Raw_exp_struct*)_tmp468)->f4;_LL3E:
# 2745
 t=Cyc_Tcexp_tcComprehension(te,loc,topt,_tmp4B6,_tmp4B5,_tmp4B4,_tmp4B3);goto _LL0;case 28U: _LL3F: _tmp4B9=((struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4B8=(void*)((struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_tmp4B7=(int*)&((struct Cyc_Absyn_ComprehensionNoinit_e_Absyn_Raw_exp_struct*)_tmp468)->f3;_LL40:
# 2747
 t=Cyc_Tcexp_tcComprehensionNoinit(te,loc,topt,_tmp4B9,_tmp4B8,_tmp4B7);goto _LL0;case 29U: _LL41: _tmp4BD=(struct _tuple1**)&((struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4BC=(struct Cyc_List_List**)&((struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_tmp4BB=((struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*)_tmp468)->f3;_tmp4BA=(struct Cyc_Absyn_Aggrdecl**)&((struct Cyc_Absyn_Aggregate_e_Absyn_Raw_exp_struct*)_tmp468)->f4;_LL42:
# 2749
 t=Cyc_Tcexp_tcAggregate(te,loc,topt,_tmp4BD,_tmp4BC,_tmp4BB,_tmp4BA);goto _LL0;case 30U: _LL43: _tmp4BF=(void*)((struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4BE=((struct Cyc_Absyn_AnonStruct_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL44:
# 2751
 t=Cyc_Tcexp_tcAnonStruct(te,loc,_tmp4BF,_tmp4BE);goto _LL0;case 31U: _LL45: _tmp4C2=((struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4C1=((struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_tmp4C0=((struct Cyc_Absyn_Datatype_e_Absyn_Raw_exp_struct*)_tmp468)->f3;_LL46:
# 2753
 t=Cyc_Tcexp_tcDatatype(te,loc,topt,e,_tmp4C2,_tmp4C1,_tmp4C0);goto _LL0;case 32U: _LL47: _tmp4C4=((struct Cyc_Absyn_Enum_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4C3=((struct Cyc_Absyn_Enum_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL48:
# 2755
 t=Cyc_Absyn_enum_type(_tmp4C4->name,_tmp4C4);goto _LL0;case 33U: _LL49: _tmp4C6=(void*)((struct Cyc_Absyn_AnonEnum_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4C5=((struct Cyc_Absyn_AnonEnum_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL4A:
# 2757
 t=_tmp4C6;goto _LL0;case 34U: _LL4B: _tmp4CB=(int*)&(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp468)->f1).is_calloc;_tmp4CA=(struct Cyc_Absyn_Exp**)&(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp468)->f1).rgn;_tmp4C9=(void***)&(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp468)->f1).elt_type;_tmp4C8=(struct Cyc_Absyn_Exp**)&(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp468)->f1).num_elts;_tmp4C7=(int*)&(((struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_tmp468)->f1).fat_result;_LL4C:
# 2759
 t=Cyc_Tcexp_tcMalloc(te,loc,topt,_tmp4CA,_tmp4C9,_tmp4C8,_tmp4CB,_tmp4C7);goto _LL0;case 35U: _LL4D: _tmp4CD=((struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4CC=((struct Cyc_Absyn_Swap_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL4E:
# 2761
 t=Cyc_Tcexp_tcSwap(te,loc,topt,_tmp4CD,_tmp4CC);goto _LL0;case 38U: _LL4F: _tmp4CF=((struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_tmp4CE=((struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct*)_tmp468)->f2;_LL50:
# 2763
 t=Cyc_Tcexp_tcTagcheck(te,loc,topt,_tmp4CF,_tmp4CE);goto _LL0;case 41U: _LL51: _tmp4D0=((struct Cyc_Absyn_Extension_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LL52:
# 2765
 t=Cyc_Tcexp_tcExp(te,topt,_tmp4D0);goto _LL0;case 39U: _LL53: _tmp4D1=(void*)((struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct*)_tmp468)->f1;_LL54:
# 2767
 if(!te->allow_valueof)
({void*_tmp476=0U;({unsigned int _tmp803=e->loc;struct _dyneither_ptr _tmp802=({const char*_tmp477="valueof(-) can only occur within types";_tag_dyneither(_tmp477,sizeof(char),39U);});Cyc_Tcutil_terr(_tmp803,_tmp802,_tag_dyneither(_tmp476,sizeof(void*),0U));});});
# 2775
t=Cyc_Absyn_sint_type;
goto _LL0;default: _LL55: _LL56:
# 2778
({void*_tmp478=0U;({unsigned int _tmp805=e->loc;struct _dyneither_ptr _tmp804=({const char*_tmp479="asm expressions cannot occur within Cyclone code.";_tag_dyneither(_tmp479,sizeof(char),50U);});Cyc_Tcutil_terr(_tmp805,_tmp804,_tag_dyneither(_tmp478,sizeof(void*),0U));});});
t=Cyc_Absyn_wildtyp(Cyc_Tcenv_lookup_opt_type_vars(te));}_LL0:;}
# 2781
e->topt=t;}
