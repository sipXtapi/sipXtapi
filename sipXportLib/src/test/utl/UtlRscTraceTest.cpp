//
// Copyright (C) 2026 SIPez LLC.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Tests for level 1 resource accounting (utl/UtlRscTrace.h): the live
// counts, the snapshot diff and its one-line format, the process numbers,
// and the force-terminated task count.

#include <sipxunittests.h>

#include <os/OsBSem.h>
#include <os/OsCSem.h>
#include <os/OsMutex.h>
#include <os/OsRWMutex.h>
#include <os/OsMsgQ.h>
#include <os/OsTimer.h>
#include <os/OsEvent.h>
#include <os/OsTask.h>
#include <utl/UtlString.h>
#include <utl/UtlRscTrace.h>

// Task whose run() returns at once; deleting it after start is clean.
class RscQuickTask : public OsTask
{
public:
   RscQuickTask(const UtlString& name) : OsTask(name) {}
   virtual ~RscQuickTask() { waitUntilShutDown(); }
   virtual int run(void* pArg) { return 0; }
};

// Task that blocks forever on a semaphore nobody releases, so the only way
// to end it is a forced terminate.
class RscBlockedTask : public OsTask
{
public:
   RscBlockedTask(const UtlString& name, OsBSem* pSem)
   : OsTask(name), mpSem(pSem) {}
   virtual int run(void* pArg) { mpSem->acquire(); return 0; }
private:
   OsBSem* mpSem;
};

class UtlRscTraceTest : public SIPX_UNIT_BASE_CLASS
{
   CPPUNIT_TEST_SUITE(UtlRscTraceTest);
   CPPUNIT_TEST(testSyncObjectCounts);
   CPPUNIT_TEST(testQueueTimerTaskCounts);
   CPPUNIT_TEST(testSnapshotDeltaAndNames);
   CPPUNIT_TEST(testProcessStats);
   CPPUNIT_TEST(testForcedTerminateCounted);
   CPPUNIT_TEST_SUITE_END();

public:

   // One object of each sync type moves its own count by exactly one up
   // and back. Other counts may move: some types are built from others.
   void testSyncObjectCounts()
   {
      long before = UtlRscCount::live(UtlRscCount::OSBSEM);
      OsBSem* pBSem = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
      CPPUNIT_ASSERT_EQUAL(before + 1, UtlRscCount::live(UtlRscCount::OSBSEM));
      delete pBSem;
      CPPUNIT_ASSERT_EQUAL(before, UtlRscCount::live(UtlRscCount::OSBSEM));

      before = UtlRscCount::live(UtlRscCount::OSCSEM);
      OsCSem* pCSem = new OsCSem(OsCSem::Q_PRIORITY, 4, 0);
      CPPUNIT_ASSERT_EQUAL(before + 1, UtlRscCount::live(UtlRscCount::OSCSEM));
      delete pCSem;
      CPPUNIT_ASSERT_EQUAL(before, UtlRscCount::live(UtlRscCount::OSCSEM));

      before = UtlRscCount::live(UtlRscCount::OSMUTEX);
      OsMutex* pMutex = new OsMutex(OsMutex::Q_PRIORITY);
      CPPUNIT_ASSERT_EQUAL(before + 1, UtlRscCount::live(UtlRscCount::OSMUTEX));
      delete pMutex;
      CPPUNIT_ASSERT_EQUAL(before, UtlRscCount::live(UtlRscCount::OSMUTEX));

      before = UtlRscCount::live(UtlRscCount::OSRWMUTEX);
      OsRWMutex* pRW = new OsRWMutex(OsRWMutex::Q_PRIORITY);
      CPPUNIT_ASSERT_EQUAL(before + 1, UtlRscCount::live(UtlRscCount::OSRWMUTEX));
      delete pRW;
      CPPUNIT_ASSERT_EQUAL(before, UtlRscCount::live(UtlRscCount::OSRWMUTEX));

      CPPUNIT_ASSERT(UtlRscCount::typeName(UtlRscCount::OSBSEM) != NULL);
      CPPUNIT_ASSERT(UtlRscCount::typeName(UtlRscCount::MEMORY) == NULL);
   }

   void testQueueTimerTaskCounts()
   {
      long before = UtlRscCount::live(UtlRscCount::OSMSGQ);
      OsMsgQ* pQ = new OsMsgQ();
      CPPUNIT_ASSERT_EQUAL(before + 1, UtlRscCount::live(UtlRscCount::OSMSGQ));
      delete pQ;
      CPPUNIT_ASSERT_EQUAL(before, UtlRscCount::live(UtlRscCount::OSMSGQ));

      OsEvent event;
      before = UtlRscCount::live(UtlRscCount::OSTIMER);
      OsTimer* pTimer = new OsTimer(event);
      CPPUNIT_ASSERT_EQUAL(before + 1, UtlRscCount::live(UtlRscCount::OSTIMER));
      delete pTimer;
      CPPUNIT_ASSERT_EQUAL(before, UtlRscCount::live(UtlRscCount::OSTIMER));

      before = UtlRscCount::live(UtlRscCount::OSTASK);
      RscQuickTask* pTask = new RscQuickTask("RscQuickTask-%d");
      CPPUNIT_ASSERT_EQUAL(before + 1, UtlRscCount::live(UtlRscCount::OSTASK));
      CPPUNIT_ASSERT(pTask->start());
      delete pTask;
      CPPUNIT_ASSERT_EQUAL(before, UtlRscCount::live(UtlRscCount::OSTASK));
   }

