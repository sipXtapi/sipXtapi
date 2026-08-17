//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef EXCLUDE_RTCP

#include <string.h>

#include <os/OsIntTypes.h>
#include <os/OsTask.h>
#include <os/OsSysLog.h>
#include <os/OsMutex.h>
#include <os/OsLock.h>

#include <sipxunittests.h>
#include "rtcp/RTCManager.h"
#include "rtcp/INetDispatch.h"
#include "rtcp/IRTPDispatch.h"
#include "rtcp/ISetSenderStatistics.h"

// Test hook exported by RTCPConnection.cpp under DEBUGGING_RTCP_REPORTS.
extern "C" { extern int adjustRtcpPeriod(int x); }

// A well formed inbound compound packet: Sender Report followed by SDES.  The
// SDES chunk is taken from the real capture used by RtcpParserTest.
//
// Shared by the feeder threads and by the warm-up, so that the warm-up drives
// the same receive path the measured run does.  ProcessPacket() parses in
// place, so callers copy this rather than passing it directly.
static const unsigned char sRtcpCompoundPacket[] =
{
    // Sender Report: V=2, P=0, RC=0, PT=200, length=6 (28 bytes)
    0x80, 0xc8, 0x00, 0x06,
    0x1f, 0xcd, 0x53, 0xe2,   // SSRC
    0xc7, 0x00, 0x00, 0x01,   // NTP timestamp, most significant
    0x40, 0x00, 0x00, 0x00,   // NTP timestamp, least significant
    0x00, 0x00, 0x03, 0xe8,   // RTP timestamp
    0x00, 0x00, 0x00, 0x64,   // sender's packet count
    0x00, 0x00, 0x27, 0x10,   // sender's octet count

    // SDES: V=2, P=0, SC=1, PT=202, length=6 (28 bytes)
    0x81, 0xca, 0x00, 0x06,
    0x1f, 0xcd, 0x53, 0xe2,   // SSRC
    0x01, 0x11,               // CNAME, length 17
    0x4c, 0x69, 0x66, 0x65, 0x53, 0x69, 0x7a, 0x65, 0x20,
    0x45, 0x6e, 0x64, 0x70, 0x6f, 0x69, 0x6e, 0x74,
    0x00                      // end of SDES items
};

// Leak checking uses the MSVC debug CRT heap, so it is only available on a
// Windows debug build.  Everywhere else the checks compile away entirely.
#if defined(_MSC_VER) && defined(_DEBUG)
#  include <crtdbg.h>
#  define RTCP_STRESS_LEAK_CHECK 1
#endif

/**
 * Concurrency stress for RTCP session and connection lifetime.
 *
 * This drives the RTCP layer directly rather than through the media stack:
 * IRTCPControl::CreateSession(), IRTCPSession::CreateRTCPConnection() and
 * IRTCPControl::TerminateSession() are the whole surface, so no flowgraph,
 * media task or socket is required.  GenerateRTCPReports() still runs its
 * full path without a network render attached -- it builds the reports and
 * fires the notifications, and merely logs that the send could not happen --
 * which is exactly the path that must be raced against teardown.
 *
 * What it is meant to catch, all of which involves two threads meeting:
 *
 *   - CRTCPConnection::GenerateRTCPReports() on the CRTCManager message
 *     thread against Terminate()/StopRenderer() on this thread.  Teardown
 *     clears m_bInitialized and then releases and NULLs m_poRTCPRender, which
 *     is precisely what the report generator dereferences.
 *   - CRTCPSession::TerminateAllConnections() draining the connection list
 *     while the reporting alarm is delivering into the same session.
 *   - CRTCPSession::ResetAllConnections() snapshotting that list while
 *     connections are being torn down underneath it.
 *   - Reference ownership across CreateRTCPConnection() and teardown.
 *
 * The reporting cadence is cranked down from the production 5 seconds to a
 * few tens of milliseconds for the duration of the run.  At 5 seconds a
 * teardown essentially never lands on top of a live reporting alarm, so the
 * interesting window is only reachable by luck; at 60ms with several
 * connections per session the two collide continuously.
 *
 * Duration is deliberately modest so this can live in the normal suite.  For
 * a real soak, raise the iteration count -- the failure modes here are
 * probabilistic, and a long run is worth far more than a short one.
 */
class RtcpStressTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(RtcpStressTest);

    CPPUNIT_TEST(testConcurrentSessionChurn);

    CPPUNIT_TEST_SUITE_END();

public:

    // Tuning.
    //
    // DWELL_MS must exceed REPORT_PERIOD_MS.  A session that dies before its
    // first reporting alarm never gives GenerateRTCPReports() a chance to be
    // running when Terminate() arrives, which is the entire point of the
    // exercise -- with a dwell shorter than the period most iterations test
    // nothing but plain construction and destruction.
    //
    // Wall time is roughly ITERATIONS_PER_THREAD * DWELL_MS (the threads run
    // concurrently), so 5000 x 80ms is about seven minutes.  Drop iterations
    // to ~200 for a sub-20-second smoke run; raise it for a real soak, since
    // these failures are probabilistic.
    enum
    {
        CHURN_THREADS           = 4,
        ITERATIONS_PER_THREAD   = 200,
        CONNECTIONS_PER_SESSION = 4,
        // ONE feeder, deliberately.  In production every inbound RTCP packet
        // is delivered by the single NetInTask thread, which reads all sockets
        // and calls MprFromNet::pushPacket() serially, so ProcessPacket() is
        // never re-entered concurrently for a connection.
        //
        // Running two feeders does not model anything real, and it produces a
        // false positive: CRTCPSource lazily creates m_poSenderReport with an
        // unguarded "if (m_poSenderReport != NULL); else if ((m_poSenderReport
        // = new CSenderReport(...)))", so two threads both see NULL, both
        // allocate, and one assignment orphans the other.  The orphan also
        // holds the AddRef its constructor took on m_piSetReceiverStatistics
        // -- the render's CReceiverReport -- so each lost race leaks a
        // CSenderReport/CReceiverReport pair.  Measured at 317 pairs over 3200
        // connections with two feeders, and exactly zero with one.
        FEED_THREADS            = 1,

        REPORT_PERIOD_MS        = 60,   // must exceed adjustRtcpPeriod's 50ms floor
        DWELL_MS                = 80,   // must exceed REPORT_PERIOD_MS
        MAX_CONNECTIONS         = 16,

        FEED_BURST              = 4,    // packets per feeder before yielding
        PACKET_BYTES            = 56,   // Sender Report (28) + SDES (28)
        MAX_LIVE_DISPATCHERS    = CHURN_THREADS * MAX_CONNECTIONS,

        // Time allowed for the CRTCManager message queue to drain.  Queued
        // CMessage objects hold references to their connection and session,
        // so anything still in flight counts as live memory.
        QUEUE_DRAIN_MS          = 2000
    };

    /**
     * The set of INetDispatch interfaces belonging to connections that are
     * currently alive, so the feeder threads have something to deliver into.
     *
     * Reference handling mirrors MprFromNet::setDispatchers(): the network
     * side takes its OWN reference on the dispatcher, because
     * CRTCPConnection::GetDispatchInterfaces() hands out
     * (INetDispatch *)m_poRTCPSource WITHOUT one.  Holding that reference is
     * what stops CRTCPConnection::Terminate() -- which releases and NULLs
     * m_poRTCPSource -- from freeing the source while a packet is being
     * parsed on another thread.  A borrower takes a further reference for the
     * duration of its ProcessPacket() call, exactly as the real receive path
     * keeps the object alive across a dispatch.
     */
    class LiveDispatchRegistry
    {
    public:
        LiveDispatchRegistry()
            : mMutex(OsMutex::Q_PRIORITY)
            , mCount(0)
        {
        }

        void publish(INetDispatch *piDispatch)
        {
            if (piDispatch == NULL) return;

            OsLock lock(mMutex);
            if (mCount < MAX_LIVE_DISPATCHERS)
            {
                piDispatch->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
                mapDispatch[mCount++] = piDispatch;
            }
        }

        void withdraw(INetDispatch *piDispatch)
        {
            if (piDispatch == NULL) return;

            OsLock lock(mMutex);
            for (int i = 0; i < mCount; i++)
            {
                if (mapDispatch[i] == piDispatch)
                {
                    mapDispatch[i] = mapDispatch[--mCount];
                    piDispatch->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
                    return;
                }
            }
        }

        // Release anything still published, returning how many there were.
        // The count should be zero once the churn threads have run to
        // completion; anything else means a worker was cut short mid-iteration
        // and the harness itself is holding references.
        int releaseAll()
        {
            OsLock lock(mMutex);
            int iRemaining = mCount;
            while (mCount > 0)
            {
                mapDispatch[--mCount]->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
            }
            return iRemaining;
        }

        // Returns a dispatcher with a reference taken on the caller's behalf,
        // or NULL when nothing is live.  The caller must Release() it.
        INetDispatch *borrow(int iHint)
        {
            OsLock lock(mMutex);
            if (mCount == 0) return NULL;

            INetDispatch *piDispatch = mapDispatch[iHint % mCount];
            piDispatch->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));
            return piDispatch;
        }

    private:
        OsMutex       mMutex;
        INetDispatch *mapDispatch[MAX_LIVE_DISPATCHERS];
        int           mCount;
    };

    /**
     * Delivers inbound RTCP compound packets into whichever connections are
     * live, so that CRTCPSource::ProcessPacket() is running on one thread
     * while another thread tears the owning connection down.
     */
    class RtcpFeedTask : public OsTask
    {
    public:
        RtcpFeedTask(LiveDispatchRegistry *pRegistry)
            : OsTask("RtcpFeed-%d")
            , mpRegistry(pRegistry)
            , mPacketsFed(0)
            , mStop(FALSE)
            , mFinished(FALSE)
        {
        }

        virtual ~RtcpFeedTask()
        {
            waitUntilShutDown();
        }

        UtlBoolean waitForCompletion(int milliSecToWait)
        {
            return OsTask::waitUntilShutDown(milliSecToWait);
        }

        int run(void *pArg)
        {
            int iHint  = 0;
            int iBurst = 0;

            while (!mStop)
            {
                INetDispatch *piDispatch = mpRegistry->borrow(iHint++);
                if (piDispatch != NULL)
                {
                    // ProcessPacket() parses in place, so every feeder works
                    // from its own copy rather than a shared buffer.
                    unsigned char uchPacket[PACKET_BYTES];
                    memcpy(uchPacket, sRtcpCompoundPacket, PACKET_BYTES);

                    piDispatch->ProcessPacket(uchPacket, PACKET_BYTES);
                    piDispatch->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

                    mPacketsFed++;

                    // Pace the feed.
                    //
                    // Every inbound report becomes RTCP event messages posted
                    // to the single CRTCManager consumer thread, and on Win32
                    // that queue is a CTLinkedList with NO bound (VxWorks and
                    // POSIX both cap it at MAX_NOTIFY_MESSAGES_DEFINE).
                    // Feeding flat out simply outruns the consumer: the
                    // backlog grows without limit, and since each queued
                    // CMessage holds references to its connection and session
                    // it pins those objects alive as well, so the whole thing
                    // presents as a steadily climbing memory footprint.
                    //
                    // Real RTCP is low rate by design -- on the order of one
                    // compound packet per source every few seconds -- so
                    // pacing here is realism rather than politeness.  An
                    // unpaced feeder measures how fast this thread can outrun
                    // the queue, which is not what this test is for.
                    if (++iBurst >= FEED_BURST)
                    {
                        iBurst = 0;
                        OsTask::delay(1);
                    }
                }
                else
                {
                    OsTask::delay(1);
                }
            }

            mFinished = TRUE;
            return 0;
        }

        void       requestStop()      { mStop = TRUE; }
        UtlBoolean isFinished() const { return mFinished; }
        int        getPacketsFed() const { return mPacketsFed; }

    private:
        LiveDispatchRegistry *mpRegistry;
        int                   mPacketsFed;
        volatile UtlBoolean   mStop;
        volatile UtlBoolean   mFinished;
    };

    /**
     * Repeatedly builds a session with several connections, lets the
     * reporting alarm run against it, then tears it down.
     */
    class RtcpChurnTask : public OsTask
    {
    public:
        RtcpChurnTask(IRTCPControl *piRTCPControl,
                      LiveDispatchRegistry *pRegistry)
            : OsTask("RtcpChurn-%d")
            , mpiRTCPControl(piRTCPControl)
            , mpRegistry(pRegistry)
            , mSessionsCompleted(0)
            , mCreateFailures(0)
            , mFinished(FALSE)
        {
        }

        virtual ~RtcpChurnTask()
        {
            waitUntilShutDown();
        }

        // OsTaskBase::waitUntilShutDown() is protected, so it is reachable
        // from this derived class but not from the test body that needs to
        // join the workers.  Forward it, as UtlHashMapPerformance does.
        UtlBoolean waitForCompletion(int milliSecToWait)
        {
            return OsTask::waitUntilShutDown(milliSecToWait);
        }

        int run(void *pArg)
        {
            for (int i = 0; i < ITERATIONS_PER_THREAD && !isShuttingDown(); i++)
            {
                IRTCPSession *piSession = mpiRTCPControl->CreateSession();
                if (piSession == NULL)
                {
                    mCreateFailures++;
                    continue;
                }

                IRTCPConnection *apiConnections[MAX_CONNECTIONS];
                INetDispatch    *apiDispatchers[MAX_CONNECTIONS];
                int iConnections = 0;

                for (int c = 0; c < CONNECTIONS_PER_SESSION; c++)
                {
                    ssrc_t localSSRC = mpiRTCPControl->GenerateUniqueLocalSSRC();
                    IRTCPConnection *piConnection =
                                    piSession->CreateRTCPConnection(localSSRC);
                    if (piConnection != NULL)
                    {
                        // Publish the inbound dispatch interface so the feeder
                        // threads can deliver RTCP into this connection while
                        // it is alive -- and, critically, while it is being
                        // torn down below.
                        //
                        // GetDispatchInterfaces() does NOT reference the
                        // INetDispatch or IRTPDispatch it hands back, but it
                        // DOES reference the ISetSenderStatistics (by way of
                        // GetSenderStatInterface).  Release that one here, as
                        // ~MprToNet() does; leaving it held would prop up the
                        // Sender Report's count and mask exactly the kind of
                        // teardown defect this test exists to find.
                        INetDispatch         *piNetDispatch  = NULL;
                        IRTPDispatch         *piRTPDispatch  = NULL;
                        ISetSenderStatistics *piSenderStats  = NULL;

                        piConnection->GetDispatchInterfaces(&piNetDispatch,
                                                            &piRTPDispatch,
                                                            &piSenderStats);
                        if (piSenderStats != NULL)
                        {
                            piSenderStats->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
                        }

                        mpRegistry->publish(piNetDispatch);

                        apiDispatchers[iConnections] = piNetDispatch;
                        apiConnections[iConnections] = piConnection;
                        iConnections++;
                    }
                    else
                    {
                        mCreateFailures++;
                    }
                }

                // Stay alive long enough for the reporting alarm to fire
                // against these connections, so that the teardown below has a
                // live GenerateRTCPReports() to collide with.
                OsTask::delay(DWELL_MS);

                // Teardown, in the same order the media subsystem uses.
                // TerminateSession() drains the session's connection list and
                // releases the reference that list held on each connection;
                // the reference handed back by CreateRTCPConnection() belongs
                // to us and must be released separately -- exactly what
                // ~MpFlowGraphBase() does when it drains mRtcpConnMap.
                mpiRTCPControl->TerminateSession(piSession);

                // Withdraw the dispatchers only AFTER the session has been
                // terminated.  That is deliberate, and it mirrors the real
                // ordering: ~MpFlowGraphBase() terminates the RTCP session
                // before destroying the flowgraph resources that hold the
                // dispatch references, so a packet genuinely can be in
                // ProcessPacket() while Terminate() runs.  Withdrawing first
                // would quietly close the very window under test.
                for (int c = 0; c < iConnections; c++)
                {
                    mpRegistry->withdraw(apiDispatchers[c]);
                }

                for (int c = 0; c < iConnections; c++)
                {
                    apiConnections[c]->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
                }

                mSessionsCompleted++;
            }

            mFinished = TRUE;
            return 0;
        }

        // Set once run() has completed its iterations.  The test body waits on
        // this rather than on waitUntilShutDown(), which would REQUEST shutdown
        // and cut the run short (see the note in testConcurrentSessionChurn).
        UtlBoolean isFinished() const    { return mFinished; }

        int getSessionsCompleted() const { return mSessionsCompleted; }
        int getCreateFailures() const    { return mCreateFailures; }

    private:
        IRTCPControl         *mpiRTCPControl;
        LiveDispatchRegistry *mpRegistry;
        int                   mSessionsCompleted;
        int                   mCreateFailures;
        volatile UtlBoolean   mFinished;
    };

    // One complete create/terminate cycle, run before the leak baseline is
    // taken.  The first pass through this code allocates a good deal that is
    // never freed by design -- the CRTCManager singleton and its message
    // thread, the local SDES report, assorted lazily initialised statics --
    // and none of that is distinguishable from a leak in a heap diff.  Doing
    // one cycle up front moves those allocations before the baseline.
    void warmUp(IRTCPControl *piRTCPControl)
    {
        IRTCPSession *piSession = piRTCPControl->CreateSession();
        CPPUNIT_ASSERT(piSession != NULL);

        IRTCPConnection *piConnection = piSession->CreateRTCPConnection(
                                    piRTCPControl->GenerateUniqueLocalSSRC());
        CPPUNIT_ASSERT(piConnection != NULL);

        INetDispatch         *piNetDispatch = NULL;
        IRTPDispatch         *piRTPDispatch = NULL;
        ISetSenderStatistics *piSenderStats = NULL;

        piConnection->GetDispatchInterfaces(&piNetDispatch, &piRTPDispatch,
                                            &piSenderStats);
        if (piSenderStats != NULL)
        {
            piSenderStats->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        }

        // Drive one packet through the receive path as well.  The measured run
        // does this constantly, and the first pass through it allocates state
        // that is never freed -- the inbound CSenderReport, the SDES entry for
        // the remote SSRC, and the stdio buffers behind the logging it emits.
        // Left until after the baseline those look exactly like a leak, which
        // is what the residual couple of blocks turned out to be.
        if (piNetDispatch != NULL)
        {
            piNetDispatch->AddRef(ADD_RELEASE_CALL_ARGS(__LINE__));

            unsigned char uchPacket[PACKET_BYTES];
            memcpy(uchPacket, sRtcpCompoundPacket, PACKET_BYTES);
            piNetDispatch->ProcessPacket(uchPacket, PACKET_BYTES);

            piNetDispatch->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
        }

        piRTCPControl->TerminateSession(piSession);
        piConnection->Release(ADD_RELEASE_CALL_ARGS(__LINE__));

        // Let the events this generated work their way out of the queue.
        OsTask::delay(QUEUE_DRAIN_MS);
    }

    void testConcurrentSessionChurn()
    {
        // Acquire the control interface ONCE, on this thread, before any
        // worker starts.  CRTCManager::getRTCPControl() creates the singleton
        // with its serialising lock commented out, so concurrent first callers
        // can race to construct it.  That is a known, separate defect; taking
        // the reference up front keeps it out of these results.
        // NOTE: logging is set up by UnitTestLogHooks, which the test
        // environment calls around each test class -- output lands in
        // ./RtcpStressTest.log at PRI_DEBUG.  Do NOT initialise OsSysLog from
        // inside a test: initialize() without a matching shutdown() leaves the
        // logger alive into static destruction, and anything that logs from a
        // destructor then reaches OsDateTimeWnt's file-static critical section
        // after it has been deleted, which faults inside EnterCriticalSection.
        IRTCPControl *piRTCPControl = CRTCManager::getRTCPControl();
        CPPUNIT_ASSERT(piRTCPControl != NULL);

        // Collapse the reporting cadence so teardown and report generation
        // actually contend.  Restored before returning.
        int savedPeriod = adjustRtcpPeriod(REPORT_PERIOD_MS);

        // Get the one-time allocations out of the way, then take the baseline.
        warmUp(piRTCPControl);

#ifdef RTCP_STRESS_LEAK_CHECK
        // Route the CRT reports to stdout.  By default _CRT_WARN goes to the
        // debugger via OutputDebugString, so running from a console -- which
        // is how this is normally driven -- would discard the dump entirely.
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
        _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);

        _CrtMemState memBefore, memAfter, memDiff;
        _CrtMemCheckpoint(&memBefore);
