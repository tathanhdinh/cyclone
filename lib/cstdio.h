/* This file is part of the Cyclone Library.
   Copyright (C) 2001 Greg Morrisett, AT&T

   This library is free software; you can redistribute it and/or it
   under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place, Suite
   330, Boston, MA 02111-1307 USA. */

/* Some definitions taken from the GNU C Library, released under LGPL:
   Copyright (C) 1991-1999, 2000 Free Software Foundation, Inc. */

/*
 *	ISO C99 Standard: 7.19 Input/output	<stdio.h>
 */

#ifndef _STDIO_H_
#define _STDIO_H_
#include <core.h>
namespace std {
using Core;

/* The name __sFILE is derived from Cygwin but we have our own
   implementation in runtime_cyc.c */
extern struct __sFILE;  
typedef struct __sFILE FILE;

/* Standard streams.  */
extern FILE @stdout;
extern FILE @stdin;
extern FILE @stderr;

/* The type of the second argument to `fgetpos' and `fsetpos'.  */
typedef long fpos_t;  //FIX: __off_t in linux

/* The possibilities for the third argument to `setvbuf'.  */
#define	_IOFBF 0  /* Full buffering.  */
#define	_IOLBF 1  /* Line buffering.  */
#define	_IONBF 2  /* No buffering.  */

/* Default buffer size.  */
#define	BUFSIZ	1024

/* End of file character.
   Some things throughout the library rely on this being -1.  */
#define	EOF	(-1)

/* CAREFUL: the constants TMP_MAX FOPEN_MAX FILENAME_MAX
   L_tmpnam L_cuserid L_ctermid
   are platform-dependent, and some can lead to buffer overflows
   if incorrect */
#ifdef __CYGWIN__
/* Cygwin */
#define TMP_MAX 26
#define FOPEN_MAX 20
#define FILENAME_MAX 1024
#define L_tmpnam FILENAME_MAX
#define L_cuserid 9
#define L_ctermid 16
#else
/* Linux */
#define TMP_MAX 238328
#define FOPEN_MAX 16
#define FILENAME_MAX 4095
#define L_tmpnam 20
#define L_cuserid 9
#define L_ctermid 9
#endif


/* The possibilities for the third argument to `fseek'.
   These values should not be changed.  */
#define	SEEK_SET	0	/* Seek from beginning of file.  */
#define	SEEK_CUR	1	/* Seek from current position.  */
#define	SEEK_END	2	/* Seek from end of file.  */

/* Remove file FILENAME.  */
extern int remove (const char ?);
/* Rename file OLD to NEW.  */
extern int rename (const char ?, const char ?);

/* Create a temporary file and open it read/write.  */
extern FILE *tmpfile ();
/* Generate a temporary filename.  */
extern char ?`H tmpnam (char ?`H);

/* Close STREAM.  */
extern int fclose (FILE @);
/* Flush STREAM, or all streams if STREAM is NULL.  */
extern int fflush (FILE *);

/* Open a file and create a new stream for it.  */
extern FILE *fopen (const char ?__filename, const char ?__modes);
/* Open a file, replacing an existing stream with it. */
extern FILE *freopen (const char ?, const char ?, FILE @);

/* If BUF is NULL, make STREAM unbuffered.
   Else make it use buffer BUF, of size BUFSIZ.  */
extern void setbuf (FILE @ __stream, char ? __buf);
/* Make STREAM use buffering mode MODE.
   If BUF is not NULL, use N bytes of it for buffering;
   else allocate an internal buffer N bytes long.  */
extern int setvbuf (FILE @ __stream, char ? __buf,
		    int __modes, size_t __n);

/* Read a character from STREAM.  */
extern int fgetc (FILE @__stream);
extern int getc (FILE @__stream);

/* Read a character from stdin.  */
  //FIX: not much use doing this because getc is not a macro
#define	getchar() getc(stdin)

/* Get a newline-terminated string from stdin, removing the newline.
   DO NOT USE THIS FUNCTION!!  There is no limit on how much it will read.  */
// extern string     gets(string); // unsafe!

/* Get a newline-terminated string of finite length from STREAM.  */
extern char ?`r fgets (char ?`r __s, int __n, FILE @ __stream);

/* Write a character to STREAM.  */
extern int fputc (int __c, FILE @__stream);
extern int putc (int __c, FILE @__stream);

/* Write a character to stdout.  */
  //FIX: not much use doing this because putc is not a macro, no speedup
#define	putchar(__c) putc(__c, stdout)

/* Write a string to STREAM.  */
extern int fputs (const char ? __s, FILE @ __stream);
/* Write a string, followed by a newline, to stdout.  */
extern int puts (const char ? __s);

/* Push a character back onto the input buffer of STREAM.  */
extern int ungetc (int __c, FILE @ __stream);

/* Read chunks of generic data from STREAM.  */
extern size_t fread (char ? __ptr, size_t __size,
		     size_t __n, FILE @ __stream);
/* Write chunks of generic data to STREAM.  */
extern size_t fwrite (const char ? __ptr, size_t __size,
		      size_t __n, FILE @ __s);

/* Seek to a certain position on STREAM.  */
extern int fseek (FILE @__stream, long __off, int __whence);
/* Return the current position of STREAM.  */
extern long ftell (FILE @__stream);
/* Rewind to the beginning of STREAM.  */
extern void rewind (FILE @__stream);

/* Get STREAM's position.  */
extern int fgetpos (FILE @__stream, fpos_t @ __pos);
/* Set STREAM's position.  */
extern int fsetpos (FILE @__stream, fpos_t @ __pos);

