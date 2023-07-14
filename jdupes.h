/* jdupes main program header
 * See jdupes.c for license information */

#ifndef JDUPES_H
#define JDUPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Detect Windows and modify as needed */
#if !defined NO_WINDOWS && (defined _WIN32 || defined __MINGW32__)
 #ifndef ON_WINDOWS
  #define ON_WINDOWS 1
 #endif
 #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
 #endif
 #include <windows.h>
 #include <io.h>
#endif /* Win32 */

#include <limits.h>
#include <stdint.h>
#include <sys/types.h>

/* Some types are different on Windows */
#if defined _WIN32 || defined __MINGW32__
 typedef uint64_t jdupes_ino_t;
 typedef uint32_t jdupes_mode_t;
 #ifdef UNICODE
  extern const wchar_t *FILE_MODE_RO;
 #else
  extern const char *FILE_MODE_RO;
 #endif /* UNICODE */

#else /* Not Windows */
 #include <sys/stat.h>
 typedef ino_t jdupes_ino_t;
 typedef mode_t jdupes_mode_t;
 extern const char *FILE_MODE_RO;
 #ifdef UNICODE
  #error Do not define UNICODE on non-Windows platforms.
  #undef UNICODE
 #endif
#endif /* _WIN32 || __MINGW32__ */

/* Windows + Unicode compilation */
#ifdef UNICODE
 #ifndef PATHBUF_SIZE
  #ifndef WPATH_MAX
   #define WPATH_MAX 8192
  #endif
  #define PATHBUF_SIZE WPATH_MAX
 #else
  #ifndef WPATH_MAX
   #define WPATH_MAX PATHBUF_SIZE
  #endif
 #endif /* PATHBUF_SIZE */
 typedef wchar_t wpath_t[WPATH_MAX];
 #define M2W(a,b) MultiByteToWideChar(CP_UTF8, 0, a, -1, (LPWSTR)b, WPATH_MAX)
 #define W2M(a,b) WideCharToMultiByte(CP_UTF8, 0, a, -1, (LPSTR)b, WPATH_MAX, NULL, NULL)
 extern wpath_t wstr;
#endif /* UNICODE */

/* Maximum path buffer size to use; must be large enough for a path plus
 * any work that might be done to the array it's stored in. PATH_MAX is
 * not always true. Read this article on the false promises of PATH_MAX:
 * http://insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html
 * Windows + Unicode needs a lot more space than UTF-8 in Linux/Mac OS X
 */
#ifndef PATHBUF_SIZE
 #define PATHBUF_SIZE 4096
#endif
/* Complain if PATHBUF_SIZE is too small */
#if PATHBUF_SIZE < PATH_MAX
 #if !defined LOW_MEMORY && !defined BARE_BONES
  #warning "PATHBUF_SIZE is less than PATH_MAX"
 #endif
#endif

/* Flag operations */
#define ISFLAG(a,b) ((a & b) == b)
#define SETFLAG(a,b) (a |= b)
#define CLEARFLAG(a,b) (a &= (~b))


/* Normal and aggressive ("loud") debugging */
#ifdef DEBUG
 #define DBG(a) a
#else
 #define DBG(a)
#endif
#ifdef LOUD_DEBUG
 #ifndef DEBUG
  #define DEBUG
 #endif
 #define LOUD(...) if ISFLAG(flags, F_LOUD) __VA_ARGS__
#else
 #define LOUD(a)
#endif

#ifndef PARTIAL_HASH_SIZE
 #define PARTIAL_HASH_SIZE 4096
#endif


/* Program-wide behavior flags */
extern uint64_t flags;
#define F_HIDEPROGRESS		(1ULL << 0)
#define F_RECURSE		(1ULL << 1)


/* Per-inode information - stat() etc. */
typedef struct _inode {
  uint64_t filehash_partial;
  uint64_t filehash;
  jdupes_mode_t mode;
  off_t size;
  dev_t device;
  jdupes_ino_t inode;
#ifndef NO_MTIME
  time_t mtime;
#endif
#ifndef NO_ATIME
  time_t atime;
#endif
#ifndef NO_USER_ORDER
  unsigned int user_order; /* Order of the originating command-line parameter */
#endif
#ifndef NO_HARDLINKS
 #ifdef ON_WINDOWS
  uint32_t nlink;  /* link count on Windows is always a DWORD */
 #else
  nlink_t nlink;
 #endif /* ON_WINDOWS */
#endif
#ifndef NO_PERMS
  uid_t uid;
  gid_t gid;
#endif
} inode_t;


/* Per-file information */
typedef struct _file {
  struct _file *next;
  inode_t *inode;
  char *d_name;
#ifndef NO_USER_ORDER
  unsigned int user_order; /* Order of the originating command-line parameter */
#endif
  uint32_t flags;  /* jdupes file status flags */
} file_t;


/* Match pairs */
typedef struct _match {
  file_t *file1;
  file_t *file2;
  uint32_t m_flags;
} match_t;


/* Variables */
extern char *program_name;
extern int exit_status;


#ifdef __cplusplus
}
#endif

#endif /* JDUPES_H */
