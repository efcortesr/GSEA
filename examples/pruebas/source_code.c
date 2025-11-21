/* Define ISO C stdio on top of C++ iostreams.
   Copyright (C) 1991-2024 Free Software Foundation, Inc.
   Copyright The GNU Toolchain Authors.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	ISO C99 Standard: 7.19 Input/output	<stdio.h>
 */

#ifndef _STDIO_H
#define _STDIO_H	1

#define __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION
#include <bits/libc-header-start.h>

__BEGIN_DECLS

#define __need_size_t
#define __need_NULL
#include <stddef.h>

#define __need___va_list
#include <stdarg.h>

#include <bits/types.h>
#include <bits/types/__fpos_t.h>
#include <bits/types/__fpos64_t.h>
#include <bits/types/__FILE.h>
#include <bits/types/FILE.h>
#include <bits/types/struct_FILE.h>

#ifdef __USE_MISC
# include <bits/types/cookie_io_functions_t.h>
#endif

#if defined __USE_XOPEN || defined __USE_XOPEN2K8
# ifdef __GNUC__
#  ifndef _VA_LIST_DEFINED
typedef __gnuc_va_list va_list;
#   define _VA_LIST_DEFINED
#  endif
# else
#  include <stdarg.h>
# endif
#endif

#if defined __USE_UNIX98 || defined __USE_XOPEN2K
# ifndef __off_t_defined
# ifndef __USE_FILE_OFFSET64
typedef __off_t off_t;
# else
typedef __off64_t off_t;
# endif
# define __off_t_defined
# endif
# if defined __USE_LARGEFILE64 && !defined __off64_t_defined
typedef __off64_t off64_t;
# define __off64_t_defined
# endif
#endif

#ifdef __USE_XOPEN2K8
# ifndef __ssize_t_defined
typedef __ssize_t ssize_t;
# define __ssize_t_defined
# endif
#endif

/* The type of the second argument to `fgetpos' and `fsetpos'.  */
#ifndef __USE_FILE_OFFSET64
typedef __fpos_t fpos_t;
#else
typedef __fpos64_t fpos_t;
#endif
#ifdef __USE_LARGEFILE64
typedef __fpos64_t fpos64_t;
#endif

/* The possibilities for the third argument to `setvbuf'.  */
#define _IOFBF 0		/* Fully buffered.  */
#define _IOLBF 1		/* Line buffered.  */
#define _IONBF 2		/* No buffering.  */


/* Default buffer size.  */
#define BUFSIZ 8192


/* The value returned by fgetc and similar functions to indicate the
   end of the file.  */
#define EOF (-1)


/* The possibilities for the third argument to `fseek'.
   These values should not be changed.  */
#define SEEK_SET	0	/* Seek from beginning of file.  */
#define SEEK_CUR	1	/* Seek from current position.  */
#define SEEK_END	2	/* Seek from end of file.  */
#ifdef __USE_GNU
# define SEEK_DATA	3	/* Seek to next data.  */
# define SEEK_HOLE	4	/* Seek to next hole.  */
#endif


#if defined __USE_MISC || defined __USE_XOPEN
/* Default path prefix for `tempnam' and `tmpnam'.  */
# define P_tmpdir	"/tmp"
#endif

#define L_tmpnam 20
#define TMP_MAX 238328

/* Get the values:
   FILENAME_MAX	Maximum length of a filename.  */
#include <bits/stdio_lim.h>

#ifdef __USE_POSIX
# define L_ctermid 9
# if !defined __USE_XOPEN2K || defined __USE_GNU
#  define L_cuserid 9
# endif
#endif

#undef  FOPEN_MAX
#define FOPEN_MAX 16


#if __GLIBC_USE (ISOC2X)
/* Maximum length of printf output for a NaN.  */
# define _PRINTF_NAN_LEN_MAX 4
#endif


/* Standard streams.  */
extern FILE *stdin;		/* Standard input stream.  */
extern FILE *stdout;		/* Standard output stream.  */
extern FILE *stderr;		/* Standard error output stream.  */
/* C89/C99 say they're macros.  Make them happy.  */
#define stdin stdin
#define stdout stdout
#define stderr stderr

/* Remove file FILENAME.  */
extern int remove (const char *__filename) __THROW;
/* Rename file OLD to NEW.  */
extern int rename (const char *__old, const char *__new) __THROW;

#ifdef __USE_ATFILE
/* Rename file OLD relative to OLDFD to NEW relative to NEWFD.  */
extern int renameat (int __oldfd, const char *__old, int __newfd,
		     const char *__new) __THROW;
#endif

#ifdef __USE_GNU
/* Flags for renameat2.  */
# define RENAME_NOREPLACE (1 << 0)
# define RENAME_EXCHANGE (1 << 1)
# define RENAME_WHITEOUT (1 << 2)

/* Rename file OLD relative to OLDFD to NEW relative to NEWFD, with
   additional flags.  */
extern int renameat2 (int __oldfd, const char *__old, int __newfd,
		      const char *__new, unsigned int __flags) __THROW;
#endif

/* Close STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fclose (FILE *__stream) __nonnull ((1));

#undef __attr_dealloc_fclose
#define __attr_dealloc_fclose __attr_dealloc (fclose, 1)

/* Create a temporary file and open it read/write.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
#ifndef __USE_FILE_OFFSET64
extern FILE *tmpfile (void)
  __attribute_malloc__ __attr_dealloc_fclose __wur;
#else
# ifdef __REDIRECT
extern FILE *__REDIRECT (tmpfile, (void), tmpfile64)
  __attribute_malloc__ __attr_dealloc_fclose __wur;
# else
#  define tmpfile tmpfile64
# endif
#endif

#ifdef __USE_LARGEFILE64
extern FILE *tmpfile64 (void)
   __attribute_malloc__ __attr_dealloc_fclose __wur;
#endif

/* Generate a temporary filename.  */
extern char *tmpnam (char[L_tmpnam]) __THROW __wur;

#ifdef __USE_MISC
/* This is the reentrant variant of `tmpnam'.  The only difference is
   that it does not allow S to be NULL.  */
extern char *tmpnam_r (char __s[L_tmpnam]) __THROW __wur;
#endif


#if defined __USE_MISC || defined __USE_XOPEN
/* Generate a unique temporary filename using up to five characters of PFX
   if it is not NULL.  The directory to put this file in is searched for
   as follows: First the environment variable "TMPDIR" is checked.
   If it contains the name of a writable directory, that directory is used.
   If not and if DIR is not NULL, that value is checked.  If that fails,
   P_tmpdir is tried and finally "/tmp".  The storage for the filename
   is allocated by `malloc'.  */
extern char *tempnam (const char *__dir, const char *__pfx)
   __THROW __attribute_malloc__ __wur __attr_dealloc_free;
#endif

/* Flush STREAM, or all streams if STREAM is NULL.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fflush (FILE *__stream);

#ifdef __USE_MISC
/* Faster versions when locking is not required.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern int fflush_unlocked (FILE *__stream);
#endif

#ifdef __USE_GNU
/* Close all streams.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern int fcloseall (void);
#endif


#ifndef __USE_FILE_OFFSET64
/* Open a file and create a new stream for it.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern FILE *fopen (const char *__restrict __filename,
		    const char *__restrict __modes)
  __attribute_malloc__ __attr_dealloc_fclose __wur;
/* Open a file, replacing an existing stream with it.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern FILE *freopen (const char *__restrict __filename,
		      const char *__restrict __modes,
		      FILE *__restrict __stream) __wur __nonnull ((3));
#else
# ifdef __REDIRECT
extern FILE *__REDIRECT (fopen, (const char *__restrict __filename,
				 const char *__restrict __modes), fopen64)
  __attribute_malloc__ __attr_dealloc_fclose __wur;
extern FILE *__REDIRECT (freopen, (const char *__restrict __filename,
				   const char *__restrict __modes,
				   FILE *__restrict __stream), freopen64)
  __wur __nonnull ((3));
# else
#  define fopen fopen64
#  define freopen freopen64
# endif
#endif
#ifdef __USE_LARGEFILE64
extern FILE *fopen64 (const char *__restrict __filename,
		      const char *__restrict __modes)
  __attribute_malloc__ __attr_dealloc_fclose __wur;
extern FILE *freopen64 (const char *__restrict __filename,
			const char *__restrict __modes,
			FILE *__restrict __stream) __wur __nonnull ((3));
#endif

#ifdef	__USE_POSIX
/* Create a new stream that refers to an existing system file descriptor.  */
extern FILE *fdopen (int __fd, const char *__modes) __THROW
  __attribute_malloc__ __attr_dealloc_fclose __wur;
#endif

#ifdef	__USE_MISC
/* Create a new stream that refers to the given magic cookie,
   and uses the given functions for input and output.  */
extern FILE *fopencookie (void *__restrict __magic_cookie,
			  const char *__restrict __modes,
			  cookie_io_functions_t __io_funcs) __THROW
  __attribute_malloc__ __attr_dealloc_fclose __wur;
#endif

#if defined __USE_XOPEN2K8 || __GLIBC_USE (LIB_EXT2)
/* Create a new stream that refers to a memory buffer.  */
extern FILE *fmemopen (void *__s, size_t __len, const char *__modes)
  __THROW __attribute_malloc__ __attr_dealloc_fclose __wur;

/* Open a stream that writes into a malloc'd buffer that is expanded as
   necessary.  *BUFLOC and *SIZELOC are updated with the buffer's location
   and the number of characters written on fflush or fclose.  */
extern FILE *open_memstream (char **__bufloc, size_t *__sizeloc) __THROW
  __attribute_malloc__ __attr_dealloc_fclose __wur;

#ifdef _WCHAR_H
/* Like OPEN_MEMSTREAM, but the stream is wide oriented and produces
   a wide character string.  Declared here only to add attribute malloc
   and only if <wchar.h> has been previously #included.  */
extern __FILE *open_wmemstream (wchar_t **__bufloc, size_t *__sizeloc) __THROW
  __attribute_malloc__ __attr_dealloc_fclose;
# endif
#endif

/* If BUF is NULL, make STREAM unbuffered.
   Else make it use buffer BUF, of size BUFSIZ.  */
extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __THROW
  __nonnull ((1));
/* Make STREAM use buffering mode MODE.
   If BUF is not NULL, use N bytes of it for buffering;
   else allocate an internal buffer N bytes long.  */
extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
		    int __modes, size_t __n) __THROW __nonnull ((1));

#ifdef	__USE_MISC
/* If BUF is NULL, make STREAM unbuffered.
   Else make it use SIZE bytes of BUF for buffering.  */
extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
		       size_t __size) __THROW __nonnull ((1));

/* Make STREAM line-buffered.  */
extern void setlinebuf (FILE *__stream) __THROW __nonnull ((1));
#endif


/* Write formatted output to STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fprintf (FILE *__restrict __stream,
		    const char *__restrict __format, ...) __nonnull ((1));
/* Write formatted output to stdout.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int printf (const char *__restrict __format, ...);
/* Write formatted output to S.  */
extern int sprintf (char *__restrict __s,
		    const char *__restrict __format, ...) __THROWNL;

/* Write formatted output to S from argument list ARG.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int vfprintf (FILE *__restrict __s, const char *__restrict __format,
		     __gnuc_va_list __arg) __nonnull ((1));
/* Write formatted output to stdout from argument list ARG.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int vprintf (const char *__restrict __format, __gnuc_va_list __arg);
/* Write formatted output to S from argument list ARG.  */
extern int vsprintf (char *__restrict __s, const char *__restrict __format,
		     __gnuc_va_list __arg) __THROWNL;

#if defined __USE_ISOC99 || defined __USE_UNIX98
/* Maximum chars of output to write in MAXLEN.  */
extern int snprintf (char *__restrict __s, size_t __maxlen,
		     const char *__restrict __format, ...)
     __THROWNL __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
		      const char *__restrict __format, __gnuc_va_list __arg)
     __THROWNL __attribute__ ((__format__ (__printf__, 3, 0)));