#endif

        LiveDispatchRegistry registry;

        RtcpChurnTask *apTasks[CHURN_THREADS];

        // Sized so FEED_THREADS can be set to 0 to take the inbound packet
        // path out of the run entirely -- a zero length array is not legal.
        RtcpFeedTask  *apFeeders[FEED_THREADS > 0 ? FEED_THREADS : 1];
        int t;
        int f;

        for (t = 0; t < CHURN_THREADS; t++)
        {
            apTasks[t] = new RtcpChurnTask(piRTCPControl, &registry);
        }
        for (f = 0; f < FEED_THREADS; f++)
        {
            apFeeders[f] = new RtcpFeedTask(&registry);
        }

        // Feeders first, so inbound packets are already in flight by the time
        // the first session comes up.
        for (f = 0; f < FEED_THREADS; f++)
        {
            CPPUNIT_ASSERT(apFeeders[f]->start());
        }
        for (t = 0; t < CHURN_THREADS; t++)
        {
            CPPUNIT_ASSERT(apTasks[t]->start());
        }

        // Wait for the workers to finish their iterations.
        //
        // NOT via waitUntilShutDown(): that calls requestShutdown() before it
        // joins, and the run loop checks isShuttingDown(), so joining first
        // would truncate the run to however long the main thread took to get
        // here.  Poll for natural completion, then join.
        //
        // Exhausting this deadline is itself a result -- it means teardown
        // wedged, which under POSIX is the signature of a list lock held
        // across a callback that re-enters it (the non-recursive OsBSem case
        // EnterCriticalSection() now asserts on).
        const int POLL_MS     = 100;
        const int DEADLINE_MS = ITERATIONS_PER_THREAD * (DWELL_MS + 40) + 30000;

        int        iWaitedMs   = 0;
        UtlBoolean allFinished = FALSE;

        while (!allFinished && iWaitedMs < DEADLINE_MS)
        {
            allFinished = TRUE;
            for (t = 0; t < CHURN_THREADS; t++)
            {
                if (!apTasks[t]->isFinished())
                {
                    allFinished = FALSE;
                    break;
                }
            }

            if (!allFinished)
            {
                OsTask::delay(POLL_MS);
                iWaitedMs += POLL_MS;
            }
        }

        CPPUNIT_ASSERT_MESSAGE(
            "RTCP churn threads did not finish -- probable teardown deadlock",
            allFinished);

        // The churn is done, so no more dispatchers will be published.  Stop
        // the feeders and let them drain out of any in-flight ProcessPacket().
        int totalPacketsFed = 0;

        for (f = 0; f < FEED_THREADS; f++)
        {
            apFeeders[f]->requestStop();
        }

        int iFeederWaitMs = 0;
        UtlBoolean feedersFinished = FALSE;
        while (!feedersFinished && iFeederWaitMs < 20000)
        {
            feedersFinished = TRUE;
            for (f = 0; f < FEED_THREADS; f++)
            {
                if (!apFeeders[f]->isFinished())
                {
                    feedersFinished = FALSE;
                    break;
                }
            }
            if (!feedersFinished)
            {
                OsTask::delay(POLL_MS);
                iFeederWaitMs += POLL_MS;
            }
        }

        CPPUNIT_ASSERT_MESSAGE(
            "RTCP feeder threads did not finish -- probable stall in ProcessPacket",
            feedersFinished);

        int totalSessions = 0;
        int totalFailures = 0;

        for (t = 0; t < CHURN_THREADS; t++)
        {
            // Safe to request shutdown now; the run loop has already exited.
            apTasks[t]->waitForCompletion(20000);

            totalSessions += apTasks[t]->getSessionsCompleted();
            totalFailures += apTasks[t]->getCreateFailures();
        }

        for (f = 0; f < FEED_THREADS; f++)
        {
            apFeeders[f]->waitForCompletion(20000);
            totalPacketsFed += apFeeders[f]->getPacketsFed();
        }

        for (t = 0; t < CHURN_THREADS; t++)
        {
            delete apTasks[t];
        }
        for (f = 0; f < FEED_THREADS; f++)
        {
            delete apFeeders[f];
        }

        // Nothing should still be published once every worker has run to
        // completion.  If something is, the harness is holding references and
        // any leak numbers below would be its fault rather than the code's.
        int iStillPublished = registry.releaseAll();
        CPPUNIT_ASSERT_EQUAL(0, iStillPublished);

        // Let the queue drain before snapshotting: a CMessage still in flight
        // holds references to its connection and session, and would otherwise
        // be reported as leaked memory rather than as memory in transit.
        OsTask::delay(QUEUE_DRAIN_MS);

