// 
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsCallback.h>
#include <os/OsTimer.h>
#include <os/OsTimerTask.h>
#include <os/OsTime.h>
#include <sipxunit/TestUtilities.h>
#include <os/OsDateTime.h>
#include <os/OsLock.h>
#include <os/OsEvent.h>
#include <os/OsTimerMsg.h>

#include <time.h>
#include <string.h>

/* System-dependent definition of "current time" structure. */

#if defined(WIN32) || defined(_VXWORKS)

typedef struct _GTimeVal
{
    long tv_usec;
    long tv_sec;
} GTimeVal;

#elif defined(__pingtel_on_posix__)

#include <sys/time.h>
typedef struct timeval GTimeVal;

#else

#error "Cannot determine system type."

#endif


/* System-dependent code to fetch the current time. */

#if defined(WIN32)

void g_get_current_time(GTimeVal* curTime)
{
    typedef union 
    {
        FILETIME         ft ;
        unsigned __int64 int64 ;    
    } g_FILETIME ;

    unsigned __int64 ticks ;
    unsigned __int64 freq ;
    static bool       sbInitialized = false ;
    static g_FILETIME sOsFileTime ;
    static unsigned __int64 sLastTicks = 0 ;
    static unsigned __int64 sResetTime = 0 ;

    QueryPerformanceCounter((LARGE_INTEGER*) &ticks) ;
    QueryPerformanceFrequency((LARGE_INTEGER*) &freq) ;

    if (!sbInitialized || sOsFileTime.int64 > sResetTime)
    {
        sbInitialized = true ;
        GetSystemTimeAsFileTime(&sOsFileTime.ft);
        sResetTime = -1 ; // sOsFileTime.int64 + (freq - 1) ;
        sLastTicks = ticks ;
    }
    else
    {
        unsigned __int64 delta = ticks - sLastTicks ;

        sLastTicks = ticks ;
        sOsFileTime.int64 = sOsFileTime.int64 + 
                (((unsigned __int64) 10000000) * (delta / freq)) + 
                (((unsigned __int64) 10000000) * (delta % freq)) / freq ;    
        
        SYSTEMTIME si ;
        FileTimeToSystemTime(&sOsFileTime.ft, &si) ;
    }
    curTime->tv_sec = (long)  ((sOsFileTime.int64 - ((unsigned __int64) 116444736000000000)) / ((unsigned __int64) 10000000));
    curTime->tv_usec = (long) ((sOsFileTime.int64 / ((unsigned __int64) 10)) % ((unsigned __int64) 1000000));
}

#elif defined(_VXWORKS)

void g_get_current_time(GTimeVal* curTime)
{
    OsTime rTime;

    OsDateTimeVxw::highResTime( rTime );

    curTime->tv_sec  = rTime.seconds();
    curTime->tv_usec = rTime.usecs();
}

#elif defined(__pingtel_on_posix__)

void g_get_current_time(GTimeVal* curTime)
{
   gettimeofday(curTime, NULL);
}

#endif

/*
 * OSTIMETOLERANCE is the allowed 'slop', in milliseconds; timers may
 * be off by this much before the test is considered a failure.  The
 * current value may be too high; see XPL-39.
 */
#define OSTIMETOLERANCE 40

#define REPORT_SKEW(x) printf x
//#define REPORT_SKEW(x) /* x */

using namespace std ; 

OsTime      tenMsec(0, 10000);// timer offset ten msec into the future
OsTime      hundredMsec(0, 100000);// timer offset hundred msec into the future
OsTime      oneSecond(1,0);   // timer offset one second into the future
OsTime      twoSeconds(2,0);  // timer offset two seconds into the future
OsTime      tenSeconds(10,0); // timer offset ten seconds into the future
OsTime      tenYears(10*365*24*60*60, 0);  // ten years into the future

class OsTimerTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsTimerTest);

    CPPUNIT_TEST(testImmediateTimer);
    CPPUNIT_TEST(testOneShotAfter);
    CPPUNIT_TEST(testTimerAccuracy) ; 
    CPPUNIT_TEST(testOneShotAt) ; 
    CPPUNIT_TEST(testStopTimerAfterOneShot);
    CPPUNIT_TEST(testPeriodicTimer) ; 
    CPPUNIT_TEST(testOneshotPeriodicComboTimer) ; 
    CPPUNIT_TEST(testStopPeriodicTimer) ; 
    CPPUNIT_TEST(testPeriodicTimer_FractionalTime);
    CPPUNIT_TEST(testDeleteTimerBeforeExpires);

    CPPUNIT_TEST(testStartFire);
    CPPUNIT_TEST(testStartStop);
    CPPUNIT_TEST(testStartPeriodicStop);
    CPPUNIT_TEST(testStartDelete);
    CPPUNIT_TEST(testStartStopStartFire);
    CPPUNIT_TEST(testStartStart);
    CPPUNIT_TEST(testStartStopStop);
    CPPUNIT_TEST(testStartFireStop);
    CPPUNIT_TEST(testStop);

    // Test to verify that GTimeVal and g_get_current_time work in the
    // current environment.
    CPPUNIT_TEST(sanityVerificationOfTimeFunctions);

    CPPUNIT_TEST(testDelayedStopMessage);

    CPPUNIT_TEST_SUITE_END();

private :
    static GTimeVal progStart;

    GTimeVal startTV;
    GTimeVal delayTV;
    GTimeVal endTV;

    int gCallBackCount;

