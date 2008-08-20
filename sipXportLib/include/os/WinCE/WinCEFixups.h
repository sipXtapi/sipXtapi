#ifndef _WINCEFIXUPS_H_ 
#define _WINCEFIXUPS_H_


// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#if _WIN32_WCE >= 0x0600
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <windows.h>
#include <string.h>
#include <stdio.h>

//  GetProcAddress( )
#include <winbase.h>
#include <assert.h>
#include <time.h>
#ifdef WINCE
#include <crtdefs.h>
#endif

//  The assert( ) macro explicitly uses OutputDebugstringW( ), which explicitly overrides our _MBCS compiler flag.
//  Therefore, we need to explicitly fix this...
#undef ASSERT_PRINT
#undef ASSERT_AT
#undef assert
#define ASSERT_PRINT(exp,file,line) OutputDebugStringB(TEXT("\r\n*** ASSERTION FAILED in ") TEXT(file) TEXT("(") TEXT(#line) TEXT("):\r\n") TEXT(#exp) TEXT("\r\n"))
#define ASSERT_AT(exp,file,line) (void)( (exp) || (ASSERT_PRINT(exp,file,line), DebugBreak(), 0 ) )
#define assert(exp) ASSERT_AT(exp,__FILE__,__LINE__)

#ifndef _WIN32_WCE
typedef long intptr_t;
#endif

#define sprintf_s _snprintf

struct _timeb
{
	time_t			time;
	unsigned short	millitm;
	short			timezone;
	short			dstflag;
};

#define stricmp(x, y )						_stricmp( (x), (y) )
#define strcasecmp _stricmp
#define strncasecmp _strnicmp

#undef RegQueryValueEx
#define RegQueryValueEx(x,y,z,a,b,c)		RegQueryValueExB(x,y,z,a,b,c)
#undef FormatMessage
#define FormatMessage(x,y,z,a,b,c,d)		FormatMessageB(x,y,z,a,b,c,d)
#undef CreateMutex
#define CreateMutex( x,y,z)					CreateMutexB( x,y,z)
#undef CreateSemaphore
#define CreateSemaphore( x,y,z,a)			CreateSemaphoreB( x,y,z,a)
#undef CreateProcess
#define CreateProcess(x,y,z,a,b,c,d,e,f,g)	CreateProcessB(x,y,z,a,b,c,d,e,f,g)
#undef CreateFile
#define CreateFile(x,y,z,a,b,c,d)			CreateFileB(x,y,z,a,b,c,d)
#undef OutputDebugString
#define OutputDebugString(x)				OutputDebugStringB(x)

#define SEM_FAILCRITICALERRORS		0

#define chdir(x)					_chdir(x)
#define strdup(x)					_strdup(x)
#define itoa(x, y, z)				_itoa(x, y, z)
#define memicmp(x, y, z)			_memicmp(x, y, z)
#define putenv(x)					_putenv(x)

#define	EACCES						13  // Orig 1
#define EMFILE						24  // Orig 2
#define ENOENT						2   // Orig 3		

//#define OutputDebugstringW(x)		OutputDebugStringA(x)
#define GetProcAddressW(x, y)		GetProcAddressA(x, y)

//  for the file res_query.c
#define BUFSIZ		2048
#define	EINVAL		22 // Orig 1234
#define EINTR		4  // Orig 1235

struct _finddata_t
{
	unsigned	attrib;
	time_t		time_create;
	time_t		time_access;
	time_t		time_write;
	//  JEP - TODO - fix this
//	_fsize_t	size;
	int     	size;
	char		name[ 260 ];
};

#define _A_NORMAL	0x00
#define _A_RDONLY	0x01
#define _A_HIDDEN	0x02
#define _A_SYSTEM	0x04
#define _A_SUBDIR	0x10
#define _A_ARCH		0x20

typedef struct _xFILETIME {
	int dwLowDateTime;
	int dwHighDateTime;
} *xLPFILETIME;
//} FILETIME, *PFILETIME, *LPFILETIME;

#define O_RDONLY	0x0000
#define O_WRONLY	0x0001
#define O_RDWR		0x0002
#define O_APPEND	0x0008

#define O_CREAT		0x0100
#define O_TRUNC		0x0200
#define O_EXCL		0x0400

#define O_TEXT		0x4000  /* file mode is text (translated) */
#define O_BINARY	0x8000  /* file mode is binary (untranslated) */

#define _S_IFMT		0x0001	// mask type
#define _S_IFDIR	0x0002	// directory
#define _S_IFCHR	0x0004	// Character special (indicates a device if set)
#define _S_IFREG	0x0008	// Regular
#define _S_IREAD	0x0010	// Read permission, owner
#define _S_IWRITE	0x0020	// Write permission, owner
#define _S_IEXEC	0x0040	// Execute/serach permission, owner

#define S_IFMT		0x0001	// mask type
#define S_IFDIR		0x0002	// directory
#define S_IFCHR		0x0004	// Character special (indicates a device if set)
#define S_IFREG		0x0008	// Regular
#define S_IREAD		0x0010	// Read permission, owner
#define S_IWRITE	0x0020	// Write permission, owner
#define S_IEXEC		0x0040	// Execute/serach permission, owner

struct stat
{
	int	st_atime;
	int	st_ctime;
	int	st_dev;
	int	st_mode;
	int	st_mtime;
	int	st_nlink;
	int	st_rdev;
	int	st_size;
};



#define STARTF_USESTDHANDLES		0x00000100


//#define INVALID_HANDLE_VALUE	-1
#define STD_INPUT_HANDLE		-10
#define STD_OUTPUT_HANDLE		-11
#define STD_ERROR_HANDLE		-12