#ifdef RTCP_STRESS_LEAK_CHECK
        // Drain the logger before snapshotting.  OsSysLog hands messages to
        // its own task, so anything emitted earlier in this method
        // in particular -- is still sitting in the queue as a formatted string
        // plus its wrapper.  That is in-flight log data, but to a heap diff it
        // is indistinguishable from a leak.
        OsSysLog::flush();

        _CrtMemCheckpoint(&memAfter);

        if (_CrtMemDifference(&memDiff, &memBefore, &memAfter))
        {
            // Both dumps go to the debugger Output window: the statistics give
            // the size and block counts, the object dump names each surviving
            // allocation with its file and line.
            _CrtMemDumpStatistics(&memDiff);
            _CrtMemDumpAllObjectsSince(&memBefore);

            CPPUNIT_ASSERT_MESSAGE(
                "RTCP session churn leaked heap memory -- see the Output window "
                "for the allocation dump",
                FALSE);
        }
#endif

        adjustRtcpPeriod(savedPeriod);

        OsSysLog::add(FAC_MP, PRI_INFO,
                      "RtcpStressTest: %d sessions torn down, %d inbound RTCP "
                      "packets dispatched, %d creation failures",
                      totalSessions, totalPacketsFed, totalFailures);

        // If no packet ever landed, the inbound half of this test did nothing
        // and a green result would be misleading.  Only meaningful when the
        // inbound path is enabled -- FEED_THREADS is an enum, so this has to
        // be a runtime test rather than a #if, which the compiler folds away.
        if (FEED_THREADS > 0)
        {
            CPPUNIT_ASSERT_MESSAGE(
                "no inbound RTCP packets were dispatched -- inbound coverage did not run",
                totalPacketsFed > 0);
        }

        // Every session and connection must have been created successfully;
        // a failure here means we exhausted something rather than raced.
        CPPUNIT_ASSERT_EQUAL(0, totalFailures);
        CPPUNIT_ASSERT_EQUAL((int)(CHURN_THREADS * ITERATIONS_PER_THREAD),
                             totalSessions);

        piRTCPControl->Release(ADD_RELEASE_CALL_ARGS(__LINE__));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(RtcpStressTest);

#endif // EXCLUDE_RTCP