#endif

#if defined (__USE_MISC) || __GLIBC_USE (LIB_EXT2)
/* Write formatted output to a string dynamically allocated with `malloc'.
   Store the address of the string in *PTR.  */
extern int vasprintf (char **__restrict __ptr, const char *__restrict __f,
		      __gnuc_va_list __arg)
     __THROWNL __attribute__ ((__format__ (__printf__, 2, 0))) __wur;
extern int __asprintf (char **__restrict __ptr,
		       const char *__restrict __fmt, ...)
     __THROWNL __attribute__ ((__format__ (__printf__, 2, 3))) __wur;
extern int asprintf (char **__restrict __ptr,
		     const char *__restrict __fmt, ...)
     __THROWNL __attribute__ ((__format__ (__printf__, 2, 3))) __wur;
#endif

#ifdef __USE_XOPEN2K8
/* Write formatted output to a file descriptor.  */
extern int vdprintf (int __fd, const char *__restrict __fmt,
		     __gnuc_va_list __arg)
     __attribute__ ((__format__ (__printf__, 2, 0)));
extern int dprintf (int __fd, const char *__restrict __fmt, ...)
     __attribute__ ((__format__ (__printf__, 2, 3)));
#endif


/* Read formatted input from STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fscanf (FILE *__restrict __stream,
		   const char *__restrict __format, ...) __wur __nonnull ((1));
/* Read formatted input from stdin.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int scanf (const char *__restrict __format, ...) __wur;
/* Read formatted input from S.  */
extern int sscanf (const char *__restrict __s,
		   const char *__restrict __format, ...) __THROW;

/* For historical reasons, the C99-compliant versions of the scanf
   functions are at alternative names.  When __LDBL_COMPAT or
   __LDOUBLE_REDIRECTS_TO_FLOAT128_ABI are in effect, this is handled in
   bits/stdio-ldbl.h.  */
#include <bits/floatn.h>
#if !__GLIBC_USE (DEPRECATED_SCANF) && !defined __LDBL_COMPAT \
    && __LDOUBLE_REDIRECTS_TO_FLOAT128_ABI == 0
# if __GLIBC_USE (C2X_STRTOL)
#  ifdef __REDIRECT
extern int __REDIRECT (fscanf, (FILE *__restrict __stream,
				const char *__restrict __format, ...),
		       __isoc23_fscanf) __wur __nonnull ((1));
extern int __REDIRECT (scanf, (const char *__restrict __format, ...),
		       __isoc23_scanf) __wur;
extern int __REDIRECT_NTH (sscanf, (const char *__restrict __s,
				    const char *__restrict __format, ...),
			   __isoc23_sscanf);
#  else
extern int __isoc23_fscanf (FILE *__restrict __stream,
			    const char *__restrict __format, ...) __wur
  __nonnull ((1));
extern int __isoc23_scanf (const char *__restrict __format, ...) __wur;
extern int __isoc23_sscanf (const char *__restrict __s,
			    const char *__restrict __format, ...) __THROW;
#   define fscanf __isoc23_fscanf
#   define scanf __isoc23_scanf
#   define sscanf __isoc23_sscanf
#  endif
# else
#  ifdef __REDIRECT
extern int __REDIRECT (fscanf, (FILE *__restrict __stream,
				const char *__restrict __format, ...),
		       __isoc99_fscanf) __wur __nonnull ((1));
extern int __REDIRECT (scanf, (const char *__restrict __format, ...),
		       __isoc99_scanf) __wur;
extern int __REDIRECT_NTH (sscanf, (const char *__restrict __s,
				    const char *__restrict __format, ...),
			   __isoc99_sscanf);
#  else
extern int __isoc99_fscanf (FILE *__restrict __stream,
			    const char *__restrict __format, ...) __wur
  __nonnull ((1));
extern int __isoc99_scanf (const char *__restrict __format, ...) __wur;
extern int __isoc99_sscanf (const char *__restrict __s,
			    const char *__restrict __format, ...) __THROW;
#   define fscanf __isoc99_fscanf
#   define scanf __isoc99_scanf
#   define sscanf __isoc99_sscanf
#  endif
# endif
#endif

#ifdef	__USE_ISOC99
/* Read formatted input from S into argument list ARG.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int vfscanf (FILE *__restrict __s, const char *__restrict __format,
		    __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 2, 0))) __wur __nonnull ((1));

/* Read formatted input from stdin into argument list ARG.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int vscanf (const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__format__ (__scanf__, 1, 0))) __wur;

/* Read formatted input from S into argument list ARG.  */
extern int vsscanf (const char *__restrict __s,
		    const char *__restrict __format, __gnuc_va_list __arg)
     __THROW __attribute__ ((__format__ (__scanf__, 2, 0)));

/* Same redirection as above for the v*scanf family.  */
# if !__GLIBC_USE (DEPRECATED_SCANF)
#  if __GLIBC_USE (C2X_STRTOL)
#   if defined __REDIRECT && !defined __LDBL_COMPAT	\
      && __LDOUBLE_REDIRECTS_TO_FLOAT128_ABI == 0
extern int __REDIRECT (vfscanf,
		       (FILE *__restrict __s,
			const char *__restrict __format, __gnuc_va_list __arg),
		       __isoc23_vfscanf)
     __attribute__ ((__format__ (__scanf__, 2, 0))) __wur __nonnull ((1));
extern int __REDIRECT (vscanf, (const char *__restrict __format,
				__gnuc_va_list __arg), __isoc23_vscanf)
     __attribute__ ((__format__ (__scanf__, 1, 0))) __wur;
extern int __REDIRECT_NTH (vsscanf,
			   (const char *__restrict __s,
			    const char *__restrict __format,
			    __gnuc_va_list __arg), __isoc23_vsscanf)
     __attribute__ ((__format__ (__scanf__, 2, 0)));
#   elif !defined __REDIRECT
extern int __isoc23_vfscanf (FILE *__restrict __s,
			     const char *__restrict __format,
			     __gnuc_va_list __arg) __wur __nonnull ((1));
extern int __isoc23_vscanf (const char *__restrict __format,
			    __gnuc_va_list __arg) __wur;
extern int __isoc23_vsscanf (const char *__restrict __s,
			     const char *__restrict __format,
			     __gnuc_va_list __arg) __THROW;
#    define vfscanf __isoc23_vfscanf
#    define vscanf __isoc23_vscanf
#    define vsscanf __isoc23_vsscanf
#   endif
#  else
#   if defined __REDIRECT && !defined __LDBL_COMPAT	\
      && __LDOUBLE_REDIRECTS_TO_FLOAT128_ABI == 0
extern int __REDIRECT (vfscanf,
		       (FILE *__restrict __s,
			const char *__restrict __format, __gnuc_va_list __arg),
		       __isoc99_vfscanf)
     __attribute__ ((__format__ (__scanf__, 2, 0))) __wur __nonnull ((1));
extern int __REDIRECT (vscanf, (const char *__restrict __format,
				__gnuc_va_list __arg), __isoc99_vscanf)
     __attribute__ ((__format__ (__scanf__, 1, 0))) __wur;
extern int __REDIRECT_NTH (vsscanf,
			   (const char *__restrict __s,
			    const char *__restrict __format,
			    __gnuc_va_list __arg), __isoc99_vsscanf)
     __attribute__ ((__format__ (__scanf__, 2, 0)));
#   elif !defined __REDIRECT
extern int __isoc99_vfscanf (FILE *__restrict __s,
			     const char *__restrict __format,
			     __gnuc_va_list __arg) __wur __nonnull ((1));
extern int __isoc99_vscanf (const char *__restrict __format,
			    __gnuc_va_list __arg) __wur;
extern int __isoc99_vsscanf (const char *__restrict __s,
			     const char *__restrict __format,
			     __gnuc_va_list __arg) __THROW;
#    define vfscanf __isoc99_vfscanf
#    define vscanf __isoc99_vscanf
#    define vsscanf __isoc99_vsscanf
#   endif
#  endif
# endif
#endif /* Use ISO C9x.  */


/* Read a character from STREAM.

   These functions are possible cancellation points and therefore not
   marked with __THROW.  */
extern int fgetc (FILE *__stream) __nonnull ((1));
extern int getc (FILE *__stream) __nonnull ((1));

/* Read a character from stdin.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int getchar (void);

#ifdef __USE_POSIX199506
/* These are defined in POSIX.1:1996.

   These functions are possible cancellation points and therefore not
   marked with __THROW.  */
extern int getc_unlocked (FILE *__stream) __nonnull ((1));
extern int getchar_unlocked (void);
#endif /* Use POSIX.  */

#ifdef __USE_MISC
/* Faster version when locking is not necessary.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern int fgetc_unlocked (FILE *__stream) __nonnull ((1));
#endif /* Use MISC.  */


/* Write a character to STREAM.

   These functions are possible cancellation points and therefore not
   marked with __THROW.

   These functions is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fputc (int __c, FILE *__stream) __nonnull ((2));
extern int putc (int __c, FILE *__stream) __nonnull ((2));

/* Write a character to stdout.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int putchar (int __c);

#ifdef __USE_MISC
/* Faster version when locking is not necessary.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern int fputc_unlocked (int __c, FILE *__stream) __nonnull ((2));
#endif /* Use MISC.  */

#ifdef __USE_POSIX199506
/* These are defined in POSIX.1:1996.

   These functions are possible cancellation points and therefore not
   marked with __THROW.  */
extern int putc_unlocked (int __c, FILE *__stream) __nonnull ((2));
extern int putchar_unlocked (int __c);
#endif /* Use POSIX.  */


#if defined __USE_MISC \
    || (defined __USE_XOPEN && !defined __USE_XOPEN2K)
/* Get a word (int) from STREAM.  */
extern int getw (FILE *__stream) __nonnull ((1));

/* Write a word (int) to STREAM.  */
extern int putw (int __w, FILE *__stream) __nonnull ((2));
#endif


/* Get a newline-terminated string of finite length from STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream)
     __wur __fortified_attr_access (__write_only__, 1, 2) __nonnull ((3));

#if __GLIBC_USE (DEPRECATED_GETS)
/* Get a newline-terminated string from stdin, removing the newline.

   This function is impossible to use safely.  It has been officially
   removed from ISO C11 and ISO C++14, and we have also removed it
   from the _GNU_SOURCE feature list.  It remains available when
   explicitly using an old ISO C, Unix, or POSIX standard.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern char *gets (char *__s) __wur __attribute_deprecated__;
#endif

#ifdef __USE_GNU
/* This function does the same as `fgets' but does not lock the stream.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern char *fgets_unlocked (char *__restrict __s, int __n,
			     FILE *__restrict __stream) __wur
    __fortified_attr_access (__write_only__, 1, 2) __nonnull ((3));
#endif


#if defined __USE_XOPEN2K8 || __GLIBC_USE (LIB_EXT2)
/* Read up to (and including) a DELIMITER from STREAM into *LINEPTR
   (and null-terminate it). *LINEPTR is a pointer returned from malloc (or
   NULL), pointing to *N characters of space.  It is realloc'd as
   necessary.  Returns the number of characters read (not including the
   null terminator), or -1 on error or EOF.

   These functions are not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation they are cancellation points and
   therefore not marked with __THROW.  */
extern __ssize_t __getdelim (char **__restrict __lineptr,
                             size_t *__restrict __n, int __delimiter,
                             FILE *__restrict __stream) __wur __nonnull ((4));
extern __ssize_t getdelim (char **__restrict __lineptr,
                           size_t *__restrict __n, int __delimiter,
                           FILE *__restrict __stream) __wur __nonnull ((4));

