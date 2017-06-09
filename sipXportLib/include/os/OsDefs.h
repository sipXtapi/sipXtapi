/*
// 
// 
// Copyright (C) 2005-2017 SIPez LLC.
//
// Copyright (C) 2004-2009 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
*/

/* NOTE THIS FILE MUST BE INCLUDABLE IN C CODE as well as C++ */
/* USE C Style comments */

#ifndef _OsDefs_h_
#define _OsDefs_h_

/* SYSTEM INCLUDES */
/* APPLICATION INCLUDES  */
#include <os/OsIntTypes.h>

#ifdef _VXWORKS
#  include "os/Vxw/OsVxwDefs.h"
#endif /* _VXWORKS */
#ifdef __pingtel_on_posix__
#  include "os/linux/OsLinuxDefs.h"
#endif /* __pingtel_on_posix__ */

/* MACROS                */
#if defined(_VXWORKS)
   /* wdn - OK == 0 defined in vxWorks.h but has issues ??? */
#  define IS_INET_RETURN_OK( x )    (x == 0)
#else
#  define IS_INET_RETURN_OK( x )    (x > 0)
#endif

/*
* S_IREAD and S_IWRITE are not always defined, e.g. they're not
* defined in bionic (Android's libc).
*/
#ifdef DEFINE_S_IREAD_IWRITE
#  define S_IREAD  (S_IRUSR | S_IRGRP | S_IROTH)
#  define S_IWRITE (S_IWUSR)
#endif

// O_BINARY is needed for WIN32, but is not defined under VxWorks and Linux
#ifdef __pingtel_on_posix__
#  define O_BINARY 0
#endif

/*
* If we're compiling for windows using a visual studio prior to VS2008
* http:*www.casabasecurity.com/content/visual-studio-2008-crt-bug
* Basically, prior versions of Visual Studio did not define snprintf
* or vsnprintf, because they were not ANSI compliant.
* Even now, with VS2008, they aren't compliant, however visual studio
* has gone about and defined vsnprintf.
* http:*connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=101293
*/
#if defined(WIN32) && defined(_MSC_VER)

#if (_MSC_VER < 1900)
#  define snprintf _snprintf
#endif

#if (_MSC_VER < 1500)
#  define vsnprintf _vsnprintf
#endif

#define popen _popen
#define pclose _pclose
/*
* WIN32 doesn't have wait(), so the return value for children
* is simply the return value specified by the child, without
* any additional information on whether the child terminated
* on its own or via a signal.  These macros are also used
* to interpret the return value of system().
*/
#define WEXITSTATUS(w) (w)
#define WIFEXITED(w) (true)
#define WIFSIGNALED(w) (false)
#define WTERMSIG(w) (0)
#endif

#if defined(va_copy)
#elif defined(__va_copy)
#  define va_copy(dst, src) __va_copy((dst), (src))
#else
//#  define va_copy(dst, src) (memcpy(&(dst), &(src), sizeof (va_list)))
#  define va_copy(dst, src) ((dst) = (src))
#endif



/*
 * Handle the case-insensitive string comparison functions, by making 
 * the Posix names strcasecmp and strncasecmp available on all platforms.
 * (On newer Windows environments, str(n)casecmp are built-in, along 
 * with the older str(n)icmp, but on older ones, they are not.) 
 */
#ifdef WIN32
   /* if wince, or win and >= msvc8(vs2005) */
#  if defined(WINCE) || (defined(_MSC_VER) && (_MSC_VER >= 1400))
#     ifndef strcasecmp
#        define strcasecmp _stricmp
#     endif
#     ifndef strncasecmp
#        define strncasecmp _strnicmp
#     endif
#  else
#     ifndef strcasecmp
#        define strcasecmp stricmp
#     endif
#     ifndef strncasecmp
#        define strncasecmp strnicmp
#     endif
#  endif
#endif


/* Define min and max if they're not already defined. */
#ifndef sipx_max
#  define sipx_max(x,y) (((x)>(y))?(x):(y))
#endif

#ifndef sipx_min
#  define sipx_min(x,y) (((x)<(y))?(x):(y))
#endif

#ifdef __cplusplus
   extern "C" {
#endif

#if defined (_VXWORKS)  /*  Only needed for VxWorks --GAT */
   int strcasecmp(const char *, const char *);
   char * strdup (const char *);

   /* These function names are for code compatibility with Windows. --GAT */
#  ifndef strcmpi
#     define strcmpi strcasecmp
#  endif
#  ifndef stricmp
#     define stricmp strcasecmp
#  endif
#  ifndef _stricmp
#     define _stricmp strcasecmp
#  endif
#endif /* _VXWORKS */

extern unsigned int pspGetLocalMemLocalAddr(void);
extern unsigned int pspGetLocalMemSize(void);

#define SysLowerMemoryLimit   (pspGetLocalMemLocalAddr())
#define SysUpperMemoryLimit   (pspGetLocalMemLocalAddr() + pspGetLocalMemSize() - 4)


extern int hSipLogId;
void enableConsoleOutput(int bEnable) ;
void osPrintf(const char* format , ...)
#ifdef __GNUC__
   /* with the -Wformat switch, this enables format string checking */
   __attribute__ ((format (printf, 1, 2)))
#endif
;
         
/* A special value for "port number" which means that no port is specified. */
#define PORT_NONE (-1)

/*
 * A special value for "port number" which means that some default 
 * port number should be used.  The default may be defined by the 
 * situation, or the OS may choose a port number.
 * For use when PORT_NONE is used to mean "open no port", and in 
 * socket-opening calls.
 */
#define PORT_DEFAULT (-2)

/*
 * Macro to test a port number for validity as a real port (and not 
 * PORT_NONE or PORT_DEFAULT).  Note that 0 is a valid port number for 
 * the protocol, but the Berkeley sockets interface makes it 
 * impossible to specify it. In addition, RTP treats port 0 as a 
 * special value. Thus we forbid port 0.
 */
#define portIsValid(p) ((p) >= 1 && (p) <= 65535)

#ifdef __cplusplus
   }
#endif


/* Silence unused vailable warnings
 * May be present for the purpose of debugging */
#define SIPX_UNUSED(VAR) if(0){(void) VAR;}


#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "time.h"

#if defined(__sun) && defined(__SVR4)
#  include <strings.h>
#  include <sys/sockio.h>
#  include <netdb.h>
#  ifdef __cplusplus
      extern "C"
#  endif
   extern int getdomainname(char *, int);
#endif

#endif  /* _OsDefs_h_ */
