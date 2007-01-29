//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <sipxunit/TestRunner.h>

#if defined(_VXWORKS)
#include <usrLib.h>
#endif

//
//  External API into Device code
#if defined(_VXWORKS)
#ifdef __cplusplus
extern "C" {
#endif

int cpptest_main( void );
int cpptestvx( void );


#ifdef __cplusplus
}
#endif
#endif



/**
 * include this in your project and it will find all Suite's that have been
 * registered with the CppUnit TextFactoryRegistry and run them
 */
#if defined(_VXWORKS)
//
// spawning task to use LARGE stack size, 20K was not enough!!!
//
int cpptestvx( void )
{
   int gCppTestTaskID = taskSpawn(

            "UnitTest", 
            100,
            0, 
            (512*1024),
            (FUNCPTR)cpptest_main, 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

   assert(gCppTestTaskID  != ERROR);
 
   return gCppTestTaskID;
}
#endif


#if defined(_VXWORKS)
int cpptest_main( void )
#else
#if defined(WINCE)
int main( int argc, char* argv[] );

#if defined(WINCE6)
int wmain(int argc, wchar_t* argv[])
{
	printf( "entering wmain( )\n" );
	int		iRet		= 1;
	iRet = main( 0, NULL );

	printf( "  main( ) returned %d\n", iRet );
	return iRet;
}
#endif
// wWinMain is not defined in winbase.h.
//extern "C" int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd);
//****************************************************************
int
WINAPI
WinMain(
		HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPWSTR lpCmdLine,
		int nShowCmd
	   )
{
	printf( "entering WinMain( ) - lpCmdLine is *%s*\n", lpCmdLine );
	wchar_t	*pW			= NULL;
	int		iRet		= 1;


	iRet = main( 0, NULL );

	printf( "  main( ) returned %d\n", iRet );
	return iRet;
}

#endif
int main( int argc, char* argv[] )
#endif
{
    TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    bool wasSucessful = runner.run();

#if defined(_VXWORKS)
    // check the stack size using vxworks built-in function
    // ti( taskIdSelf() );
    taskDelay(100);

    // check all stack sizes
    checkStack(0);
#endif

    return !wasSucessful;
}


void forceUnitTest( )
{
       

}
