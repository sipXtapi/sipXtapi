// 
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
// 
// Copyright (C) 2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <string.h>
#include <stdio.h>

//  GetProcAddress( )
#include <winbase.h>
#include <assert.h>
#include <time.h>

//  The assert( ) macro explicitly uses OutputDebugstringW( ), which explicitly overrides our _MBCS compiler flag.
//  Therefore, we need to explicitly fix this...
#undef ASSERT_PRINT
#undef ASSERT_AT
#undef assert
#define ASSERT_PRINT(exp,file,line) OutputDebugStringB(TEXT("\r\n*** ASSERTION FAILED in ") TEXT(file) TEXT("(") TEXT(#line) TEXT("):\r\n") TEXT(#exp) TEXT("\r\n"))
#define ASSERT_AT(exp,file,line) (void)( (exp) || (ASSERT_PRINT(exp,file,line), DebugBreak(), 0 ) )
#define assert(exp) ASSERT_AT(exp,__FILE__,__LINE__)


typedef long intptr_t;

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

#define	EACCES						1
#define EMFILE						2
#define ENOENT						3

//#define OutputDebugstringW(x)		OutputDebugStringA(x)
#define GetProcAddressW(x, y)		GetProcAddressA(x, y)

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
#ifdef PLUGIN_HOOKS

int     errno						= 1;
char	*_tzname[ 2 ] = {"DST","STD"};


BOOL GetVersionExA( LPOSVERSIONINFOA lpVerInfo )
{
	return FALSE;
}

BOOL CreateProcessB( char *pName,
					   char *pCmdLine,
					   LPSECURITY_ATTRIBUTES lpsaProcess,
					   LPSECURITY_ATTRIBUTES lpsaThread,
					   BOOL fInheritHandles,
					   DWORD dwCreate,
					   LPVOID lpvEnv,
					   char *pCurDir,
					   LPSTARTUPINFO lpStartUpInfo,
					   LPPROCESS_INFORMATION lpProcInfo )
{
	return NULL;
}


HANDLE CreateSemaphoreB( LPSECURITY_ATTRIBUTES lpSemaphoreAttr, LONG lInitialCount, LONG lMaxCount, char *pName )
{
	return NULL;
}


HANDLE CreateMutexB( LPSECURITY_ATTRIBUTES lpMutexAttr, BOOL bInitialOwner, char *pNme )
{
	return NULL;
}


HANDLE CreateFileB( const char *pFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
					LPSECURITY_ATTRIBUTES lpSecAttr, DWORD dwCreationDisp, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile )
{
	return NULL;
}

DWORD FormatMessageB( DWORD dwFlags, void *lpVoid, DWORD dwMessageID, DWORD dwLangID, char *pBuffer, DWORD dwSize, va_list *Args )
{
	return 0;
}

struct tm * __cdecl localtime( const time_t *timer )
{
	static struct tm staticTM;
	return &staticTM;
}

int time( int iIn )
{
	return 0;
}

int _time( int iIn )
{
	return 0;
}


//****************************************************************
long RegQueryValueExB( HKEY hKey, const char *lpName, DWORD *lpReserved, DWORD *lpType, unsigned char *lpData, DWORD *lpcbData )
{
	return 0;
}


//****************************************************************
long RegOpenKeyExA( HKEY hKey, const char *pSub, DWORD dwOptions, REGSAM samDesired, PHKEY phkResults )
{
	return NULL;
}


//****************************************************************
HINSTANCE LoadLibraryExA( const char *pIn, HANDLE hIn, DWORD dwIn )
{
	return NULL;
}



//****************************************************************
HINSTANCE LoadLibraryA( const char *pIn )
{
	return NULL;
}



//****************************************************************
HMODULE GetModuleHandleA( const char *pIn )
{
	return NULL;
}


//****************************************************************
void OutputDebugStringB( const char *pC )
{
	printf( "OutputDebugStringA( ) NOT IMPLEMENTED\n" );
	assert( 0 );
}


//****************************************************************
//void WINAPI OutputDebugStringW( const unsigned short *pC )
//{
//	printf( "OutputDebugStringW( ) NOT IMPLEMENTED\n" );
//	assert( 0 );
//}


//****************************************************************
int rmdir( const char * dirname )
{
	printf( "rmdir( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return -1;
}


//****************************************************************
char * getcwd( char *buffer, int maxlen )
{
	printf( "getcwd( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return NULL;
}


//****************************************************************
unsigned int GetSystemDirectory( char *cP, unsigned int uSize )
{
	printf( "GetSystemDirectory( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return 0;
}

//****************************************************************
void GetSystemTimeAsFileTime( LPFILETIME pFT )
{
	printf( "GetSystemTimeAsFileTime( ) NOT IMPLEMENTED\n" );
	assert( 0 );
}


//****************************************************************
void WINAPI perror( char *pC )
{
	//  Just a stub...
}


//****************************************************************
int SetErrorMode( int iUnused )
{
	//  Just a stub...
	return 0;
}


//****************************************************************
char * WINAPI strerror( int iErrNo )
{
	//  Just a stub...
	static char cErrMsg[ 100 ] = { 0 };

	if( iErrNo  ||  strlen( cErrMsg ) == 0 )
		sprintf( cErrMsg, "Unable to lookup error message for error %d\n", iErrNo );

	return cErrMsg;
}


//****************************************************************
int _chdir( const char *p1 )
{
	printf( "_chdir( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return 0;
}


//****************************************************************
int _mkdir( const char *p1 )
{
	printf( "_mkdir( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return 0;
}


//****************************************************************
int rename( const char *p1, const char *p2 )
{
	printf( "rename( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return 0;
}


//****************************************************************
int remove( const char *p1 )
{
	printf( "remove( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return 0;
}

//#define _O_RDONLY		0x0000
//#define _O_WRONLY		0x0001
//#define _O_RDWR		0x0002
//#define _O_APPEND		0x0008
//
//#define _O_CREAT		0x0100
//#define _O_TRUNC		0x0200
//#define _O_EXCL		0x0400



//****************************************************************
int _open( const char *filename, int oflag )
{
	printf( "_open( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return -1;
}


//****************************************************************
int _close( int fd )
{
	printf( "_close( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return -1;
}


//****************************************************************

//typedef __int64 intptr_t;


//****************************************************************
void * _findclose( long hFile )
{
	printf( "_findclose( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return NULL;
}


//****************************************************************
intptr_t _findnext( long hFile, struct _finddata_t *pFD )
{
	printf( "_findnext( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return NULL;
}



//****************************************************************
intptr_t _findfirst( const char *pName, struct _finddata_t *pFD )
{
	printf( "_findfirst( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return NULL;
}



//****************************************************************
HANDLE GetStdHandle( int nStdHandle )
{
	printf( "GetStdHandle( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return INVALID_HANDLE_VALUE;
}


//****************************************************************
int _putenv( const char *pIn )
{
	printf( "_putenv( ) NOT IMPLEMENTED\n" );
	assert( 0 );
	return -1;
}


#else

extern char		*_tzname[ 2 ];
extern int		errno;

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

HINSTANCE		LoadLibraryA( const char *pIn );

int				rmdir( const char * dirname );
char			* getcwd( char *buffer, int maxlen );
unsigned int	GetSystemDirectory( char *cP, unsigned int uSize );
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


#endif