/* Like `getdelim', but reads up to a newline.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern __ssize_t getline (char **__restrict __lineptr,
                          size_t *__restrict __n,
                          FILE *__restrict __stream) __wur __nonnull ((3));
#endif


/* Write a string to STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fputs (const char *__restrict __s, FILE *__restrict __stream)
  __nonnull ((2));

/* Write a string, followed by a newline, to stdout.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int puts (const char *__s);


/* Push a character back onto the input buffer of STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int ungetc (int __c, FILE *__stream) __nonnull ((2));


/* Read chunks of generic data from STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern size_t fread (void *__restrict __ptr, size_t __size,
		     size_t __n, FILE *__restrict __stream) __wur
  __nonnull((4));
/* Write chunks of generic data to STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern size_t fwrite (const void *__restrict __ptr, size_t __size,
		      size_t __n, FILE *__restrict __s) __nonnull((4));

#ifdef __USE_GNU
/* This function does the same as `fputs' but does not lock the stream.

   This function is not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation it is a cancellation point and
   therefore not marked with __THROW.  */
extern int fputs_unlocked (const char *__restrict __s,
			   FILE *__restrict __stream) __nonnull ((2));
#endif

#ifdef __USE_MISC
/* Faster versions when locking is not necessary.

   These functions are not part of POSIX and therefore no official
   cancellation point.  But due to similarity with an POSIX interface
   or due to the implementation they are cancellation points and
   therefore not marked with __THROW.  */
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
			      size_t __n, FILE *__restrict __stream) __wur
  __nonnull ((4));
extern size_t fwrite_unlocked (const void *__restrict __ptr, size_t __size,
			       size_t __n, FILE *__restrict __stream)
  __nonnull ((4));
#endif


/* Seek to a certain position on STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fseek (FILE *__stream, long int __off, int __whence)
  __nonnull ((1));
/* Return the current position of STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern long int ftell (FILE *__stream) __wur __nonnull ((1));
/* Rewind to the beginning of STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern void rewind (FILE *__stream) __nonnull ((1));

/* The Single Unix Specification, Version 2, specifies an alternative,
   more adequate interface for the two functions above which deal with
   file offset.  `long int' is not the right type.  These definitions
   are originally defined in the Large File Support API.  */

#if defined __USE_LARGEFILE || defined __USE_XOPEN2K
# ifndef __USE_FILE_OFFSET64
/* Seek to a certain position on STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fseeko (FILE *__stream, __off_t __off, int __whence)
  __nonnull ((1));
/* Return the current position of STREAM.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern __off_t ftello (FILE *__stream) __wur __nonnull ((1));
# else
#  ifdef __REDIRECT
extern int __REDIRECT (fseeko,
		       (FILE *__stream, __off64_t __off, int __whence),
		       fseeko64) __nonnull ((1));
extern __off64_t __REDIRECT (ftello, (FILE *__stream), ftello64)
  __nonnull ((1));
#  else
#   define fseeko fseeko64
#   define ftello ftello64
#  endif
# endif
#endif

#ifndef __USE_FILE_OFFSET64
/* Get STREAM's position.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos)
  __nonnull ((1));
/* Set STREAM's position.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int fsetpos (FILE *__stream, const fpos_t *__pos) __nonnull ((1));
#else
# ifdef __REDIRECT
extern int __REDIRECT (fgetpos, (FILE *__restrict __stream,
				 fpos_t *__restrict __pos), fgetpos64)
  __nonnull ((1));
extern int __REDIRECT (fsetpos,
		       (FILE *__stream, const fpos_t *__pos), fsetpos64)
  __nonnull ((1));
# else
#  define fgetpos fgetpos64
#  define fsetpos fsetpos64
# endif
#endif

#ifdef __USE_LARGEFILE64
extern int fseeko64 (FILE *__stream, __off64_t __off, int __whence)
  __nonnull ((1));
extern __off64_t ftello64 (FILE *__stream) __wur __nonnull ((1));
extern int fgetpos64 (FILE *__restrict __stream, fpos64_t *__restrict __pos)
  __nonnull ((1));
extern int fsetpos64 (FILE *__stream, const fpos64_t *__pos) __nonnull ((1));
#endif

/* Clear the error and EOF indicators for STREAM.  */
extern void clearerr (FILE *__stream) __THROW __nonnull ((1));
/* Return the EOF indicator for STREAM.  */
extern int feof (FILE *__stream) __THROW __wur __nonnull ((1));
/* Return the error indicator for STREAM.  */
extern int ferror (FILE *__stream) __THROW __wur __nonnull ((1));

#ifdef __USE_MISC
/* Faster versions when locking is not required.  */
extern void clearerr_unlocked (FILE *__stream) __THROW __nonnull ((1));
extern int feof_unlocked (FILE *__stream) __THROW __wur __nonnull ((1));
extern int ferror_unlocked (FILE *__stream) __THROW __wur __nonnull ((1));
#endif


/* Print a message describing the meaning of the value of errno.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern void perror (const char *__s) __COLD;


#ifdef	__USE_POSIX
/* Return the system file descriptor for STREAM.  */
extern int fileno (FILE *__stream) __THROW __wur __nonnull ((1));
#endif /* Use POSIX.  */

#ifdef __USE_MISC
/* Faster version when locking is not required.  */
extern int fileno_unlocked (FILE *__stream) __THROW __wur __nonnull ((1));
#endif


#ifdef __USE_POSIX2
/* Close a stream opened by popen and return the status of its child.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern int pclose (FILE *__stream) __nonnull ((1));

/* Create a new stream connected to a pipe running the given command.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
extern FILE *popen (const char *__command, const char *__modes)
  __attribute_malloc__ __attr_dealloc (pclose, 1) __wur;

#endif


#ifdef	__USE_POSIX
/* Return the name of the controlling terminal.  */
extern char *ctermid (char *__s) __THROW
  __attr_access ((__write_only__, 1));
#endif /* Use POSIX.  */


#if (defined __USE_XOPEN && !defined __USE_XOPEN2K) || defined __USE_GNU
/* Return the name of the current user.  */
extern char *cuserid (char *__s)
  __attr_access ((__write_only__, 1));
#endif /* Use X/Open, but not issue 6.  */


#ifdef	__USE_GNU
struct obstack;			/* See <obstack.h>.  */

/* Write formatted output to an obstack.  */
extern int obstack_printf (struct obstack *__restrict __obstack,
			   const char *__restrict __format, ...)
     __THROWNL __attribute__ ((__format__ (__printf__, 2, 3)));
extern int obstack_vprintf (struct obstack *__restrict __obstack,
			    const char *__restrict __format,
			    __gnuc_va_list __args)
     __THROWNL __attribute__ ((__format__ (__printf__, 2, 0)));
#endif /* Use GNU.  */


#ifdef __USE_POSIX199506
/* These are defined in POSIX.1:1996.  */

/* Acquire ownership of STREAM.  */
extern void flockfile (FILE *__stream) __THROW __nonnull ((1));

/* Try to acquire ownership of STREAM but do not block if it is not
   possible.  */
extern int ftrylockfile (FILE *__stream) __THROW __wur __nonnull ((1));

/* Relinquish the ownership granted for STREAM.  */
extern void funlockfile (FILE *__stream) __THROW __nonnull ((1));
#endif /* POSIX */

#if defined __USE_XOPEN && !defined __USE_XOPEN2K && !defined __USE_GNU
/*  X/Open Issues 1-5 required getopt to be declared in this
   header.  It was removed in Issue 6.  GNU follows Issue 6.  */
# include <bits/getopt_posix.h>
#endif

/* Slow-path routines used by the optimized inline functions in
   bits/stdio.h.  */
extern int __uflow (FILE *);
extern int __overflow (FILE *, int);

#if __USE_FORTIFY_LEVEL > 0 && defined __fortify_function
/* Declare all functions from bits/stdio2-decl.h first.  */
# include <bits/stdio2-decl.h>
#endif

/* The following headers provide asm redirections.  These redirections must
   appear before the first usage of these functions, e.g. in bits/stdio.h.  */
#if defined __LDBL_COMPAT || __LDOUBLE_REDIRECTS_TO_FLOAT128_ABI == 1
# include <bits/stdio-ldbl.h>
#endif

/* If we are compiling with optimizing read this file.  It contains
   several optimizing inline functions and macros.  */
#ifdef __USE_EXTERN_INLINES
# include <bits/stdio.h>
#endif
#if __USE_FORTIFY_LEVEL > 0 && defined __fortify_function
/* Now include the function definitions and redirects too.  */
# include <bits/stdio2.h>
#endif

__END_DECLS

#endif /* <stdio.h> included.  */
/* Copyright (C) 1991-2024 Free Software Foundation, Inc.
   Copyright The GNU Toolchain Authors.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	ISO C99 Standard: 7.20 General utilities	<stdlib.h>
 */

#ifndef	_STDLIB_H

#define __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION
#include <bits/libc-header-start.h>

/* Get size_t, wchar_t and NULL from <stddef.h>.  */
#define __need_size_t
#define __need_wchar_t
#define __need_NULL
#include <stddef.h>

__BEGIN_DECLS

#define	_STDLIB_H	1

#if (defined __USE_XOPEN || defined __USE_XOPEN2K8) && !defined _SYS_WAIT_H
/* XPG requires a few symbols from <sys/wait.h> being defined.  */
# include <bits/waitflags.h>
# include <bits/waitstatus.h>

/* Define the macros <sys/wait.h> also would define this way.  */
# define WEXITSTATUS(status)	__WEXITSTATUS (status)
# define WTERMSIG(status)	__WTERMSIG (status)
# define WSTOPSIG(status)	__WSTOPSIG (status)
# define WIFEXITED(status)	__WIFEXITED (status)
# define WIFSIGNALED(status)	__WIFSIGNALED (status)
# define WIFSTOPPED(status)	__WIFSTOPPED (status)
# ifdef __WIFCONTINUED
#  define WIFCONTINUED(status)	__WIFCONTINUED (status)
# endif
#endif	/* X/Open or XPG7 and <sys/wait.h> not included.  */

/* _FloatN API tests for enablement.  */
#include <bits/floatn.h>

/* Returned by `div'.  */
typedef struct
  {
    int quot;			/* Quotient.  */
    int rem;			/* Remainder.  */
  } div_t;

/* Returned by `ldiv'.  */
#ifndef __ldiv_t_defined
typedef struct
  {
    long int quot;		/* Quotient.  */
    long int rem;		/* Remainder.  */
  } ldiv_t;
# define __ldiv_t_defined	1
#endif

#if defined __USE_ISOC99 && !defined __lldiv_t_defined
/* Returned by `lldiv'.  */
__extension__ typedef struct
  {
    long long int quot;		/* Quotient.  */
    long long int rem;		/* Remainder.  */
  } lldiv_t;
# define __lldiv_t_defined	1
#endif


/* The largest number rand will return (same as INT_MAX).  */
#define	RAND_MAX	2147483647


/* We define these the same for all machines.
   Changes from this to the outside world should be done in `_exit'.  */
#define	EXIT_FAILURE	1	/* Failing exit status.  */
#define	EXIT_SUCCESS	0	/* Successful exit status.  */


/* Maximum length of a multibyte character in the current locale.  */
#define	MB_CUR_MAX	(__ctype_get_mb_cur_max ())
extern size_t __ctype_get_mb_cur_max (void) __THROW __wur;


/* Convert a string to a floating-point number.  */
extern double atof (const char *__nptr)
     __THROW __attribute_pure__ __nonnull ((1)) __wur;
/* Convert a string to an integer.  */
extern int atoi (const char *__nptr)
     __THROW __attribute_pure__ __nonnull ((1)) __wur;
/* Convert a string to a long integer.  */
extern long int atol (const char *__nptr)
     __THROW __attribute_pure__ __nonnull ((1)) __wur;

#ifdef __USE_ISOC99
/* Convert a string to a long long integer.  */
__extension__ extern long long int atoll (const char *__nptr)
     __THROW __attribute_pure__ __nonnull ((1)) __wur;
#endif

/* Convert a string to a floating-point number.  */
extern double strtod (const char *__restrict __nptr,
		      char **__restrict __endptr)
     __THROW __nonnull ((1));

