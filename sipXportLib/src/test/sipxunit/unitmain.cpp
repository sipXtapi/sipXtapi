//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

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
