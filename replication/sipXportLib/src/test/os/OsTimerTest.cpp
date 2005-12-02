//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsCallback.h>
#include <os/OsTimer.h>
#include <os/OsTimerTask.h>
#include <os/OsTime.h>
#include <sipxunit/TestUtilities.h>

#include <time.h>
#include <string.h>

/*
 * OSTIMETOLERANCE is the allowed 'slop'; timers may be off by this
 *   much before the test is considered a failure.  The current value
 *   is too high; see XPL-39
 */
#define OSTIMETOLERANCE 150

//#define REPORT_SKEW(x) printf x
#define REPORT_SKEW(x) /* x */
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
    CPPUNIT_TEST_SUITE_END();
private :
    static GTimeVal progStart;

    GTimeVal startTV;
    GTimeVal delayTV;
    GTimeVal endTV;

    UtlBoolean gTimerCalled;
    int callBackCount ; 

public:

    OsTimerTest()
    {
        if (123456789 == progStart.tv_usec)
        {
            g_get_current_time(&progStart) ; 
        }
    }

    void setUp()
    {
        callBackCount = 0 ; 
        startTV.tv_sec = startTV.tv_usec = 0;
        endTV = startTV;
        gTimerCalled = FALSE ; 
    }

    void tearDown()
    {
    }

    void setTVCalled()
    {
        g_get_current_time(&endTV);
        gTimerCalled = TRUE;
        callBackCount++  ; 
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
#ifdef _WIN32
        KNOWN_FATAL_BUG("Fails under Win32", "XPL-101");
#endif
        OsCallback* pNotifier ;
        OsTimer* pTimer ;
        UtlBoolean returnValue ; 
        long diffUSecs ; 
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        gTimerCalled = FALSE ; 
        g_get_current_time(&startTV);
        returnValue = pTimer->oneshotAfter(OsTime::NO_WAIT);
        //Although the message is supposed to be immediate, give a little extra time
        OsTimerTask::delay(OSTIMETOLERANCE) ; 
        diffUSecs = getTimeDeltaInUsecs() ; 
        CPPUNIT_ASSERT_MESSAGE("Handle timer 1 (immediate) - ReturnValue", returnValue);
        CPPUNIT_ASSERT_MESSAGE("Handle timer 1 (immediate) - Verify timer was fired "  
                               "immediately", 
                               diffUSecs > 0 && 
                               diffUSecs <= MsecsToUsecs(OSTIMETOLERANCE));
        REPORT_SKEW(("      Timing inaccuracy = %6ld uS;\n", diffUSecs)); 
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
              OSTIMETOLERANCE },
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
            OsTimerTask::delay(20) ; 
            g_get_current_time(&startTV);
            gTimerCalled = FALSE;
            returnValue = pTimer->oneshotAfter(timeToWait);
            
            OsTimerTask::delay((expectedWaitUSecs/OsTime::USECS_PER_MSEC) + 
                testData[i].tolerance) ; 

            diffUSecs = getTimeDeltaInUsecs() ;

            TestUtilities::createMessage(2, &Message, testData[i].testDescription, 
                " - verify return value") ; 
            CPPUNIT_ASSERT_MESSAGE(Message.data(), returnValue);
            TestUtilities::createMessage(2, &Message, testData[i].testDescription, 
                " - verify timer *was* fired") ; 
            CPPUNIT_ASSERT_MESSAGE(Message.data(),
                diffUSecs >= expectedWaitUSecs - MsecsToUsecs(testData[i].tolerance) &&
                diffUSecs <= expectedWaitUSecs + MsecsToUsecs(testData[i].tolerance));
            REPORT_SKEW(("      Timing inaccuracy for iter %3d = %8ld uS; Time=%ld.%03ld;\n",
                i, 
                diffUSecs - expectedWaitUSecs, 
                testData[i].seconds,
                testData[i].milliseconds
                
                )); 
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
        OsTimerTask::delay(20) ; 
        g_get_current_time(&startTV);
        pTimer->oneshotAfter(timeToWait);
        //Sleep for a slightly additional time
        OsTimerTask::delay((expectedWaitUSecs/OsTime::USECS_PER_MSEC) +
                           OSTIMETOLERANCE) ; 

        diffUSecs = getTimeDeltaInUsecs();

        REPORT_SKEW(("      Timing inaccuracy = %8ld uS; Time=%d.%03d;\n",
                diffUSecs - expectedWaitUSecs, 
                1, 250
                )); 

        KNOWN_BUG("oneshotAfter not very precise for fractional times!", "XPL-39") ; 

        CPPUNIT_ASSERT_MESSAGE("Verify that the timer is fired accurately",
                diffUSecs >= expectedWaitUSecs - MsecsToUsecs(OSTIMETOLERANCE) &&
                diffUSecs <= expectedWaitUSecs + MsecsToUsecs(OSTIMETOLERANCE));
        delete pTimer;
        delete pNotifier;
    }

    void testOneShotAt()
    {
        KNOWN_FATAL_BUG("Create tests for methods testOneShotAt and testPeriodicAt", "XPL-40");
        OsCallback* pNotifier ;
        OsTimer* pTimer ;
        UtlBoolean returnValue ;  
        long diffUSecs ; 
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        // create an OsDateTime object such that the time is 2 seconds in the future
        // and call oneShotAt
        time_t now = time(NULL) ;
        now += 2 ; 
        tm* gmtPlus2 = gmtime(&now) ; 
        OsDateTime odt( 
                 (unsigned short)gmtPlus2->tm_year, 
                 (unsigned char)gmtPlus2->tm_mon, 
                 (unsigned char)gmtPlus2->tm_mday, 
                 (unsigned char)gmtPlus2->tm_hour, 
                 (unsigned char)gmtPlus2->tm_min, 
                 (unsigned char)gmtPlus2->tm_sec, 
                 (int)0 
                 );
        g_get_current_time(&startTV);
        gTimerCalled = FALSE ; 
        returnValue = pTimer->oneshotAt(odt);
        //Although the message is supposed to be immediate, give a little extra time
        OsTimerTask::delay(2000 + OSTIMETOLERANCE) ; 
        diffUSecs = getTimeDeltaInUsecs() ; 
        CPPUNIT_ASSERT_MESSAGE("Handle timer 1 - returnValue", returnValue);
        CPPUNIT_ASSERT_MESSAGE("Handle timer 1 - Verify timer was fired "  
                               "after 2 secs",
                               diffUSecs > MsecsToUsecs(2000 - OSTIMETOLERANCE) && 
                               diffUSecs < MsecsToUsecs(2000 + OSTIMETOLERANCE));
        printf("      Timing inaccuracy = %6ld uS;\n",
               diffUSecs - MsecsToUsecs(2000)) ;
        delete pTimer;
        delete pNotifier;
    }


    void testStopTimerAfterOneShot()
    {
        OsCallback* pNotifier; 
        OsTimer* pTimer ;
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        gTimerCalled = FALSE;
        pTimer->oneshotAfter(oneSecond) ; 
        OsTask::delay(500) ; 
        pTimer->stop() ;  
        OsTask::delay(1200) ;
        // If the timer was indeed disarmed even though we have waited for more than 
        // the initial arm time (1000 msecs - in fact we are waiting for 1+ seconds after
        // stoping which makes it a total of 1.7 seconds delay), the callback method
        // should never have been called which means that the gTimerCalled should never 
        // have been set to TRUE
        CPPUNIT_ASSERT_MESSAGE("Verify that canceling the timer disarms it", !gTimerCalled) ;  
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
        gTimerCalled = FALSE ; 
        callBackCount = 0 ;
        returnValue = pTimer->periodicEvery(twoSeconds, twoSeconds) ; 
        //Give a delay of 10+ seconds . If all went well the call back method
        //must have been called once every 2 seconds and hence the callbackcount 
        //must be up by 5. 
        OsTask::delay(11250) ; 
        CPPUNIT_ASSERT_MESSAGE("Test periodic timer - verify return value", 
            returnValue) ; 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test periodic timer - verify that the timer is " 
           "*indeed* called periodically", 5, callBackCount) ; 
        delete pTimer ; 
        delete pNotifier;
    }

    void testPeriodicTimer_FractionalTime()
    {
        OsCallback* pNotifier ;
        OsTimer* pTimer ; 
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        callBackCount = 0 ;
        pTimer->periodicEvery(OsTime::NO_WAIT, hundredMsec) ; 
        //Give a delay of 1+ seconds . If all went well the call back method
        //must have been called once in the begining and every 100 milliseconds thereafter
        //and hence the callbackcount must be up by 10+1. 
        OsTask::delay(1010) ; 
        KNOWN_BUG("Itermittent failure here; not predictable", "XPL-52");
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test periodic timer - verify that the fractional timer is " 
           "*indeed* called periodically", 11, callBackCount) ; 
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
        callBackCount = 0 ;
        g_get_current_time(&startTV);
        returnValue = pTimer->periodicEvery(oneSecond, twoSeconds) ; 
        OsTask::delay(1000 + OSTIMETOLERANCE) ; 
        diffUSecs = getTimeDeltaInUsecs() ; 
        CPPUNIT_ASSERT_MESSAGE("Test oneshot & periodic timer combo - "
            "verify return value", returnValue) ; 
        CPPUNIT_ASSERT_MESSAGE("Test oneshot/periodic combo - "
            "Verify first call is based on first argument",
            diffUSecs > MsecsToUsecs(1000 - OSTIMETOLERANCE) && 
            diffUSecs < MsecsToUsecs(1000 + OSTIMETOLERANCE) ) ;
        printf("      Timing inaccuracy = %lduS;\n",
               diffUSecs - MsecsToUsecs(1000)) ;
        // now wait for another 5+ seconds. The total time after starting the timer is 
        // 6 seconds. 
        OsTask::delay(5340) ; 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test oneshot/periodic combo - Verify the timer is called " 
            "repeatadly as per the second argument", 3, callBackCount) ;
 
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
        callBackCount = 0 ;
        gTimerCalled = FALSE ; 
        pTimer->periodicEvery(oneSecond, twoSeconds) ; 
        // Test the case where the timer is stopped even before the first leg
        // is fired
        OsTask::delay(350) ; 
        pTimer->stop() ; 
        // Wait for another 5 seconds. Neither the first shot nor the repeat legs
        // should ever have been called. 
        OsTask::delay(5000) ; 
        CPPUNIT_ASSERT_MESSAGE("Verify that a periodictimer can be stopped even " 
            "before the first leg is called", !gTimerCalled) ; 
        delete pTimer ; 

        pTimer2 = new OsTimer(*pNotifier);
        callBackCount = 0 ;
        g_get_current_time(&startTV);
        pTimer2->periodicEvery(oneSecond, twoSeconds) ; 
        OsTask::delay(1000 + OSTIMETOLERANCE) ;
        pTimer2->stop() ; 
        // Wait for another 5 seconds. Only the first shot should have been called
        OsTask::delay(5000) ; 
        diffUSecs = getTimeDeltaInUsecs() ; 
        
        CPPUNIT_ASSERT_MESSAGE("Test stoping periodic timer - Verify that the " 
            "first leg was fired", 
            diffUSecs > MsecsToUsecs(1000 - OSTIMETOLERANCE) && 
            diffUSecs < MsecsToUsecs(1000 + OSTIMETOLERANCE) ) ; 
        printf("      Timing inaccuracy = %lduS;\n",
               diffUSecs - MsecsToUsecs(1000)) ;
        // Also verify that only the first leg was called. 
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Test stoping periodic timer - Verify that ONLY the first " 
            "leg was fired", 1, callBackCount) ; 
        delete pTimer2 ; 
        delete pNotifier;

    }

    void testDeleteTimerBeforeExpires()
    {
        OsCallback* pNotifier ;
        OsTimer* pTimer ; 
        pNotifier = new OsCallback((int)this, TVCallback);
        pTimer = new OsTimer(*pNotifier);
        callBackCount = 0 ;
        gTimerCalled = FALSE ; 
        pTimer->periodicEvery(oneSecond, twoSeconds) ; 
        OsTask::delay(350) ; 
        delete pTimer ; 
        // Wait for another 5 seconds. Neither the first shot nor the repeat legs
        // should ever have been called. 
        OsTask::delay(5000) ; 
        CPPUNIT_ASSERT_MESSAGE("Verify that a periodictimer can be stopped even " 
            "before the first leg is called", !gTimerCalled) ; 
        delete pNotifier ; 
    }

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
            while ((tNow.tv_usec - tStart.tv_usec) < 1000) {
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
};

GTimeVal OsTimerTest::progStart= {0,123456789} ; 
CPPUNIT_TEST_SUITE_REGISTRATION(OsTimerTest);