#ifdef	__USE_ISOC99
/* Likewise for `float' and `long double' sizes of floating-point numbers.  */
extern float strtof (const char *__restrict __nptr,
		     char **__restrict __endptr) __THROW __nonnull ((1));

extern long double strtold (const char *__restrict __nptr,
			    char **__restrict __endptr)
     __THROW __nonnull ((1));
#endif

/* Likewise for '_FloatN' and '_FloatNx'.  */

#if __HAVE_FLOAT16 && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern _Float16 strtof16 (const char *__restrict __nptr,
			  char **__restrict __endptr)
     __THROW __nonnull ((1));
#endif

#if __HAVE_FLOAT32 && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern _Float32 strtof32 (const char *__restrict __nptr,
			  char **__restrict __endptr)
     __THROW __nonnull ((1));
#endif

#if __HAVE_FLOAT64 && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern _Float64 strtof64 (const char *__restrict __nptr,
			  char **__restrict __endptr)
     __THROW __nonnull ((1));
#endif

#if __HAVE_FLOAT128 && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern _Float128 strtof128 (const char *__restrict __nptr,
			    char **__restrict __endptr)
     __THROW __nonnull ((1));
#endif

#if __HAVE_FLOAT32X && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern _Float32x strtof32x (const char *__restrict __nptr,
			    char **__restrict __endptr)
     __THROW __nonnull ((1));
#endif

#if __HAVE_FLOAT64X && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern _Float64x strtof64x (const char *__restrict __nptr,
			    char **__restrict __endptr)
     __THROW __nonnull ((1));
#endif

#if __HAVE_FLOAT128X && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern _Float128x strtof128x (const char *__restrict __nptr,
			      char **__restrict __endptr)
     __THROW __nonnull ((1));
#endif

/* Convert a string to a long integer.  */
extern long int strtol (const char *__restrict __nptr,
			char **__restrict __endptr, int __base)
     __THROW __nonnull ((1));
/* Convert a string to an unsigned long integer.  */
extern unsigned long int strtoul (const char *__restrict __nptr,
				  char **__restrict __endptr, int __base)
     __THROW __nonnull ((1));

#ifdef __USE_MISC
/* Convert a string to a quadword integer.  */
__extension__
extern long long int strtoq (const char *__restrict __nptr,
			     char **__restrict __endptr, int __base)
     __THROW __nonnull ((1));
/* Convert a string to an unsigned quadword integer.  */
__extension__
extern unsigned long long int strtouq (const char *__restrict __nptr,
				       char **__restrict __endptr, int __base)
     __THROW __nonnull ((1));
#endif /* Use misc.  */

#ifdef __USE_ISOC99
/* Convert a string to a quadword integer.  */
__extension__
extern long long int strtoll (const char *__restrict __nptr,
			      char **__restrict __endptr, int __base)
     __THROW __nonnull ((1));
/* Convert a string to an unsigned quadword integer.  */
__extension__
extern unsigned long long int strtoull (const char *__restrict __nptr,
					char **__restrict __endptr, int __base)
     __THROW __nonnull ((1));
#endif /* ISO C99 or use MISC.  */

/* Versions of the above functions that handle '0b' and '0B' prefixes
   in base 0 or 2.  */
#if __GLIBC_USE (C2X_STRTOL)
# ifdef __REDIRECT
extern long int __REDIRECT_NTH (strtol, (const char *__restrict __nptr,
					 char **__restrict __endptr,
					 int __base), __isoc23_strtol)
     __nonnull ((1));
extern unsigned long int __REDIRECT_NTH (strtoul,
					 (const char *__restrict __nptr,
					  char **__restrict __endptr,
					  int __base), __isoc23_strtoul)
     __nonnull ((1));
#  ifdef __USE_MISC
__extension__
extern long long int __REDIRECT_NTH (strtoq, (const char *__restrict __nptr,
					      char **__restrict __endptr,
					      int __base), __isoc23_strtoll)
     __nonnull ((1));
__extension__
extern unsigned long long int __REDIRECT_NTH (strtouq,
					      (const char *__restrict __nptr,
					       char **__restrict __endptr,
					       int __base), __isoc23_strtoull)
     __nonnull ((1));
#  endif
__extension__
extern long long int __REDIRECT_NTH (strtoll, (const char *__restrict __nptr,
					       char **__restrict __endptr,
					       int __base), __isoc23_strtoll)
     __nonnull ((1));
__extension__
extern unsigned long long int __REDIRECT_NTH (strtoull,
					      (const char *__restrict __nptr,
					       char **__restrict __endptr,
					       int __base), __isoc23_strtoull)
     __nonnull ((1));
# else
extern long int __isoc23_strtol (const char *__restrict __nptr,
				 char **__restrict __endptr, int __base)
     __THROW __nonnull ((1));
extern unsigned long int __isoc23_strtoul (const char *__restrict __nptr,
					   char **__restrict __endptr,
					   int __base)
     __THROW __nonnull ((1));
__extension__
extern long long int __isoc23_strtoll (const char *__restrict __nptr,
				       char **__restrict __endptr, int __base)
     __THROW __nonnull ((1));
__extension__
extern unsigned long long int __isoc23_strtoull (const char *__restrict __nptr,
						 char **__restrict __endptr,
						 int __base)
     __THROW __nonnull ((1));
#  define strtol __isoc23_strtol
#  define strtoul __isoc23_strtoul
#  ifdef __USE_MISC
#   define strtoq __isoc23_strtoll
#   define strtouq __isoc23_strtoull
#  endif
#  define strtoll __isoc23_strtoll
#  define strtoull __isoc23_strtoull
# endif
#endif

/* Convert a floating-point number to a string.  */
#if __GLIBC_USE (IEC_60559_BFP_EXT_C2X)
extern int strfromd (char *__dest, size_t __size, const char *__format,
		     double __f)
     __THROW __nonnull ((3));

extern int strfromf (char *__dest, size_t __size, const char *__format,
		     float __f)
     __THROW __nonnull ((3));

extern int strfroml (char *__dest, size_t __size, const char *__format,
		     long double __f)
     __THROW __nonnull ((3));
#endif

#if __HAVE_FLOAT16 && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern int strfromf16 (char *__dest, size_t __size, const char * __format,
		       _Float16 __f)
     __THROW __nonnull ((3));
#endif

#if __HAVE_FLOAT32 && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern int strfromf32 (char *__dest, size_t __size, const char * __format,
		       _Float32 __f)
     __THROW __nonnull ((3));
#endif

#if __HAVE_FLOAT64 && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern int strfromf64 (char *__dest, size_t __size, const char * __format,
		       _Float64 __f)
     __THROW __nonnull ((3));
#endif

#if __HAVE_FLOAT128 && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern int strfromf128 (char *__dest, size_t __size, const char * __format,
			_Float128 __f)
     __THROW __nonnull ((3));
#endif

#if __HAVE_FLOAT32X && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern int strfromf32x (char *__dest, size_t __size, const char * __format,
			_Float32x __f)
     __THROW __nonnull ((3));
#endif

#if __HAVE_FLOAT64X && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern int strfromf64x (char *__dest, size_t __size, const char * __format,
			_Float64x __f)
     __THROW __nonnull ((3));
#endif

#if __HAVE_FLOAT128X && __GLIBC_USE (IEC_60559_TYPES_EXT)
extern int strfromf128x (char *__dest, size_t __size, const char * __format,
			 _Float128x __f)
     __THROW __nonnull ((3));
#endif


#ifdef __USE_GNU
/* Parallel versions of the functions above which take the locale to
   use as an additional parameter.  These are GNU extensions inspired
   by the POSIX.1-2008 extended locale API.  */
# include <bits/types/locale_t.h>

extern long int strtol_l (const char *__restrict __nptr,
			  char **__restrict __endptr, int __base,
			  locale_t __loc) __THROW __nonnull ((1, 4));

extern unsigned long int strtoul_l (const char *__restrict __nptr,
				    char **__restrict __endptr,
				    int __base, locale_t __loc)
     __THROW __nonnull ((1, 4));

__extension__
extern long long int strtoll_l (const char *__restrict __nptr,
				char **__restrict __endptr, int __base,
				locale_t __loc)
     __THROW __nonnull ((1, 4));

__extension__
extern unsigned long long int strtoull_l (const char *__restrict __nptr,
					  char **__restrict __endptr,
					  int __base, locale_t __loc)
     __THROW __nonnull ((1, 4));

/* Versions of the above functions that handle '0b' and '0B' prefixes
   in base 0 or 2.  */
# if __GLIBC_USE (C2X_STRTOL)
#  ifdef __REDIRECT
extern long int __REDIRECT_NTH (strtol_l, (const char *__restrict __nptr,
					   char **__restrict __endptr,
					   int __base, locale_t __loc),
				__isoc23_strtol_l)
     __nonnull ((1, 4));
extern unsigned long int __REDIRECT_NTH (strtoul_l,
					 (const char *__restrict __nptr,
					  char **__restrict __endptr,
					  int __base, locale_t __loc),
					 __isoc23_strtoul_l)
     __nonnull ((1, 4));
__extension__
extern long long int __REDIRECT_NTH (strtoll_l, (const char *__restrict __nptr,
						 char **__restrict __endptr,
						 int __base,
						 locale_t __loc),
				     __isoc23_strtoll_l)
     __nonnull ((1, 4));
__extension__
extern unsigned long long int __REDIRECT_NTH (strtoull_l,
					      (const char *__restrict __nptr,
					       char **__restrict __endptr,
					       int __base, locale_t __loc),
					      __isoc23_strtoull_l)
     __nonnull ((1, 4));
#  else
extern long int __isoc23_strtol_l (const char *__restrict __nptr,
				   char **__restrict __endptr, int __base,
				   locale_t __loc) __THROW __nonnull ((1, 4));
extern unsigned long int __isoc23_strtoul_l (const char *__restrict __nptr,
					     char **__restrict __endptr,
					     int __base, locale_t __loc)
     __THROW __nonnull ((1, 4));
__extension__
extern long long int __isoc23_strtoll_l (const char *__restrict __nptr,
					 char **__restrict __endptr,
					 int __base, locale_t __loc)
     __THROW __nonnull ((1, 4));
__extension__
extern unsigned long long int __isoc23_strtoull_l (const char *__restrict __nptr,
						   char **__restrict __endptr,
						   int __base, locale_t __loc)
     __THROW __nonnull ((1, 4));
#   define strtol_l __isoc23_strtol_l
#   define strtoul_l __isoc23_strtoul_l
#   define strtoll_l __isoc23_strtoll_l
#   define strtoull_l __isoc23_strtoull_l
#  endif
# endif

extern double strtod_l (const char *__restrict __nptr,
			char **__restrict __endptr, locale_t __loc)
     __THROW __nonnull ((1, 3));

extern float strtof_l (const char *__restrict __nptr,
		       char **__restrict __endptr, locale_t __loc)
     __THROW __nonnull ((1, 3));

extern long double strtold_l (const char *__restrict __nptr,
			      char **__restrict __endptr,
			      locale_t __loc)
     __THROW __nonnull ((1, 3));

# if __HAVE_FLOAT16
extern _Float16 strtof16_l (const char *__restrict __nptr,
			    char **__restrict __endptr,
			    locale_t __loc)
     __THROW __nonnull ((1, 3));
# endif

# if __HAVE_FLOAT32
extern _Float32 strtof32_l (const char *__restrict __nptr,
			    char **__restrict __endptr,
			    locale_t __loc)
     __THROW __nonnull ((1, 3));
# endif

# if __HAVE_FLOAT64
extern _Float64 strtof64_l (const char *__restrict __nptr,
			    char **__restrict __endptr,
			    locale_t __loc)
     __THROW __nonnull ((1, 3));
# endif