   // A snapshot diff reports created objects and the names of new named
   // queues and tasks, and reports nothing once they are gone again.
   void testSnapshotDeltaAndNames()
   {
      UtlRscSnapshot before;

      OsMsgQ* pQ = new OsMsgQ(OsMsgQ::DEF_MAX_MSGS, OsMsgQ::DEF_MAX_MSG_LEN,
                              OsMsgQ::Q_PRIORITY, "RscSnapQueue");
      RscQuickTask* pTask = new RscQuickTask("RscSnapTask");
      UtlRscSnapshot during;

      CPPUNIT_ASSERT_EQUAL(before.live(UtlRscCount::OSMSGQ) + 1,
                           during.live(UtlRscCount::OSMSGQ));
      CPPUNIT_ASSERT_EQUAL(before.live(UtlRscCount::OSTASK) + 1,
                           during.live(UtlRscCount::OSTASK));
      CPPUNIT_ASSERT(during.totalDelta(before) >= 2);

      UtlString line;
      during.formatDelta(before, "unit", line, 2);
      printf("%s\n", line.data());
      CPPUNIT_ASSERT_MESSAGE(line.data(), line.index("RSC unit:") == 0);
      CPPUNIT_ASSERT_MESSAGE(line.data(), line.index(" OsMsgQ +1(+0.50/op)") != UTL_NOT_FOUND);
      CPPUNIT_ASSERT_MESSAGE(line.data(), line.index(" OsTask +1(+0.50/op)") != UTL_NOT_FOUND);
      CPPUNIT_ASSERT_MESSAGE(line.data(), line.index("MsgQ.RscSnapQueue") != UTL_NOT_FOUND);
      CPPUNIT_ASSERT_MESSAGE(line.data(), line.index("Task.RscSnapTask") != UTL_NOT_FOUND);
      CPPUNIT_ASSERT_MESSAGE(line.data(), line.index(" new[") != UTL_NOT_FOUND);
      CPPUNIT_ASSERT_MESSAGE(line.data(), line.index(" killed +0") != UTL_NOT_FOUND);

      // The reverse diff lists them as gone.
      before.formatDelta(during, "reverse", line);
      CPPUNIT_ASSERT_MESSAGE(line.data(), line.index(" gone[") != UTL_NOT_FOUND);
      CPPUNIT_ASSERT_MESSAGE(line.data(), line.index("Task.RscSnapTask") != UTL_NOT_FOUND);

      CPPUNIT_ASSERT(pTask->start());
      delete pTask;
      delete pQ;

      UtlRscSnapshot after;
      CPPUNIT_ASSERT_EQUAL(before.live(UtlRscCount::OSMSGQ),
                           after.live(UtlRscCount::OSMSGQ));
      CPPUNIT_ASSERT_EQUAL(before.live(UtlRscCount::OSTASK),
                           after.live(UtlRscCount::OSTASK));
      after.formatDelta(before, "after", line);
      CPPUNIT_ASSERT_MESSAGE(line.data(), line.index("RscSnapQueue") == UTL_NOT_FOUND);
      CPPUNIT_ASSERT_MESSAGE(line.data(), line.index("RscSnapTask") == UTL_NOT_FOUND);
   }

   void testProcessStats()
   {
      UtlRscProcessStats stats;
      utlRscGetProcessStats(stats);
      CPPUNIT_ASSERT(stats.threads >= 1);
      CPPUNIT_ASSERT(stats.handles >= 1);
      CPPUNIT_ASSERT(stats.privateKB > 0);
   }

   // A task that cannot exit is force-terminated by deleteForce, and that
   // is counted (and logged as CRIT). Takes about 5 s on Windows (the
   // OsTaskWnt grace period) and 1 s on Linux.
   //
   // The task object and its semaphore are leaked on purpose: a killed
   // thread may leave the semaphore's internal lock held, and deleting the
   // task would wait for a shutdown that can never be acknowledged.
   void testForcedTerminateCounted()
   {
      OsBSem* pNeverReleased = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::EMPTY);
      RscBlockedTask* pTask = new RscBlockedTask("RscBlockedTask", pNeverReleased);
      CPPUNIT_ASSERT(pTask->start());
      OsTask::delay(100);   // let it block in acquire()

      long before = UtlRscCount::tasksTerminated();
      pTask->deleteForce();
      CPPUNIT_ASSERT_EQUAL(before + 1, UtlRscCount::tasksTerminated());
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(UtlRscTraceTest);