//
//  To avoid the 2000+ warning messages at link time 
//  I need to define these functions only once.
//  So I picked the file PluginHooks.cpp to be my anchor.
//  The macro PLUGIN_HOOKS is defined only for this file.
//


extern char		*_tzname[ 2 ];

#ifdef __cplusplus
	extern "C" int errno;
#else
	extern int errno;
#endif

long			RegQueryValueExB( HKEY hKey, const char *lpName, DWORD *lpReserved, DWORD *lpType, unsigned char *lpData, DWORD *lpcbData );
HANDLE			CreateMutexB( LPSECURITY_ATTRIBUTES lpMutexAttr, BOOL bInitialOwner, char *pNme );
DWORD			FormatMessageB( DWORD dwFlags, void *lpVoid, DWORD dwMessageID, DWORD dwLangID, char *pBuffer, DWORD dwSize, va_list *Args );
HANDLE			CreateSemaphoreB( LPSECURITY_ATTRIBUTES lpSemaphoreAttr, LONG lInitialCount, LONG lMaxCount, char *pName );
HANDLE			CreateFileB( const char *pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
					LPSECURITY_ATTRIBUTES lpSecAttr, DWORD dwCreationDisp, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile );
BOOL			CreateProcessB( char *pName,
					   char *pCmdLine,
					   LPSECURITY_ATTRIBUTES lpsaProcess,
					   LPSECURITY_ATTRIBUTES lpsaThread,
					   BOOL fInheritHandles,
					   DWORD dwCreate,
					   LPVOID lpvEnv,
					   char *pCurDir,
					   LPSTARTUPINFO lpStartUpInfo,
					   LPPROCESS_INFORMATION lpProcInfo );

HINSTANCE  LoadLibraryA( LPCSTR pIn );
WINBASEAPI HMODULE WINAPI    GetModuleHandleA( LPCSTR pIn );

int				rmdir( const char * dirname );
char			* getcwd( char *buffer, int maxlen );
unsigned int	GetSystemDirectory( char *cP, unsigned int uSize );
void			_ftime( struct _timeb *);
void			GetSystemTimeAsFileTime( LPFILETIME pFT );
void			OutputDebugStringB( const char *pC );
//void WINAPI		OutputDebugStringW( char *pC );
void WINAPI		perror( char *pC );
int				SetErrorMode( int iUnused );
char * WINAPI	strerror( int iErrNo );
int				_chdir( const char *p1 );
int				_mkdir( const char *p1 );
int				rename( const char *p1, const char *p2 );
int				remove( const char *p1 );
int				_open( const char *filename, int oflag );
int				_close( int fd );
void *			_findclose( long hFile );
intptr_t		_findnext( long hFile, struct _finddata_t *pFD );
intptr_t		_findfirst( const char *pName, struct _finddata_t *pFD );
HANDLE			GetStdHandle( int nStdHandle );
int				_putenv( const char *pIn );
int				write( int fd, const void *buffer, unsigned int count );
int				open( const char *filename, int oflag );
#pragma warning( disable : 4031 )
int				open( const char *filename, int oflag, int pmode );
#pragma warning( default : 4031 )
int				close( int fd );
long			lseek( int fd, long offset, int origin );
int				fstat( int fd, struct stat *buffer );
int				read( int fd, void *buffer, unsigned int count );

#ifdef __cplusplus
	extern "C" int _getpid();
#else
	extern int  _getpid();
#endif

// Have to build stuff not just from WinBase but also from WinUser.h

BOOL PostThreadMessageA(DWORD idThread, UINT Msg,
                      WPARAM wParam,
                      LPARAM lParam);
int GetMessageA(LPMSG lpMsg,HWND hWnd,unsigned int wMsgFilterMin,unsigned int wMsgFilterMax) ;


/* The following functions have to tobe deffed and undefed this is not a mistake */

#undef CreateEventA
#define CreateEventA CE_CreateEventA
HANDLE CE_CreateEventA(
					  LPSECURITY_ATTRIBUTES lpEventAttributes, 
					  BOOL bManualReset, 
					  BOOL bInitialState, 
					  LPTSTR lpName) ;

#undef  RegOpenKeyExA
#define RegOpenKeyExA CE_RegOpenKeyExA
long CE_RegOpenKeyExA (	 HKEY hKey,
						 LPCSTR lpSubKey,
						 DWORD ulOptions,
						 REGSAM samDesired,
						 PHKEY phkResult
						);
/* Came from mmsystem.h */

WINMMAPI MMRESULT WINAPI timeSetEvent(UINT uDelay, UINT uResolution,
    LPTIMECALLBACK fptc, DWORD dwUser, UINT fuEvent);

/* waveform input device capabilities structure */
typedef struct tagWAVEINCAPS_W {
    WORD    wMid;                    /* manufacturer ID */
    WORD    wPid;                    /* product ID */
    MMVERSION vDriverVersion;        /* version of the driver */
    WCHAR   szPname[MAXPNAMELEN];    /* product name (NULL terminated string) */
    DWORD   dwFormats;               /* formats supported */
    WORD    wChannels;               /* number of channels supported */
    WORD    wReserved1;              /* structure packing */
} WAVEINCAPS_W;

#ifdef __cplusplus
extern "C" {
#endif

size_t strftime(char * const s, const size_t maxsize, const char * const format, const struct tm * const t);
time_t mktime(struct tm *t);
struct tm * __cdecl gmtime( const time_t *timer );
struct tm * __cdecl localtime( const time_t *timer );
time_t __cdecl time( time_t *ptt );

#ifdef __cplusplus
}
#endif

#endif // _WINCEFIXUPS_H_