# if __HAVE_FLOAT128
extern _Float128 strtof128_l (const char *__restrict __nptr,
			      char **__restrict __endptr,
			      locale_t __loc)
     __THROW __nonnull ((1, 3));
# endif

# if __HAVE_FLOAT32X
extern _Float32x strtof32x_l (const char *__restrict __nptr,
			      char **__restrict __endptr,
			      locale_t __loc)
     __THROW __nonnull ((1, 3));
# endif

# if __HAVE_FLOAT64X
extern _Float64x strtof64x_l (const char *__restrict __nptr,
			      char **__restrict __endptr,
			      locale_t __loc)
     __THROW __nonnull ((1, 3));
# endif

# if __HAVE_FLOAT128X
extern _Float128x strtof128x_l (const char *__restrict __nptr,
				char **__restrict __endptr,
				locale_t __loc)
     __THROW __nonnull ((1, 3));
# endif
#endif /* GNU */


#ifdef __USE_EXTERN_INLINES
__extern_inline int
__NTH (atoi (const char *__nptr))
{
  return (int) strtol (__nptr, (char **) NULL, 10);
}
__extern_inline long int
__NTH (atol (const char *__nptr))
{
  return strtol (__nptr, (char **) NULL, 10);
}

# ifdef __USE_ISOC99
__extension__ __extern_inline long long int
__NTH (atoll (const char *__nptr))
{
  return strtoll (__nptr, (char **) NULL, 10);
}
# endif
#endif /* Optimizing and Inlining.  */


#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED
/* Convert N to base 64 using the digits "./0-9A-Za-z", least-significant
   digit first.  Returns a pointer to static storage overwritten by the
   next call.  */
extern char *l64a (long int __n) __THROW __wur;

/* Read a number from a string S in base 64 as above.  */
extern long int a64l (const char *__s)
     __THROW __attribute_pure__ __nonnull ((1)) __wur;

#endif	/* Use misc || extended X/Open.  */

#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED
# include <sys/types.h>	/* we need int32_t... */

/* These are the functions that actually do things.  The `random', `srandom',
   `initstate' and `setstate' functions are those from BSD Unices.
   The `rand' and `srand' functions are required by the ANSI standard.
   We provide both interfaces to the same random number generator.  */
/* Return a random long integer between 0 and 2^31-1 inclusive.  */
extern long int random (void) __THROW;

/* Seed the random number generator with the given number.  */
extern void srandom (unsigned int __seed) __THROW;

/* Initialize the random number generator to use state buffer STATEBUF,
   of length STATELEN, and seed it with SEED.  Optimal lengths are 8, 16,
   32, 64, 128 and 256, the bigger the better; values less than 8 will
   cause an error and values greater than 256 will be rounded down.  */
extern char *initstate (unsigned int __seed, char *__statebuf,
			size_t __statelen) __THROW __nonnull ((2));

/* Switch the random number generator to state buffer STATEBUF,
   which should have been previously initialized by `initstate'.  */
extern char *setstate (char *__statebuf) __THROW __nonnull ((1));


# ifdef __USE_MISC
/* Reentrant versions of the `random' family of functions.
   These functions all use the following data structure to contain
   state, rather than global state variables.  */

struct random_data
  {
    int32_t *fptr;		/* Front pointer.  */
    int32_t *rptr;		/* Rear pointer.  */
    int32_t *state;		/* Array of state values.  */
    int rand_type;		/* Type of random number generator.  */
    int rand_deg;		/* Degree of random number generator.  */
    int rand_sep;		/* Distance between front and rear.  */
    int32_t *end_ptr;		/* Pointer behind state table.  */
  };

extern int random_r (struct random_data *__restrict __buf,
		     int32_t *__restrict __result) __THROW __nonnull ((1, 2));

extern int srandom_r (unsigned int __seed, struct random_data *__buf)
     __THROW __nonnull ((2));

extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
			size_t __statelen,
			struct random_data *__restrict __buf)
     __THROW __nonnull ((2, 4));

extern int setstate_r (char *__restrict __statebuf,
		       struct random_data *__restrict __buf)
     __THROW __nonnull ((1, 2));
# endif	/* Use misc.  */
#endif	/* Use extended X/Open || misc. */


/* Return a random integer between 0 and RAND_MAX inclusive.  */
extern int rand (void) __THROW;
/* Seed the random number generator with the given number.  */
extern void srand (unsigned int __seed) __THROW;

#ifdef __USE_POSIX199506
/* Reentrant interface according to POSIX.1.  */
extern int rand_r (unsigned int *__seed) __THROW;
#endif


#if defined __USE_MISC || defined __USE_XOPEN
/* System V style 48-bit random number generator functions.  */

/* Return non-negative, double-precision floating-point value in [0.0,1.0).  */
extern double drand48 (void) __THROW;
extern double erand48 (unsigned short int __xsubi[3]) __THROW __nonnull ((1));

/* Return non-negative, long integer in [0,2^31).  */
extern long int lrand48 (void) __THROW;
extern long int nrand48 (unsigned short int __xsubi[3])
     __THROW __nonnull ((1));

/* Return signed, long integers in [-2^31,2^31).  */
extern long int mrand48 (void) __THROW;
extern long int jrand48 (unsigned short int __xsubi[3])
     __THROW __nonnull ((1));

/* Seed random number generator.  */
extern void srand48 (long int __seedval) __THROW;
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     __THROW __nonnull ((1));
extern void lcong48 (unsigned short int __param[7]) __THROW __nonnull ((1));

# ifdef __USE_MISC
/* Data structure for communication with thread safe versions.  This
   type is to be regarded as opaque.  It's only exported because users
   have to allocate objects of this type.  */
struct drand48_data
  {
    unsigned short int __x[3];	/* Current state.  */
    unsigned short int __old_x[3]; /* Old state.  */
    unsigned short int __c;	/* Additive const. in congruential formula.  */
    unsigned short int __init;	/* Flag for initializing.  */
    __extension__ unsigned long long int __a;	/* Factor in congruential
						   formula.  */
  };

/* Return non-negative, double-precision floating-point value in [0.0,1.0).  */
extern int drand48_r (struct drand48_data *__restrict __buffer,
		      double *__restrict __result) __THROW __nonnull ((1, 2));
extern int erand48_r (unsigned short int __xsubi[3],
		      struct drand48_data *__restrict __buffer,
		      double *__restrict __result) __THROW __nonnull ((1, 2));

/* Return non-negative, long integer in [0,2^31).  */
extern int lrand48_r (struct drand48_data *__restrict __buffer,
		      long int *__restrict __result)
     __THROW __nonnull ((1, 2));
extern int nrand48_r (unsigned short int __xsubi[3],
		      struct drand48_data *__restrict __buffer,
		      long int *__restrict __result)
     __THROW __nonnull ((1, 2));

/* Return signed, long integers in [-2^31,2^31).  */
extern int mrand48_r (struct drand48_data *__restrict __buffer,
		      long int *__restrict __result)
     __THROW __nonnull ((1, 2));
extern int jrand48_r (unsigned short int __xsubi[3],
		      struct drand48_data *__restrict __buffer,
		      long int *__restrict __result)
     __THROW __nonnull ((1, 2));

/* Seed random number generator.  */
extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     __THROW __nonnull ((2));

extern int seed48_r (unsigned short int __seed16v[3],
		     struct drand48_data *__buffer) __THROW __nonnull ((1, 2));

extern int lcong48_r (unsigned short int __param[7],
		      struct drand48_data *__buffer)
     __THROW __nonnull ((1, 2));

/* Return a random integer between zero and 2**32-1 (inclusive).  */
extern __uint32_t arc4random (void)
     __THROW __wur;

/* Fill the buffer with random data.  */
extern void arc4random_buf (void *__buf, size_t __size)
     __THROW __nonnull ((1));

/* Return a random number between zero (inclusive) and the specified
   limit (exclusive).  */
extern __uint32_t arc4random_uniform (__uint32_t __upper_bound)
     __THROW __wur;
# endif	/* Use misc.  */
#endif	/* Use misc or X/Open.  */

/* Allocate SIZE bytes of memory.  */
extern void *malloc (size_t __size) __THROW __attribute_malloc__
     __attribute_alloc_size__ ((1)) __wur;
/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
extern void *calloc (size_t __nmemb, size_t __size)
     __THROW __attribute_malloc__ __attribute_alloc_size__ ((1, 2)) __wur;

/* Re-allocate the previously allocated block
   in PTR, making the new block SIZE bytes long.  */
/* __attribute_malloc__ is not used, because if realloc returns
   the same pointer that was passed to it, aliasing needs to be allowed
   between objects pointed by the old and new pointers.  */
extern void *realloc (void *__ptr, size_t __size)
     __THROW __attribute_warn_unused_result__ __attribute_alloc_size__ ((2));

/* Free a block allocated by `malloc', `realloc' or `calloc'.  */
extern void free (void *__ptr) __THROW;

#ifdef __USE_MISC
/* Re-allocate the previously allocated block in PTR, making the new
   block large enough for NMEMB elements of SIZE bytes each.  */
/* __attribute_malloc__ is not used, because if reallocarray returns
   the same pointer that was passed to it, aliasing needs to be allowed
   between objects pointed by the old and new pointers.  */
extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size)
     __THROW __attribute_warn_unused_result__
     __attribute_alloc_size__ ((2, 3))
    __attr_dealloc_free;

/* Add reallocarray as its own deallocator.  */
extern void *reallocarray (void *__ptr, size_t __nmemb, size_t __size)
     __THROW __attr_dealloc (reallocarray, 1);
#endif

#ifdef __USE_MISC
# include <alloca.h>
#endif /* Use misc.  */

#if (defined __USE_XOPEN_EXTENDED && !defined __USE_XOPEN2K) \
    || defined __USE_MISC
/* Allocate SIZE bytes on a page boundary.  The storage cannot be freed.  */
extern void *valloc (size_t __size) __THROW __attribute_malloc__
     __attribute_alloc_size__ ((1)) __wur;
#endif

#ifdef __USE_XOPEN2K
/* Allocate memory of SIZE bytes with an alignment of ALIGNMENT.  */
extern int posix_memalign (void **__memptr, size_t __alignment, size_t __size)
     __THROW __nonnull ((1)) __wur;
#endif

#ifdef __USE_ISOC11
/* ISO C variant of aligned allocation.  */
extern void *aligned_alloc (size_t __alignment, size_t __size)
     __THROW __attribute_malloc__ __attribute_alloc_align__ ((1))
     __attribute_alloc_size__ ((2)) __wur;
#endif

/* Abort execution and generate a core-dump.  */
extern void abort (void) __THROW __attribute__ ((__noreturn__));


/* Register a function to be called when `exit' is called.  */
extern int atexit (void (*__func) (void)) __THROW __nonnull ((1));

#if defined __USE_ISOC11 || defined __USE_ISOCXX11
/* Register a function to be called when `quick_exit' is called.  */
# ifdef __cplusplus
extern "C++" int at_quick_exit (void (*__func) (void))
     __THROW __asm ("at_quick_exit") __nonnull ((1));
# else
extern int at_quick_exit (void (*__func) (void)) __THROW __nonnull ((1));
# endif
#endif

#ifdef	__USE_MISC
/* Register a function to be called with the status
   given to `exit' and the given argument.  */
extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     __THROW __nonnull ((1));
#endif

/* Call all functions registered with `atexit' and `on_exit',
   in the reverse of the order in which they were registered,
   perform stdio cleanup, and terminate program execution with STATUS.  */
extern void exit (int __status) __THROW __attribute__ ((__noreturn__));

#if defined __USE_ISOC11 || defined __USE_ISOCXX11
/* Call all functions registered with `at_quick_exit' in the reverse
   of the order in which they were registered and terminate program
   execution with STATUS.  */
extern void quick_exit (int __status) __THROW __attribute__ ((__noreturn__));
#endif

#ifdef __USE_ISOC99
/* Terminate the program with STATUS without calling any of the
   functions registered with `atexit' or `on_exit'.  */