public:

    void setUp()
    {
        gCallBackCount = 0;
        startTV.tv_sec = startTV.tv_usec = 0;
        endTV = startTV;
    }

    void tearDown()
    {
    }

    void setTVCalled()
    {
        g_get_current_time(&endTV);
       gCallBackCount++ ;
    }

    static void TVCallback(const int userData, const int eventData)
    {
        OsTimerTest* foo = (OsTimerTest*) userData;
        foo->setTVCalled();
    }

    long SecsToUsecs(long secs)
    {
        return (secs*OsTime::USECS_PER_SEC) ; 
    }

    long MsecsToUsecs(long mSecs)
    {
        return (mSecs*OsTime::USECS_PER_MSEC) ; 
    }

    long SecsToMsecs(long secs)
    {
        return (secs * OsTime::MSECS_PER_SEC) ; 
    }

    long getTimeDeltaInUsecs()
    {
        return (SecsToUsecs(endTV.tv_sec - startTV.tv_sec) + 
               (endTV.tv_usec - startTV.tv_usec)) ;
    }

    void DynaTest()
    {
    }

    void testImmediateTimer()
    {
        OsCallback* pNotifier ;
        OsTimer* pTimer ;
        OsStatus returnValue;
        long diffUSecs ; 
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        gCallBackCount = 0;
        g_get_current_time(&startTV);
        returnValue = pTimer->oneshotAfter(OsTime::NO_WAIT_TIME);
        // Although the message is supposed to be immediate, give a
        // little extra time
        OsTask::delay(OSTIMETOLERANCE);
        CPPUNIT_ASSERT_MESSAGE("Handle timer 1 (immediate) - ReturnValue",
                               returnValue == OS_SUCCESS);
        CPPUNIT_ASSERT_MESSAGE("Handle timer 1 (immediate) - Timer was fired",
                               gCallBackCount == 1);
        CPPUNIT_ASSERT_MESSAGE("Handle timer 1 (immediate) - mWasFired = TRUE",
                               pTimer->getWasFired() == TRUE);
        diffUSecs = getTimeDeltaInUsecs() ; 
        REPORT_SKEW(("      Timing inaccuracy = %6ld us;\n", diffUSecs));
        CPPUNIT_ASSERT_MESSAGE("Handle timer 1 (immediate) - Verify timer was "
                               "fired immediately",
                               diffUSecs > 0 && 
                               diffUSecs <= MsecsToUsecs(OSTIMETOLERANCE));
        delete pTimer;
        delete pNotifier;
    }

    void testOneShotAfter()
    {
        struct TestOneShotStruct 
        {
            const char* testDescription ;
            long seconds ; 
            long milliseconds ; 
            int tolerance ; 
        } ;

        OsCallback* pNotifier;
        string Message ; 
        int testCount ;
        
        TestOneShotStruct testData[] = {
            { "Test one shot after when time is specified as 0", 0, 0, OSTIMETOLERANCE },
            { "Test one shot after when time is equal to one second", 1, 0,
              OSTIMETOLERANCE },
            /* The next case was added to check if the inaccuracy applies ONLY to decimal
               values or even to integers
            */
            { "Test one shot after when time is greater than one second", 2, 285,
              OSTIMETOLERANCE },
            { "Test one shot after when time is an integer > 1", 3, 0, OSTIMETOLERANCE },
            { "Test one shot after when time is greater than 0 but less than 1", 0, 252,
              OSTIMETOLERANCE }
        } ;
 
        testCount = sizeof(testData)/ sizeof(testData[0]) ; 

        for (int i = 0 ; i < testCount ; i++)
        {
            long expectedWaitUSecs;
            long diffUSecs;
            OsTimer* pTimer;
            UtlBoolean returnValue;

            OsTime timeToWait(testData[i].seconds,
                testData[i].milliseconds*OsTime::USECS_PER_MSEC) ; 

            pNotifier = new OsCallback((int)this, TVCallback);
            pTimer = new OsTimer(*pNotifier);

            expectedWaitUSecs= SecsToUsecs(testData[i].seconds) + 
                MsecsToUsecs(testData[i].milliseconds) ; 

            // Give a small delay so we synchronize with the timer. 
            OsTask::delay(20);
            g_get_current_time(&startTV);
            gCallBackCount = 0;
            returnValue = pTimer->oneshotAfter(timeToWait);
            
            OsTask::delay(expectedWaitUSecs / OsTime::USECS_PER_MSEC +
                testData[i].tolerance) ; 

            TestUtilities::createMessage(2, &Message, testData[i].testDescription, 
                " - verify return value") ; 
            // gCallBackCount is reinitialized to 0 each iteration, so
            // its value should be 1 now.
            CPPUNIT_ASSERT_MESSAGE("Verify timer was fired for each iteration",
                                   gCallBackCount == 1);
            CPPUNIT_ASSERT_MESSAGE("mWasFired = TRUE in the current iteration",
                                   pTimer->getWasFired() == TRUE);
            CPPUNIT_ASSERT_MESSAGE(Message.data(), returnValue);
            TestUtilities::createMessage(2, &Message, testData[i].testDescription, 
                " - verify timer *was* fired") ; 
            diffUSecs = getTimeDeltaInUsecs();
            REPORT_SKEW(("      Timing inaccuracy for iter %3d = %8ld us; Time = %ld.%03ld;\n",
                        i, 
                        diffUSecs - expectedWaitUSecs, 
                        testData[i].seconds,
                        testData[i].milliseconds
                        )); 
            CPPUNIT_ASSERT_MESSAGE(Message.data(),
                                   diffUSecs >= expectedWaitUSecs - MsecsToUsecs(testData[i].tolerance) &&
                                   diffUSecs <= expectedWaitUSecs + MsecsToUsecs(testData[i].tolerance));
            delete pTimer;
            delete pNotifier;
        }
    }

    void testTimerAccuracy()
    {
        OsCallback* pNotifier;
        OsTimer* pTimer;
        long expectedWaitUSecs;
        long diffUSecs;

        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);

        expectedWaitUSecs=(1*OsTime::USECS_PER_SEC) + (250*OsTime::USECS_PER_MSEC) ;  
        OsTime timeToWait(1, 250*OsTime::USECS_PER_MSEC) ; 

        // Give a small delay so we synchronize with the timer. 
        OsTask::delay(20);
        g_get_current_time(&startTV);
        pTimer->oneshotAfter(timeToWait);
        //Sleep for a slightly additional time
        OsTask::delay(expectedWaitUSecs / OsTime::USECS_PER_MSEC +
                           OSTIMETOLERANCE) ; 

        CPPUNIT_ASSERT_MESSAGE("Timer was fired",
                               gCallBackCount == 1);
        diffUSecs = getTimeDeltaInUsecs();
        REPORT_SKEW(("      Timing inaccuracy = %8ld us; Time = %d.%03d;\n",
                    diffUSecs - expectedWaitUSecs, 
                    1, 250
                    )); 

        CPPUNIT_ASSERT_MESSAGE("Verify that the timer is fired accurately",
                diffUSecs >= expectedWaitUSecs - MsecsToUsecs(OSTIMETOLERANCE) &&
                diffUSecs <= expectedWaitUSecs + MsecsToUsecs(OSTIMETOLERANCE));
        delete pTimer;
        delete pNotifier;
    }

    void testOneShotAt()
    {
        OsCallback* pNotifier ;
        OsTimer* pTimer ;
        UtlBoolean returnValue ;  
        long diffUSecs ; 
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        // Create an OsDateTime object for 2 seconds in the future
        // and call oneShotAt.
        // Get the current time in seconds.
        time_t now = time(NULL) ;
        now += 2 ; 
        // Split it into the components needed to construct an OsDateTime.
        tm* gmtPlus2 = gmtime(&now) ; 
        // Get the current time to microsecond resolution.
        OsDateTime current;
        OsDateTime::getCurTime(current);
        // Put the components of gmtime into an OsDateTime, and add the
        // microseconds from "current".
        OsDateTime odt( 
          // gmtime returns (year - 1900), but OsDateTime requires 4-digit year.
          (unsigned short) gmtPlus2->tm_year + 1900,
                 (unsigned char)gmtPlus2->tm_mon, 
                 (unsigned char)gmtPlus2->tm_mday, 
                 (unsigned char)gmtPlus2->tm_hour, 
                 (unsigned char)gmtPlus2->tm_min, 
                 (unsigned char)gmtPlus2->tm_sec, 
          (unsigned int) current.getMicrosecond()
                 );
        g_get_current_time(&startTV);
        gCallBackCount = 0;
        returnValue = pTimer->oneshotAt(odt);
        //Although the message is supposed to be immediate, give a little extra time
        OsTask::delay(2000 + OSTIMETOLERANCE);
        CPPUNIT_ASSERT_MESSAGE("Handle timer 1 - returnValue", returnValue);
        CPPUNIT_ASSERT_MESSAGE("Timer was fired",
                              gCallBackCount == 1);
        CPPUNIT_ASSERT_MESSAGE("mWasFired = TRUE",
                               pTimer->getWasFired() == TRUE);
        diffUSecs = getTimeDeltaInUsecs();
        REPORT_SKEW(("      Timing inaccuracy = %6ld us;\n",
                    diffUSecs - MsecsToUsecs(2000)));
        CPPUNIT_ASSERT_MESSAGE("Handle timer 1 - Verify timer was fired "  
                               "after 2 secs",
                               diffUSecs > MsecsToUsecs(2000 - OSTIMETOLERANCE) && 
                               diffUSecs < MsecsToUsecs(2000 + OSTIMETOLERANCE));
        delete pTimer;
        delete pNotifier;
    }

    void testStopTimerAfterOneShot()
    {
        OsCallback* pNotifier; 
        OsTimer* pTimer ;
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        gCallBackCount = 0;
        pTimer->oneshotAfter(oneSecond) ; 
        OsTask::delay(500) ; 
        pTimer->stop() ;  
        OsTask::delay(1200) ;
        // We have waited for 1.7 sec after arming the timer, and the timer
        // was scheduled to fire after 1.0 sec.  But we disarmed the timer
        // after 0.5 sec, so gCallBackCount should be 0.
        CPPUNIT_ASSERT_MESSAGE("Verify that canceling the timer disarms it",
                               gCallBackCount == 0);
        CPPUNIT_ASSERT_MESSAGE("mWasFired = FALSE",
                               pTimer->getWasFired() == FALSE);
        delete pTimer ; 
        delete pNotifier;
    }

    void testPeriodicTimer()
    {
        OsCallback* pNotifier ;
        OsTimer* pTimer ; 
        UtlBoolean returnValue ; 
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        gCallBackCount = 0;
        returnValue = pTimer->periodicEvery(twoSeconds, twoSeconds) ; 
        //Give a delay of 10+ seconds . If all went well the call back method
        //must have been called once every 2 seconds and hence the callback count 
        //must be up by 5. 
        OsTask::delay(11250) ; 
        CPPUNIT_ASSERT_MESSAGE("Test periodic timer - verify return value", 
            returnValue) ; 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test periodic timer - verify that the "
                                     "timer is called periodically",
                                     5, gCallBackCount);
        //we didn't stop the timer, and it fired, so flag should be TRUE
        CPPUNIT_ASSERT_MESSAGE("mWasFired = TRUE",
                               pTimer->getWasFired() == TRUE);
        delete pTimer ; 
        delete pNotifier;
    }

    void testPeriodicTimer_FractionalTime()
    {
        OsCallback* pNotifier ;
        OsTimer* pTimer ; 
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        gCallBackCount = 0;
        pTimer->periodicEvery(OsTime::NO_WAIT_TIME, hundredMsec);
        //Give a delay of 1+ seconds . If all went well the call back method
        //must have been called once in the beginning and every 100 milliseconds thereafter
        //and hence the callback count must be up by 10+1. 
        OsTask::delay(1010) ; 
        KNOWN_BUG("Intermittent failure here; not predictable", "XPL-52");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test periodic timer - verify that the fractional timer is " 
                                    "*indeed* called periodically", 11, gCallBackCount);
        CPPUNIT_ASSERT_MESSAGE("mWasFired = TRUE",
                               pTimer->getWasFired() == TRUE);
        delete pTimer ; 
        delete pNotifier;
    }

    void testOneshotPeriodicComboTimer()
    {
        UtlBoolean returnValue ; 
        OsCallback* pNotifier ;
        OsTimer* pTimer ; 
        long diffUSecs ; 
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        gCallBackCount = 0;
        g_get_current_time(&startTV);
        returnValue = pTimer->periodicEvery(oneSecond, twoSeconds) ; 
        OsTask::delay(1000 + OSTIMETOLERANCE) ; 
        CPPUNIT_ASSERT_MESSAGE("Test oneshot & periodic timer combo - "
            "verify return value", returnValue) ; 
        CPPUNIT_ASSERT_MESSAGE("Timer was fired",
                               gCallBackCount == 1);
        diffUSecs = getTimeDeltaInUsecs();
        REPORT_SKEW(("      Timing inaccuracy = %ld us;\n",
                    diffUSecs - MsecsToUsecs(1000)));
        CPPUNIT_ASSERT_MESSAGE("Test oneshot/periodic combo - "
            "Verify first call is based on first argument",
            diffUSecs > MsecsToUsecs(1000 - OSTIMETOLERANCE) && 
            diffUSecs < MsecsToUsecs(1000 + OSTIMETOLERANCE) ) ;
        // now wait for another 5+ seconds. The total time after starting the timer is 
        // 6 seconds. 
        OsTask::delay(5340) ; 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test oneshot/periodic combo - Verify the timer is called " 
                                    "repeatadly as per the second argument", 3, gCallBackCount);
        delete pTimer ; 
        delete pNotifier;
    }

    void testStopPeriodicTimer()
    {
        OsCallback* pNotifier ;
        OsTimer* pTimer ; 
        OsTimer* pTimer2 ; 
        long diffUSecs ; 
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        gCallBackCount = 0;
        pTimer->periodicEvery(oneSecond, twoSeconds) ; 
        // Test the case where the timer is stopped even before the first leg
        // is fired
        OsTask::delay(350) ; 
        pTimer->stop() ; 
        // Wait for another 5 seconds. Neither the first shot nor the repeat legs
        // should ever have been called. 
        OsTask::delay(5000) ; 
        CPPUNIT_ASSERT_MESSAGE("Verify that a periodic timer can be stopped even " 
                              "before the first leg is called", gCallBackCount == 0);
        CPPUNIT_ASSERT_MESSAGE("mWasFired = FALSE",
                               pTimer->getWasFired() == FALSE);
        delete pTimer ; 

        pTimer2 = new OsTimer(*pNotifier);
        gCallBackCount = 0;
        g_get_current_time(&startTV);
        pTimer2->periodicEvery(oneSecond, twoSeconds) ; 
        OsTask::delay(1000 + OSTIMETOLERANCE) ;
        pTimer2->stop() ; 
        // Wait for another 5 seconds. Only the first shot should have been called.
        OsTask::delay(5000) ; 
        CPPUNIT_ASSERT_MESSAGE("Timer was fired",
                              gCallBackCount == 1);
        diffUSecs = getTimeDeltaInUsecs() ; 
        REPORT_SKEW(("      Timing inaccuracy = %ld us;\n",
                    diffUSecs - MsecsToUsecs(1000)));
        CPPUNIT_ASSERT_MESSAGE("Test stopping periodic timer - Verify that the " 
            "first leg was fired", 
            diffUSecs > MsecsToUsecs(1000 - OSTIMETOLERANCE) && 
            diffUSecs < MsecsToUsecs(1000 + OSTIMETOLERANCE) ) ; 
        // Also verify that only the first leg was called. 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test stopping periodic timer - Verify that ONLY the first " 
                                    "leg was fired", 1, gCallBackCount);
        // timer was stopped manually, so FALSE
        CPPUNIT_ASSERT_MESSAGE("mWasFired = FALSE",
                               pTimer2->getWasFired() == FALSE);
        delete pTimer2 ; 
        delete pNotifier;
    }

    void testDeleteTimerBeforeExpires()
    {
        OsCallback* pNotifier ;
        OsTimer* pTimer ; 
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        gCallBackCount = 0;
        pTimer->periodicEvery(oneSecond, twoSeconds) ; 
        OsTask::delay(350) ; 
        delete pTimer ; 
        // Wait for another 5 seconds. Neither the first shot nor the repeat legs
        // should ever have been called. 
        OsTask::delay(5000) ; 
        CPPUNIT_ASSERT_MESSAGE("Verify that a periodic timer can be stopped even " 
                              "before the first leg is called", gCallBackCount == 0);
        delete pNotifier ; 
    }

    // Tests for various sequences of operations.

    void testStartFire()
    {
       OsStatus returnValue;
       OsCallback notifier((int) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue =  timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter", returnValue == OS_SUCCESS);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/fire", gCallBackCount == 1);
       CPPUNIT_ASSERT_MESSAGE("mWasFired = TRUE",
                              timer.getWasFired() == TRUE);
    }

    void testStartStop()
    {
       OsStatus returnValue;
       OsCallback notifier((int) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop", returnValue == OS_SUCCESS);
       CPPUNIT_ASSERT_MESSAGE("mWasFired = FALSE",
                              timer.getWasFired() == FALSE);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/stop", gCallBackCount == 0);
    }

    void testStartPeriodicStop()
    {
       OsStatus returnValue;
       OsCallback notifier((int) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.periodicEvery(oneSecond, twoSeconds);
       CPPUNIT_ASSERT_MESSAGE("periodicEvery", returnValue == OS_SUCCESS);
       // Allow to fire 3 times
       OsTask::delay(6000);
       CPPUNIT_ASSERT_MESSAGE("mWasFired = TRUE",
                              timer.getWasFired() == TRUE);
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop", returnValue == OS_SUCCESS);
       CPPUNIT_ASSERT_MESSAGE("mWasFired = FALSE",
                              timer.getWasFired() == FALSE);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start-periodic/fire/stop",
                              gCallBackCount == 3);
    }

    void testStartDelete()
    {
       OsStatus returnValue;
       OsCallback notifier((int) this, TVCallback);
       OsTimer* pTimer = new OsTimer(notifier);
       gCallBackCount = 0;
       returnValue = pTimer->oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       CPPUNIT_ASSERT_MESSAGE("mWasFired = FALSE",
                              pTimer->getWasFired() == FALSE);
       // Delete the timer before it can fire.
       delete pTimer;
       // Make sure it did not fire.
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/delete", gCallBackCount == 0);
    }

    void testStartStopStartFire()
    {
       OsStatus returnValue;
       OsCallback notifier((int) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter 1", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("mWasFired = FALSE",
                              timer.getWasFired() == FALSE);
       CPPUNIT_ASSERT_MESSAGE("stop", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter 2", returnValue == OS_SUCCESS);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/fire", gCallBackCount == 1);
    }

    void testStartStart()
    {
       OsStatus returnValue;
       OsCallback notifier((int) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter 1", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter 2", returnValue == OS_FAILED);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/fire", gCallBackCount == 1);
    }

    void testStartStopStop()
    {
       OsStatus returnValue;
       OsCallback notifier((int) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop 1", returnValue == OS_SUCCESS);
       OsTask::delay(500);
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop 2", returnValue == OS_FAILED);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/fire", gCallBackCount == 0);
    }

    void testStartFireStop()
    {
       OsStatus returnValue;
       OsCallback notifier((int) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.oneshotAfter(oneSecond);
       CPPUNIT_ASSERT_MESSAGE("oneshotAfter", returnValue == OS_SUCCESS);
       OsTask::delay(1500);
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop", returnValue == OS_FAILED);
       OsTask::delay(5000);
       CPPUNIT_ASSERT_MESSAGE("Test start/fire", gCallBackCount == 1);
    }

    void testStop()
    {
       OsStatus returnValue;
       OsCallback notifier((int) this, TVCallback);
       OsTimer timer(notifier);
       gCallBackCount = 0;
       returnValue = timer.stop();
       CPPUNIT_ASSERT_MESSAGE("stop", returnValue == OS_FAILED);
       CPPUNIT_ASSERT_MESSAGE("Test start/fire", gCallBackCount == 0);
    }

    // Test to verify that GTimeVal and g_get_current_time work in the
    // current environment.
    void sanityVerificationOfTimeFunctions()
    {
#define N_TIME_DELTAS 10
        int i;
        struct TimeDeltas
        {
            int iterCount ; 
            GTimeVal start ; 
        } ; 
        TimeDeltas td[N_TIME_DELTAS] ; 
        GTimeVal tStart, tNow;
        int calls;

        for (i=0; i<N_TIME_DELTAS; i++) {
            calls = 2;
            g_get_current_time(&tStart) ; 
            g_get_current_time(&tNow) ; 
            while ((tNow.tv_usec - tStart.tv_usec) < 1000 && 
                    (tNow.tv_usec - tStart.tv_usec) >= 0) {
                g_get_current_time(&tNow) ; 
                calls++;
            }
            td[i].iterCount = calls;
            td[i].start = tStart;
            tStart = tNow;
        }

        for (i=0; i<N_TIME_DELTAS; i++) {
            REPORT_SKEW(("      %4d: t=%ld.%06ld, %5d calls\n", i,
                td[i].start.tv_sec,
                td[i].start.tv_usec,
                td[i].iterCount));
        }
        
    }

    void testDelayedStopMessage()
    {
       // Test a race condition where a periodic timer fires after
       // the application has stopped it, but before OsTimerTask has
       // processed the update message for the stop.
       // Previously, OsTimerTask would leave the task state as
       // "started" but not put the timer back in the timer queue, so
       // when the update message caused OsTimerTask::removeTimer to run, it
       // couldn't find the timer.
       OsCallback notifier((int) this, TVCallback);
       OsTimer timer(notifier);

       // Start timer to fire in 1 sec, and every 1 sec after that.
       timer.periodicEvery(OsTime(1, 0), OsTime(1, 0));
       // Delay slightly to ensure the timer start message is processed,
       // so timer.mOutstandingMessages == 0.
       OsTask::delay(100);
       CPPUNIT_ASSERT(timer.mOutstandingMessages == 0);

       // This is timer.OsTimer::stop(TRUE), with a delay put in the middle.
       {
          UtlBoolean synchronous = TRUE;
          OsStatus result;
          {
             UtlBoolean sendMessage = FALSE;
             
             // Update members.
             {
                OsLock lock(timer.mBSem);

                // Determine whether the call is successful.
                if (OsTimer::isStarted(timer.mApplicationState))
                {
                   // Update state to stopped.
                   timer.mApplicationState++;
                   result = OS_SUCCESS;
                   if (timer.mOutstandingMessages == 0)
                   {
                      // We will send a message.
                      sendMessage = TRUE;
                      timer.mOutstandingMessages++;
                   }
                }
                else
                {
                   result = OS_FAILED;
                }
             }

             // Delay 2 seconds, which is long enough for the timer to fire.
             OsTask::delay(2000);

             // If we need to, send an UPDATE message to the timer task.
             if (sendMessage)
             {
                if (synchronous) {
                   // Send message and wait.
                   OsEvent event;
                   OsTimerMsg msg(OsTimerMsg::OS_TIMER_UPDATE_SYNC, &timer, &event);
                   OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
                   assert(res == OS_SUCCESS);
                   event.wait();
                }
                else
                {
                   // Send message.
                   OsTimerMsg msg(OsTimerMsg::OS_TIMER_UPDATE, &timer, NULL);
                   OsStatus res = OsTimerTask::getTimerTask()->postMessage(msg);
                   assert(res == OS_SUCCESS);
                }
             }
          }
          CPPUNIT_ASSERT(result == OS_SUCCESS);
       }

       // We do no explicit test for success of this test, as the success
       // condition is that it does not trigger "assert(current)" in
       // OsTimerTask::removeTimer.
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsTimerTest);