/* Clear the error and EOF indicators for STREAM.  */
extern void clearerr (FILE @__stream);
/* Return the EOF indicator for STREAM.  */
extern int feof (FILE @__stream);
/* Return the error indicator for STREAM.  */
extern int ferror (FILE @__stream);

/* Print a message describing the meaning of the value of errno.  */
extern void perror (const char ?__s);

/* Create a new stream that refers to an existing system file descriptor.  */
extern FILE *fdopen (int __fd, const char ?__modes);

/* Return the system file descriptor for STREAM.  */
extern int fileno (FILE @__stream);

/* Get a word (int) from STREAM.  */
extern int getw (FILE @__stream);

/* Write a word (int) to STREAM.  */
extern int putw (int __w, FILE @__stream);

  //FIX: setbuffer and setlinebuf only if __USE_BSD
/* If BUF is NULL, make STREAM unbuffered.
   Else make it use SIZE bytes of BUF for buffering.  */
extern void setbuffer (FILE @__stream, char ?__buf, size_t __size);
/* Make STREAM line-buffered.  */
extern void setlinebuf (FILE @__stream);

// 
// Routines added for Cyclone
//
extern xtunion exn {
  extern FileOpenError(const char ?);
  extern FileCloseError;
};

extern FILE @file_open(const char ?`r1 fname, const char ?`r2 mode);
extern void file_close(FILE @`r);
extern void file_delete(const char ?`r);
extern void file_length(const char ?`r);
// these two provided in cyc_runtime.c
extern int file_string_read(FILE @`r1 fd, char ?`r2 dest, int dest_offset, 
			    int max_count);
extern int file_string_write(FILE @`r1 fd, const char?`r2 src, int src_offset, 
			     int max_count);

//////////////////////////////////////////////////////////////
// printf and friends:  see printf.cyc
//////////////////////////////////////////////////////////////
// vararg union for printf, fprintf, sprintf 
extern tunion PrintArg<`r::R> {
  String_pa(const char ?`r);
  Int_pa(unsigned long);
  Double_pa(double);
  ShortPtr_pa(short @`r);
  IntPtr_pa(unsigned long @`r);
};
typedef tunion `r PrintArg<`r> parg_t<`r>;

// Printing functions behave as with C
extern int fprintf(FILE @`r1,const char ?`r2 fmt, ...`r3 inject parg_t<`r4>)
  __attribute__((format(printf,2,3)))
  ;
extern int printf(const char ?`r fmt, ...`r1 inject parg_t<`r2>)
  __attribute__((format(printf,1,2)))
  ;
extern int sprintf(char ?`r1 s, const char ?`r2 fmt, ...`r3 inject parg_t<`r4>)
  __attribute__((format(printf,2,3)))
  ;
// Similar to sprintf but allocates a result of the right size
extern char ? aprintf(const char ?`r2 fmt, ...`r3 inject parg_t<`r4>)
  __attribute__((format(printf,1,2)))
  ;
extern char ?`r1 raprintf(region_t<`r1>, const char ?`r2 fmt, 
                          ...`r3 inject parg_t<`r4> ap)
  __attribute__((format(printf,2,3)))
  ;

// Same as above but suitable for calling from a user's vararg function
extern int vfprintf(FILE @`r1,const char ?`r2 fmt, parg_t<`r3> ? `r4 ap)
  __attribute__((format(printf,2,0)))
  ;
extern int vprintf(const char ?`r fmt, parg_t<`r2> ? `r1)
  __attribute__((format(printf,1,0)))
  ;
extern int vsprintf(char ?`r1 s, const char ?`r2 fmt, parg_t<`r4> ? `r3)
  __attribute__((format(printf,2,0)))
  ;
extern char ?`r1 vraprintf(region_t<`r1> r1, const char ?`r2 fmt, 
                           parg_t<`r4> ? `r3 ap)
  __attribute__((format(printf,2,0)))
  ;

//////////////////////////////////////////////////////////////
// scanf and friends:  see scanf.cyc
//////////////////////////////////////////////////////////////
// vararg tunion for scanf, fscanf, sscanf, etc.
extern tunion ScanfArg<`r::R> {
  ShortPtr_sa(short @`r);
  UShortPtr_sa(unsigned short @`r);
  IntPtr_sa(int @`r);
  UIntPtr_sa(unsigned int @`r);
  StringPtr_sa(char ?`r);
  DoublePtr_sa(double @`r);
  FloatPtr_sa(float @`r);
};
typedef tunion `r2 ScanfArg<`r1> sarg_t<`r1,`r2>;

// Scanning functions behave as in C...
extern int scanf(const char ?`r1 fmt, ...`r2 inject sarg_t<`r3,`r4>)
  __attribute__((format(scanf,1,2)))
  ;
extern int fscanf(FILE @`r1 stream, const char ?`r2 fmt, 
                     ...`r3 inject sarg_t<`r4,`r5>)
  __attribute__((format(scanf,2,3)))
  ;
extern int sscanf(const char ?`r src, const char ?`r1 fmt, 
                     ...`r2 inject sarg_t<`r3,`r4>)
  __attribute__((format(scanf,2,3)))
  ;


// Same as above but suitable for calling from a user's vararg function
extern int vfscanf(FILE @`r1 stream, const char ?`r2 fmt, 
                   sarg_t<`r3,`r4> ? `r5)
  __attribute__((format(scanf,2,0)))
  ;
extern int vsscanf(const char ?`r src, const char ?`r1 fmt, 
                   sarg_t<`r3,`r4> ? `r2)
  __attribute__((format(scanf,2,0)))
  ;

}

#endif /* stdio.h  */