extern void _Exit (int __status) __THROW __attribute__ ((__noreturn__));
#endif


/* Return the value of envariable NAME, or NULL if it doesn't exist.  */
extern char *getenv (const char *__name) __THROW __nonnull ((1)) __wur;

#ifdef __USE_GNU
/* This function is similar to the above but returns NULL if the
   programs is running with SUID or SGID enabled.  */
extern char *secure_getenv (const char *__name)
     __THROW __nonnull ((1)) __wur;
#endif

#if defined __USE_MISC || defined __USE_XOPEN
/* The SVID says this is in <stdio.h>, but this seems a better place.	*/
/* Put STRING, which is of the form "NAME=VALUE", in the environment.
   If there is no `=', remove NAME from the environment.  */
extern int putenv (char *__string) __THROW __nonnull ((1));
#endif

#ifdef __USE_XOPEN2K
/* Set NAME to VALUE in the environment.
   If REPLACE is nonzero, overwrite an existing value.  */
extern int setenv (const char *__name, const char *__value, int __replace)
     __THROW __nonnull ((2));

/* Remove the variable NAME from the environment.  */
extern int unsetenv (const char *__name) __THROW __nonnull ((1));
#endif

#ifdef	__USE_MISC
/* The `clearenv' was planned to be added to POSIX.1 but probably
   never made it.  Nevertheless the POSIX.9 standard (POSIX bindings
   for Fortran 77) requires this function.  */
extern int clearenv (void) __THROW;
#endif


#if defined __USE_MISC \
    || (defined __USE_XOPEN_EXTENDED && !defined __USE_XOPEN2K8)
/* Generate a unique temporary file name from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the file name unique.
   Always returns TEMPLATE, it's either a temporary file name or a null
   string if it cannot get a unique file name.  */
extern char *mktemp (char *__template) __THROW __nonnull ((1));
#endif

#if defined __USE_XOPEN_EXTENDED || defined __USE_XOPEN2K8
/* Generate a unique temporary file name from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the filename unique.
   Returns a file descriptor open on the file for reading and writing,
   or -1 if it cannot create a uniquely-named file.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
# ifndef __USE_FILE_OFFSET64
extern int mkstemp (char *__template) __nonnull ((1)) __wur;
# else
#  ifdef __REDIRECT
extern int __REDIRECT (mkstemp, (char *__template), mkstemp64)
     __nonnull ((1)) __wur;
#  else
#   define mkstemp mkstemp64
#  endif
# endif
# ifdef __USE_LARGEFILE64
extern int mkstemp64 (char *__template) __nonnull ((1)) __wur;
# endif
#endif

#ifdef __USE_MISC
/* Similar to mkstemp, but the template can have a suffix after the
   XXXXXX.  The length of the suffix is specified in the second
   parameter.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
# ifndef __USE_FILE_OFFSET64
extern int mkstemps (char *__template, int __suffixlen) __nonnull ((1)) __wur;
# else
#  ifdef __REDIRECT
extern int __REDIRECT (mkstemps, (char *__template, int __suffixlen),
		       mkstemps64) __nonnull ((1)) __wur;
#  else
#   define mkstemps mkstemps64
#  endif
# endif
# ifdef __USE_LARGEFILE64
extern int mkstemps64 (char *__template, int __suffixlen)
     __nonnull ((1)) __wur;
# endif
#endif

#ifdef __USE_XOPEN2K8
/* Create a unique temporary directory from TEMPLATE.
   The last six characters of TEMPLATE must be "XXXXXX";
   they are replaced with a string that makes the directory name unique.
   Returns TEMPLATE, or a null pointer if it cannot get a unique name.
   The directory is created mode 700.  */
extern char *mkdtemp (char *__template) __THROW __nonnull ((1)) __wur;
#endif

#ifdef __USE_GNU
/* Generate a unique temporary file name from TEMPLATE similar to
   mkstemp.  But allow the caller to pass additional flags which are
   used in the open call to create the file..

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
# ifndef __USE_FILE_OFFSET64
extern int mkostemp (char *__template, int __flags) __nonnull ((1)) __wur;
# else
#  ifdef __REDIRECT
extern int __REDIRECT (mkostemp, (char *__template, int __flags), mkostemp64)
     __nonnull ((1)) __wur;
#  else
#   define mkostemp mkostemp64
#  endif
# endif
# ifdef __USE_LARGEFILE64
extern int mkostemp64 (char *__template, int __flags) __nonnull ((1)) __wur;
# endif

/* Similar to mkostemp, but the template can have a suffix after the
   XXXXXX.  The length of the suffix is specified in the second
   parameter.

   This function is a possible cancellation point and therefore not
   marked with __THROW.  */
# ifndef __USE_FILE_OFFSET64
extern int mkostemps (char *__template, int __suffixlen, int __flags)
     __nonnull ((1)) __wur;
# else
#  ifdef __REDIRECT
extern int __REDIRECT (mkostemps, (char *__template, int __suffixlen,
				   int __flags), mkostemps64)
     __nonnull ((1)) __wur;
#  else
#   define mkostemps mkostemps64
#  endif
# endif
# ifdef __USE_LARGEFILE64
extern int mkostemps64 (char *__template, int __suffixlen, int __flags)
     __nonnull ((1)) __wur;
# endif
#endif


/* Execute the given line as a shell command.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
extern int system (const char *__command) __wur;


#ifdef	__USE_GNU
/* Return a malloc'd string containing the canonical absolute name of the
   existing named file.  */
extern char *canonicalize_file_name (const char *__name)
     __THROW __nonnull ((1)) __attribute_malloc__
     __attr_dealloc_free __wur;
#endif

#if defined __USE_MISC || defined __USE_XOPEN_EXTENDED
/* Return the canonical absolute name of file NAME.  If RESOLVED is
   null, the result is malloc'd; otherwise, if the canonical name is
   PATH_MAX chars or more, returns null with `errno' set to
   ENAMETOOLONG; if the name fits in fewer than PATH_MAX chars,
   returns the name in RESOLVED.  */
extern char *realpath (const char *__restrict __name,
		       char *__restrict __resolved) __THROW __wur;
#endif


/* Shorthand for type of comparison functions.  */
#ifndef __COMPAR_FN_T
# define __COMPAR_FN_T
typedef int (*__compar_fn_t) (const void *, const void *);

# ifdef	__USE_GNU
typedef __compar_fn_t comparison_fn_t;
# endif
#endif
#ifdef __USE_GNU
typedef int (*__compar_d_fn_t) (const void *, const void *, void *);
#endif

/* Do a binary search for KEY in BASE, which consists of NMEMB elements
   of SIZE bytes each, using COMPAR to perform the comparisons.  */
extern void *bsearch (const void *__key, const void *__base,
		      size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __nonnull ((1, 2, 5)) __wur;

#ifdef __USE_EXTERN_INLINES
# include <bits/stdlib-bsearch.h>
#endif

/* Sort NMEMB elements of BASE, of SIZE bytes each,
   using COMPAR to perform the comparisons.  */
extern void qsort (void *__base, size_t __nmemb, size_t __size,
		   __compar_fn_t __compar) __nonnull ((1, 4));
#ifdef __USE_GNU
extern void qsort_r (void *__base, size_t __nmemb, size_t __size,
		     __compar_d_fn_t __compar, void *__arg)
  __nonnull ((1, 4));
#endif


/* Return the absolute value of X.  */
extern int abs (int __x) __THROW __attribute__ ((__const__)) __wur;
extern long int labs (long int __x) __THROW __attribute__ ((__const__)) __wur;

#ifdef __USE_ISOC99
__extension__ extern long long int llabs (long long int __x)
     __THROW __attribute__ ((__const__)) __wur;
#endif


/* Return the `div_t', `ldiv_t' or `lldiv_t' representation
   of the value of NUMER over DENOM. */
/* GCC may have built-ins for these someday.  */
extern div_t div (int __numer, int __denom)
     __THROW __attribute__ ((__const__)) __wur;
extern ldiv_t ldiv (long int __numer, long int __denom)
     __THROW __attribute__ ((__const__)) __wur;

#ifdef __USE_ISOC99
__extension__ extern lldiv_t lldiv (long long int __numer,
				    long long int __denom)
     __THROW __attribute__ ((__const__)) __wur;
#endif


#if (defined __USE_XOPEN_EXTENDED && !defined __USE_XOPEN2K8) \
    || defined __USE_MISC
/* Convert floating point numbers to strings.  The returned values are
   valid only until another call to the same function.  */

/* Convert VALUE to a string with NDIGIT digits and return a pointer to
   this.  Set *DECPT with the position of the decimal character and *SIGN
   with the sign of the number.  */
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
		   int *__restrict __sign) __THROW __nonnull ((3, 4)) __wur;

/* Convert VALUE to a string rounded to NDIGIT decimal digits.  Set *DECPT
   with the position of the decimal character and *SIGN with the sign of
   the number.  */
extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
		   int *__restrict __sign) __THROW __nonnull ((3, 4)) __wur;

/* If possible convert VALUE to a string with NDIGIT significant digits.
   Otherwise use exponential representation.  The resulting string will
   be written to BUF.  */
extern char *gcvt (double __value, int __ndigit, char *__buf)
     __THROW __nonnull ((3)) __wur;
#endif

#ifdef __USE_MISC
/* Long double versions of above functions.  */
extern char *qecvt (long double __value, int __ndigit,
		    int *__restrict __decpt, int *__restrict __sign)
     __THROW __nonnull ((3, 4)) __wur;
extern char *qfcvt (long double __value, int __ndigit,
		    int *__restrict __decpt, int *__restrict __sign)
     __THROW __nonnull ((3, 4)) __wur;
extern char *qgcvt (long double __value, int __ndigit, char *__buf)
     __THROW __nonnull ((3)) __wur;


/* Reentrant version of the functions above which provide their own
   buffers.  */
extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
		   int *__restrict __sign, char *__restrict __buf,
		   size_t __len) __THROW __nonnull ((3, 4, 5));
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
		   int *__restrict __sign, char *__restrict __buf,
		   size_t __len) __THROW __nonnull ((3, 4, 5));

extern int qecvt_r (long double __value, int __ndigit,
		    int *__restrict __decpt, int *__restrict __sign,
		    char *__restrict __buf, size_t __len)
     __THROW __nonnull ((3, 4, 5));
extern int qfcvt_r (long double __value, int __ndigit,
		    int *__restrict __decpt, int *__restrict __sign,
		    char *__restrict __buf, size_t __len)
     __THROW __nonnull ((3, 4, 5));
#endif	/* misc */


/* Return the length of the multibyte character
   in S, which is no longer than N.  */
extern int mblen (const char *__s, size_t __n) __THROW;
/* Return the length of the given multibyte character,
   putting its `wchar_t' representation in *PWC.  */
extern int mbtowc (wchar_t *__restrict __pwc,
		   const char *__restrict __s, size_t __n) __THROW;
/* Put the multibyte character represented
   by WCHAR in S, returning its length.  */
extern int wctomb (char *__s, wchar_t __wchar) __THROW;


/* Convert a multibyte string to a wide char string.  */
extern size_t mbstowcs (wchar_t *__restrict  __pwcs,
			const char *__restrict __s, size_t __n) __THROW
    __attr_access ((__read_only__, 2));
/* Convert a wide char string to multibyte string.  */
extern size_t wcstombs (char *__restrict __s,
			const wchar_t *__restrict __pwcs, size_t __n)
     __THROW
  __fortified_attr_access (__write_only__, 1, 3)
  __attr_access ((__read_only__, 2));

#ifdef __USE_MISC
/* Determine whether the string value of RESPONSE matches the affirmation
   or negative response expression as specified by the LC_MESSAGES category
   in the program's current locale.  Returns 1 if affirmative, 0 if
   negative, and -1 if not matching.  */
extern int rpmatch (const char *__response) __THROW __nonnull ((1)) __wur;
#endif


