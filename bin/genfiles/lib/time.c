 struct Cyc_timeval{ int tv_sec; int tv_usec; } ; struct Cyc_timespec{
unsigned int tv_sec; int tv_nsec; } ; struct Cyc_itimerspec{ struct Cyc_timespec
it_interval; struct Cyc_timespec it_value; } ; struct Cyc__types_fd_set{ int
fds_bits[ 2u]; } ; extern void exit( int); extern void* abort(); struct Cyc_Core_Opt{
void* v; } ; extern unsigned char Cyc_Core_InvalidArg[ 15u]; struct Cyc_Core_InvalidArg_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern unsigned char Cyc_Core_Failure[
12u]; struct Cyc_Core_Failure_struct{ unsigned char* tag; struct _tagged_arr f1;
} ; extern unsigned char Cyc_Core_Impossible[ 15u]; struct Cyc_Core_Impossible_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern unsigned char Cyc_Core_Not_found[
14u]; extern unsigned char Cyc_Core_Unreachable[ 16u]; struct Cyc_Core_Unreachable_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern unsigned char*
string_to_Cstring( struct _tagged_arr); extern unsigned char* underlying_Cstring(
struct _tagged_arr); extern struct _tagged_arr Cstring_to_string( unsigned char*);
extern struct _tagged_arr wrap_Cstring_as_string( unsigned char*, int); extern
struct _tagged_arr ntCsl_to_ntsl( unsigned char**); extern int system(
unsigned char*); extern int* __errno(); struct Cyc_Time_tm{ int tm_sec; int
tm_min; int tm_hour; int tm_mday; int tm_mon; int tm_year; int tm_wday; int
tm_yday; int tm_isdst; } ; extern unsigned int time( unsigned int* t); extern
struct _tagged_arr Cyc_Time_asctime( const struct Cyc_Time_tm* _tblock); extern
struct _tagged_arr Cyc_Time_ctime( const unsigned int* _time); extern int Cyc_Time_strftime(
struct _tagged_arr _s, int _maxsize, struct _tagged_arr _fmt, const struct Cyc_Time_tm*
_t); extern struct _tagged_arr Cyc_Time_asctime_r( const struct Cyc_Time_tm*,
struct _tagged_arr); extern struct _tagged_arr Cyc_Time_ctime_r( const
unsigned int*, struct _tagged_arr); extern unsigned char* asctime( const struct
Cyc_Time_tm* _tblock); extern unsigned char* ctime( const unsigned int* _time);
extern int strftime( unsigned char* _s, int _maxsize, unsigned char* _fmt, const
struct Cyc_Time_tm* _t); extern unsigned char* asctime_r( const struct Cyc_Time_tm*,
unsigned char*); extern unsigned char* ctime_r( const unsigned int*,
unsigned char*); struct _tagged_arr Cyc_Time_asctime( const struct Cyc_Time_tm*
_tblock){ return Cstring_to_string( asctime( _tblock));} struct _tagged_arr Cyc_Time_ctime(
const unsigned int* _time){ return Cstring_to_string( ctime( _time));} int Cyc_Time_strftime(
struct _tagged_arr _s, int _maxsize, struct _tagged_arr _fmt, const struct Cyc_Time_tm*
_t){ int m=( int)( _get_arr_size( _s, sizeof( unsigned char)) < _maxsize?
_get_arr_size( _s, sizeof( unsigned char)):( unsigned int) _maxsize); return
strftime( underlying_Cstring( _s), m, underlying_Cstring( _fmt), _t);} struct
_tagged_arr Cyc_Time_asctime_r( const struct Cyc_Time_tm* t, struct _tagged_arr
s){ if( _get_arr_size( s, sizeof( unsigned char)) < 50){( int) _throw(( void*)({
struct Cyc_Core_InvalidArg_struct* _temp0=( struct Cyc_Core_InvalidArg_struct*)
GC_malloc( sizeof( struct Cyc_Core_InvalidArg_struct)); _temp0[ 0]=({ struct Cyc_Core_InvalidArg_struct
_temp1; _temp1.tag= Cyc_Core_InvalidArg; _temp1.f1= _tag_arr("Time::asctime_r: string too small (< 50)",
sizeof( unsigned char), 41u); _temp1;}); _temp0;}));} return Cstring_to_string(
asctime_r( t, underlying_Cstring(( struct _tagged_arr) s)));} struct _tagged_arr
Cyc_Time_ctime_r( const unsigned int* t, struct _tagged_arr s){ if(
_get_arr_size( s, sizeof( unsigned char)) < 50){( int) _throw(( void*)({ struct
Cyc_Core_InvalidArg_struct* _temp2=( struct Cyc_Core_InvalidArg_struct*)
GC_malloc( sizeof( struct Cyc_Core_InvalidArg_struct)); _temp2[ 0]=({ struct Cyc_Core_InvalidArg_struct
_temp3; _temp3.tag= Cyc_Core_InvalidArg; _temp3.f1= _tag_arr("Time::ctime_r: string too small (< 50)",
sizeof( unsigned char), 39u); _temp3;}); _temp2;}));} return Cstring_to_string(
ctime_r( t, underlying_Cstring(( struct _tagged_arr) s)));}