#if defined __USE_XOPEN_EXTENDED || defined __USE_XOPEN2K8
/* Parse comma separated suboption from *OPTIONP and match against
   strings in TOKENS.  If found return index and set *VALUEP to
   optional value introduced by an equal sign.  If the suboption is
   not part of TOKENS return in *VALUEP beginning of unknown
   suboption.  On exit *OPTIONP is set to the beginning of the next
   token or at the terminating NUL character.  */
extern int getsubopt (char **__restrict __optionp,
		      char *const *__restrict __tokens,
		      char **__restrict __valuep)
     __THROW __nonnull ((1, 2, 3)) __wur;
#endif


/* X/Open pseudo terminal handling.  */

#ifdef __USE_XOPEN2KXSI
/* Return a master pseudo-terminal handle.  */
extern int posix_openpt (int __oflag) __wur;
#endif

#ifdef __USE_XOPEN_EXTENDED
/* The next four functions all take a master pseudo-tty fd and
   perform an operation on the associated slave:  */

/* Chown the slave to the calling user.  */
extern int grantpt (int __fd) __THROW;

/* Release an internal lock so the slave can be opened.
   Call after grantpt().  */
extern int unlockpt (int __fd) __THROW;

/* Return the pathname of the pseudo terminal slave associated with
   the master FD is open on, or NULL on errors.
   The returned storage is good until the next call to this function.  */
extern char *ptsname (int __fd) __THROW __wur;
#endif

#ifdef __USE_GNU
/* Store at most BUFLEN characters of the pathname of the slave pseudo
   terminal associated with the master FD is open on in BUF.
   Return 0 on success, otherwise an error number.  */
extern int ptsname_r (int __fd, char *__buf, size_t __buflen)
     __THROW __nonnull ((2)) __fortified_attr_access (__write_only__, 2, 3);

/* Open a master pseudo terminal and return its file descriptor.  */
extern int getpt (void);
#endif

#ifdef __USE_MISC
/* Put the 1 minute, 5 minute and 15 minute load averages into the first
   NELEM elements of LOADAVG.  Return the number written (never more than
   three, but may be less than NELEM), or -1 if an error occurred.  */
extern int getloadavg (double __loadavg[], int __nelem)
     __THROW __nonnull ((1));
#endif

#if defined __USE_XOPEN_EXTENDED && !defined __USE_XOPEN2K
/* Return the index into the active-logins file (utmp) for
   the controlling terminal.  */
extern int ttyslot (void) __THROW;
#endif

#include <bits/stdlib-float.h>

/* Define some macros helping to catch buffer overflows.  */
#if __USE_FORTIFY_LEVEL > 0 && defined __fortify_function
# include <bits/stdlib.h>
#endif

#include <bits/floatn.h>
#if defined __LDBL_COMPAT || __LDOUBLE_REDIRECTS_TO_FLOAT128_ABI == 1
# include <bits/stdlib-ldbl.h>
#endif

__END_DECLS

#endif /* stdlib.h  */
/* Copyright (C) 1991-2024 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

/*
 *	ISO C99 Standard: 7.21 String handling	<string.h>
 */

#ifndef	_STRING_H
#define	_STRING_H	1

#define __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION
#include <bits/libc-header-start.h>

__BEGIN_DECLS

/* Get size_t and NULL from <stddef.h>.  */
#define	__need_size_t
#define	__need_NULL
#include <stddef.h>

/* Tell the caller that we provide correct C++ prototypes.  */
#if defined __cplusplus && (__GNUC_PREREQ (4, 4) \
			    || __glibc_clang_prereq (3, 5))
# define __CORRECT_ISO_CPP_STRING_H_PROTO
#endif


/* Copy N bytes of SRC to DEST.  */
extern void *memcpy (void *__restrict __dest, const void *__restrict __src,
		     size_t __n) __THROW __nonnull ((1, 2));
/* Copy N bytes of SRC to DEST, guaranteeing
   correct behavior for overlapping strings.  */
extern void *memmove (void *__dest, const void *__src, size_t __n)
     __THROW __nonnull ((1, 2));

/* Copy no more than N bytes of SRC to DEST, stopping when C is found.
   Return the position in DEST one byte past where C was copied,
   or NULL if C was not found in the first N bytes of SRC.  */
#if defined __USE_MISC || defined __USE_XOPEN || __GLIBC_USE (ISOC2X)
extern void *memccpy (void *__restrict __dest, const void *__restrict __src,
		      int __c, size_t __n)
    __THROW __nonnull ((1, 2)) __attr_access ((__write_only__, 1, 4));
#endif /* Misc || X/Open.  */


/* Set N bytes of S to C.  */
extern void *memset (void *__s, int __c, size_t __n) __THROW __nonnull ((1));

/* Compare N bytes of S1 and S2.  */
extern int memcmp (const void *__s1, const void *__s2, size_t __n)
     __THROW __attribute_pure__ __nonnull ((1, 2));

/* Compare N bytes of S1 and S2.  Return zero if S1 and S2 are equal.
   Return some non-zero value otherwise.

   Essentially __memcmpeq has the exact same semantics as memcmp
   except the return value is less constrained.  memcmp is always a
   correct implementation of __memcmpeq.  As well !!memcmp, -memcmp,
   or bcmp are correct implementations.

   __memcmpeq is meant to be used by compilers when memcmp return is
   only used for its boolean value.

   __memcmpeq is declared only for use by compilers.  Programs should
   continue to use memcmp.  */
extern int __memcmpeq (const void *__s1, const void *__s2, size_t __n)
     __THROW __attribute_pure__ __nonnull ((1, 2));

/* Search N bytes of S for C.  */
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++"
{
extern void *memchr (void *__s, int __c, size_t __n)
      __THROW __asm ("memchr") __attribute_pure__ __nonnull ((1));
extern const void *memchr (const void *__s, int __c, size_t __n)
      __THROW __asm ("memchr") __attribute_pure__ __nonnull ((1));

# ifdef __OPTIMIZE__
__extern_always_inline void *
memchr (void *__s, int __c, size_t __n) __THROW
{
  return __builtin_memchr (__s, __c, __n);
}

__extern_always_inline const void *
memchr (const void *__s, int __c, size_t __n) __THROW
{
  return __builtin_memchr (__s, __c, __n);
}
# endif
}
#else
extern void *memchr (const void *__s, int __c, size_t __n)
      __THROW __attribute_pure__ __nonnull ((1));
#endif

#ifdef __USE_GNU
/* Search in S for C.  This is similar to `memchr' but there is no
   length limit.  */
# ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" void *rawmemchr (void *__s, int __c)
     __THROW __asm ("rawmemchr") __attribute_pure__ __nonnull ((1));
extern "C++" const void *rawmemchr (const void *__s, int __c)
     __THROW __asm ("rawmemchr") __attribute_pure__ __nonnull ((1));
# else
extern void *rawmemchr (const void *__s, int __c)
     __THROW __attribute_pure__ __nonnull ((1));
# endif

/* Search N bytes of S for the final occurrence of C.  */
# ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" void *memrchr (void *__s, int __c, size_t __n)
      __THROW __asm ("memrchr") __attribute_pure__ __nonnull ((1))
      __attr_access ((__read_only__, 1, 3));
extern "C++" const void *memrchr (const void *__s, int __c, size_t __n)
      __THROW __asm ("memrchr") __attribute_pure__ __nonnull ((1))
      __attr_access ((__read_only__, 1, 3));
# else
extern void *memrchr (const void *__s, int __c, size_t __n)
      __THROW __attribute_pure__ __nonnull ((1))
      __attr_access ((__read_only__, 1, 3));
# endif
#endif


/* Copy SRC to DEST.  */
extern char *strcpy (char *__restrict __dest, const char *__restrict __src)
     __THROW __nonnull ((1, 2));
/* Copy no more than N characters of SRC to DEST.  */
extern char *strncpy (char *__restrict __dest,
		      const char *__restrict __src, size_t __n)
     __THROW __nonnull ((1, 2));

/* Append SRC onto DEST.  */
extern char *strcat (char *__restrict __dest, const char *__restrict __src)
     __THROW __nonnull ((1, 2));
/* Append no more than N characters from SRC onto DEST.  */
extern char *strncat (char *__restrict __dest, const char *__restrict __src,
		      size_t __n) __THROW __nonnull ((1, 2));

/* Compare S1 and S2.  */
extern int strcmp (const char *__s1, const char *__s2)
     __THROW __attribute_pure__ __nonnull ((1, 2));
/* Compare N characters of S1 and S2.  */
extern int strncmp (const char *__s1, const char *__s2, size_t __n)
     __THROW __attribute_pure__ __nonnull ((1, 2));

/* Compare the collated forms of S1 and S2.  */
extern int strcoll (const char *__s1, const char *__s2)
     __THROW __attribute_pure__ __nonnull ((1, 2));
/* Put a transformation of SRC into no more than N bytes of DEST.  */
extern size_t strxfrm (char *__restrict __dest,
		       const char *__restrict __src, size_t __n)
    __THROW __nonnull ((2)) __attr_access ((__write_only__, 1, 3));

#ifdef __USE_XOPEN2K8
/* POSIX.1-2008 extended locale interface (see locale.h).  */
# include <bits/types/locale_t.h>

/* Compare the collated forms of S1 and S2, using sorting rules from L.  */
extern int strcoll_l (const char *__s1, const char *__s2, locale_t __l)
     __THROW __attribute_pure__ __nonnull ((1, 2, 3));
/* Put a transformation of SRC into no more than N bytes of DEST,
   using sorting rules from L.  */
extern size_t strxfrm_l (char *__dest, const char *__src, size_t __n,
			 locale_t __l) __THROW __nonnull ((2, 4))
     __attr_access ((__write_only__, 1, 3));
#endif

#if (defined __USE_XOPEN_EXTENDED || defined __USE_XOPEN2K8	\
     || __GLIBC_USE (LIB_EXT2) || __GLIBC_USE (ISOC2X))
/* Duplicate S, returning an identical malloc'd string.  */
extern char *strdup (const char *__s)
     __THROW __attribute_malloc__ __nonnull ((1));
#endif

/* Return a malloc'd copy of at most N bytes of STRING.  The
   resultant string is terminated even if no null terminator
   appears before STRING[N].  */
#if defined __USE_XOPEN2K8 || __GLIBC_USE (LIB_EXT2) || __GLIBC_USE (ISOC2X)
extern char *strndup (const char *__string, size_t __n)
     __THROW __attribute_malloc__ __nonnull ((1));
#endif

#if defined __USE_GNU && defined __GNUC__
/* Duplicate S, returning an identical alloca'd string.  */
# define strdupa(s)							      \
  (__extension__							      \
    ({									      \
      const char *__old = (s);						      \
      size_t __len = strlen (__old) + 1;				      \
      char *__new = (char *) __builtin_alloca (__len);			      \
      (char *) memcpy (__new, __old, __len);				      \
    }))

/* Return an alloca'd copy of at most N bytes of string.  */
# define strndupa(s, n)							      \
  (__extension__							      \
    ({									      \
      const char *__old = (s);						      \
      size_t __len = strnlen (__old, (n));				      \
      char *__new = (char *) __builtin_alloca (__len + 1);		      \
      __new[__len] = '\0';						      \
      (char *) memcpy (__new, __old, __len);				      \
    }))
#endif

/* Find the first occurrence of C in S.  */
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++"
{
extern char *strchr (char *__s, int __c)
     __THROW __asm ("strchr") __attribute_pure__ __nonnull ((1));
extern const char *strchr (const char *__s, int __c)
     __THROW __asm ("strchr") __attribute_pure__ __nonnull ((1));

# ifdef __OPTIMIZE__
__extern_always_inline char *
strchr (char *__s, int __c) __THROW
{
  return __builtin_strchr (__s, __c);
}

__extern_always_inline const char *
strchr (const char *__s, int __c) __THROW
{
  return __builtin_strchr (__s, __c);
}
# endif
}
#else
extern char *strchr (const char *__s, int __c)
     __THROW __attribute_pure__ __nonnull ((1));
#endif
/* Find the last occurrence of C in S.  */
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++"
{
extern char *strrchr (char *__s, int __c)
     __THROW __asm ("strrchr") __attribute_pure__ __nonnull ((1));
extern const char *strrchr (const char *__s, int __c)
     __THROW __asm ("strrchr") __attribute_pure__ __nonnull ((1));

# ifdef __OPTIMIZE__
__extern_always_inline char *
strrchr (char *__s, int __c) __THROW
{
  return __builtin_strrchr (__s, __c);
}

__extern_always_inline const char *
strrchr (const char *__s, int __c) __THROW
{
  return __builtin_strrchr (__s, __c);
}
# endif
}
#else
extern char *strrchr (const char *__s, int __c)
     __THROW __attribute_pure__ __nonnull ((1));
#endif

#ifdef __USE_MISC
/* This function is similar to `strchr'.  But it returns a pointer to
   the closing NUL byte in case C is not found in S.  */
# ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" char *strchrnul (char *__s, int __c)
     __THROW __asm ("strchrnul") __attribute_pure__ __nonnull ((1));
extern "C++" const char *strchrnul (const char *__s, int __c)
     __THROW __asm ("strchrnul") __attribute_pure__ __nonnull ((1));
# else
extern char *strchrnul (const char *__s, int __c)
     __THROW __attribute_pure__ __nonnull ((1));
# endif
#endif

/* Return the length of the initial segment of S which
   consists entirely of characters not in REJECT.  */
extern size_t strcspn (const char *__s, const char *__reject)
     __THROW __attribute_pure__ __nonnull ((1, 2));
/* Return the length of the initial segment of S which
   consists entirely of characters in ACCEPT.  */
extern size_t strspn (const char *__s, const char *__accept)
     __THROW __attribute_pure__ __nonnull ((1, 2));
/* Find the first occurrence in S of any character in ACCEPT.  */
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++"
{
extern char *strpbrk (char *__s, const char *__accept)
     __THROW __asm ("strpbrk") __attribute_pure__ __nonnull ((1, 2));
extern const char *strpbrk (const char *__s, const char *__accept)
     __THROW __asm ("strpbrk") __attribute_pure__ __nonnull ((1, 2));

# ifdef __OPTIMIZE__
__extern_always_inline char *
strpbrk (char *__s, const char *__accept) __THROW
{
  return __builtin_strpbrk (__s, __accept);
}

__extern_always_inline const char *
strpbrk (const char *__s, const char *__accept) __THROW
{
  return __builtin_strpbrk (__s, __accept);
}
# endif
}
#else
extern char *strpbrk (const char *__s, const char *__accept)
     __THROW __attribute_pure__ __nonnull ((1, 2));
#endif
/* Find the first occurrence of NEEDLE in HAYSTACK.  */
#ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++"
{
extern char *strstr (char *__haystack, const char *__needle)
     __THROW __asm ("strstr") __attribute_pure__ __nonnull ((1, 2));
extern const char *strstr (const char *__haystack, const char *__needle)
     __THROW __asm ("strstr") __attribute_pure__ __nonnull ((1, 2));

# ifdef __OPTIMIZE__
__extern_always_inline char *
strstr (char *__haystack, const char *__needle) __THROW
{
  return __builtin_strstr (__haystack, __needle);
}

__extern_always_inline const char *
strstr (const char *__haystack, const char *__needle) __THROW
{
  return __builtin_strstr (__haystack, __needle);
}
# endif
}
#else
extern char *strstr (const char *__haystack, const char *__needle)
     __THROW __attribute_pure__ __nonnull ((1, 2));
#endif


/* Divide S into tokens separated by characters in DELIM.  */
extern char *strtok (char *__restrict __s, const char *__restrict __delim)
     __THROW __nonnull ((2));

/* Divide S into tokens separated by characters in DELIM.  Information
   passed between calls are stored in SAVE_PTR.  */
extern char *__strtok_r (char *__restrict __s,
			 const char *__restrict __delim,
			 char **__restrict __save_ptr)
     __THROW __nonnull ((2, 3));
#ifdef __USE_POSIX
extern char *strtok_r (char *__restrict __s, const char *__restrict __delim,
		       char **__restrict __save_ptr)
     __THROW __nonnull ((2, 3));
#endif

#ifdef __USE_MISC
/* Similar to `strstr' but this function ignores the case of both strings.  */
# ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" char *strcasestr (char *__haystack, const char *__needle)
     __THROW __asm ("strcasestr") __attribute_pure__ __nonnull ((1, 2));
extern "C++" const char *strcasestr (const char *__haystack,
				     const char *__needle)
     __THROW __asm ("strcasestr") __attribute_pure__ __nonnull ((1, 2));
# else
extern char *strcasestr (const char *__haystack, const char *__needle)
     __THROW __attribute_pure__ __nonnull ((1, 2));
# endif
#endif

#ifdef __USE_MISC
/* Find the first occurrence of NEEDLE in HAYSTACK.
   NEEDLE is NEEDLELEN bytes long;
   HAYSTACK is HAYSTACKLEN bytes long.  */
extern void *memmem (const void *__haystack, size_t __haystacklen,
		     const void *__needle, size_t __needlelen)
     __THROW __attribute_pure__ __nonnull ((1, 3))
    __attr_access ((__read_only__, 1, 2))
    __attr_access ((__read_only__, 3, 4));

/* Copy N bytes of SRC to DEST, return pointer to bytes after the
   last written byte.  */
extern void *__mempcpy (void *__restrict __dest,
			const void *__restrict __src, size_t __n)
     __THROW __nonnull ((1, 2));
extern void *mempcpy (void *__restrict __dest,
		      const void *__restrict __src, size_t __n)
     __THROW __nonnull ((1, 2));
#endif


/* Return the length of S.  */
extern size_t strlen (const char *__s)
     __THROW __attribute_pure__ __nonnull ((1));

#ifdef	__USE_XOPEN2K8
/* Find the length of STRING, but scan at most MAXLEN characters.
   If no '\0' terminator is found in that many characters, return MAXLEN.  */
extern size_t strnlen (const char *__string, size_t __maxlen)
     __THROW __attribute_pure__ __nonnull ((1));
#endif


/* Return a string describing the meaning of the `errno' code in ERRNUM.  */
extern char *strerror (int __errnum) __THROW;
#ifdef __USE_XOPEN2K
/* Reentrant version of `strerror'.
   There are 2 flavors of `strerror_r', GNU which returns the string
   and may or may not use the supplied temporary buffer and POSIX one
   which fills the string into the buffer.
   To use the POSIX version, -D_XOPEN_SOURCE=600 or -D_POSIX_C_SOURCE=200112L
   without -D_GNU_SOURCE is needed, otherwise the GNU version is
   preferred.  */
# if defined __USE_XOPEN2K && !defined __USE_GNU
/* Fill BUF with a string describing the meaning of the `errno' code in
   ERRNUM.  */
#  ifdef __REDIRECT_NTH
extern int __REDIRECT_NTH (strerror_r,
			   (int __errnum, char *__buf, size_t __buflen),
			   __xpg_strerror_r) __nonnull ((2))
    __attr_access ((__write_only__, 2, 3));
#  else
extern int __xpg_strerror_r (int __errnum, char *__buf, size_t __buflen)
     __THROW __nonnull ((2)) __attr_access ((__write_only__, 2, 3));
#   define strerror_r __xpg_strerror_r
#  endif
# else
/* If a temporary buffer is required, at most BUFLEN bytes of BUF will be
   used.  */
extern char *strerror_r (int __errnum, char *__buf, size_t __buflen)
     __THROW __nonnull ((2)) __wur  __attr_access ((__write_only__, 2, 3));
# endif

# ifdef __USE_GNU
/* Return a string describing the meaning of tthe error in ERR.  */
extern const char *strerrordesc_np (int __err) __THROW;
/* Return a string with the error name in ERR.  */
extern const char *strerrorname_np (int __err) __THROW;
# endif
#endif

#ifdef __USE_XOPEN2K8
/* Translate error number to string according to the locale L.  */
extern char *strerror_l (int __errnum, locale_t __l) __THROW;
#endif

#ifdef __USE_MISC
# include <strings.h>

/* Set N bytes of S to 0.  The compiler will not delete a call to this
   function, even if S is dead after the call.  */
extern void explicit_bzero (void *__s, size_t __n) __THROW __nonnull ((1))
    __fortified_attr_access (__write_only__, 1, 2);

/* Return the next DELIM-delimited token from *STRINGP,
   terminating it with a '\0', and update *STRINGP to point past it.  */
extern char *strsep (char **__restrict __stringp,
		     const char *__restrict __delim)
     __THROW __nonnull ((1, 2));
#endif

#ifdef	__USE_XOPEN2K8
/* Return a string describing the meaning of the signal number in SIG.  */
extern char *strsignal (int __sig) __THROW;

# ifdef __USE_GNU
/* Return an abbreviation string for the signal number SIG.  */
extern const char *sigabbrev_np (int __sig) __THROW;
/* Return a string describing the meaning of the signal number in SIG,
   the result is not translated.  */
extern const char *sigdescr_np (int __sig) __THROW;
# endif

/* Copy SRC to DEST, returning the address of the terminating '\0' in DEST.  */
extern char *__stpcpy (char *__restrict __dest, const char *__restrict __src)
     __THROW __nonnull ((1, 2));
extern char *stpcpy (char *__restrict __dest, const char *__restrict __src)
     __THROW __nonnull ((1, 2));

/* Copy no more than N characters of SRC to DEST, returning the address of
   the last character written into DEST.  */
extern char *__stpncpy (char *__restrict __dest,
			const char *__restrict __src, size_t __n)
     __THROW __nonnull ((1, 2));
extern char *stpncpy (char *__restrict __dest,
		      const char *__restrict __src, size_t __n)
     __THROW __nonnull ((1, 2));
#endif

#ifdef __USE_MISC
/* Copy at most N - 1 characters from SRC to DEST.  */
extern size_t strlcpy (char *__restrict __dest,
		       const char *__restrict __src, size_t __n)
  __THROW __nonnull ((1, 2)) __attr_access ((__write_only__, 1, 3));

/* Append SRC to DEST, possibly with truncation to keep the total size
   below N.  */
extern size_t strlcat (char *__restrict __dest,
		       const char *__restrict __src, size_t __n)
  __THROW __nonnull ((1, 2))  __attr_access ((__read_write__, 1, 3));
#endif

#ifdef	__USE_GNU
/* Compare S1 and S2 as strings holding name & indices/version numbers.  */
extern int strverscmp (const char *__s1, const char *__s2)
     __THROW __attribute_pure__ __nonnull ((1, 2));

/* Sautee STRING briskly.  */
extern char *strfry (char *__string) __THROW __nonnull ((1));

/* Frobnicate N bytes of S.  */
extern void *memfrob (void *__s, size_t __n) __THROW __nonnull ((1))
    __attr_access ((__read_write__, 1, 2));

# ifndef basename
/* Return the file name within directory of FILENAME.  We don't
   declare the function if the `basename' macro is available (defined
   in <libgen.h>) which makes the XPG version of this function
   available.  */
#  ifdef __CORRECT_ISO_CPP_STRING_H_PROTO
extern "C++" char *basename (char *__filename)
     __THROW __asm ("basename") __nonnull ((1));
extern "C++" const char *basename (const char *__filename)
     __THROW __asm ("basename") __nonnull ((1));
#  else
extern char *basename (const char *__filename) __THROW __nonnull ((1));
#  endif
# endif
#endif

#if __GNUC_PREREQ (3,4)
# if __USE_FORTIFY_LEVEL > 0 && defined __fortify_function
/* Functions with security checks.  */
#  include <bits/string_fortified.h>
# endif
#endif

__END_DECLS

#endif /* string.h  */
