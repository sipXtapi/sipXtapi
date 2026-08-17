//
// Copyright (C) 2026 SIP Spectrum, Inc.  All rights reserved.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef EXCLUDE_RTCP

#include <os/OsIntTypes.h>

#include <sipxunittests.h>
#include "rtcp/TLinkedList.h"

/**
 * Unittest for CTLinkedList
 *
 * CTLinkedList underpins every RTCP collection: the RTCManager session and
 * event-registration lists, the session's connection list, the CMsgQueue
 * message list on Win32, and the source-description/receiver-report lists in
 * CRTCPSource.  Several of those are drained with the
 * RemoveFirstEntry()/RemoveNextEntry() idiom during teardown, so the removal
 * paths are what these tests concentrate on.
 */
class TLinkedListTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(TLinkedListTest);

    CPPUNIT_TEST(testAddEntryUpdatesCount);
    CPPUNIT_TEST(testRemoveFirstEntryOnEmptyList);
    CPPUNIT_TEST(testRemoveFirstEntryReturnsSoleEntry);
    CPPUNIT_TEST(testRemoveFirstNextDrainsList);
    CPPUNIT_TEST(testRepeatedRemoveFirstEntryDrainsList);
    CPPUNIT_TEST(testGetFirstNextDoesNotRemove);
    CPPUNIT_TEST(testListIsReusableAfterDrain);

    CPPUNIT_TEST_SUITE_END();

public:

    void testAddEntryUpdatesCount()
    {
        int a = 1, b = 2, c = 3;
        CTLinkedList<int *> list;

        CPPUNIT_ASSERT_EQUAL((unsigned long)0, list.GetCount());

        CPPUNIT_ASSERT(list.AddEntry(&a));
        CPPUNIT_ASSERT(list.AddEntry(&b));
        CPPUNIT_ASSERT(list.AddEntry(&c));

        CPPUNIT_ASSERT_EQUAL((unsigned long)3, list.GetCount());
    }

    void testRemoveFirstEntryOnEmptyList()
    {
        CTLinkedList<int *> list;

        // An empty list has nothing to hand back, and must not go negative.
        CPPUNIT_ASSERT(list.RemoveFirstEntry() == NULL);
        CPPUNIT_ASSERT_EQUAL((unsigned long)0, list.GetCount());
    }

    // Regression test.
    //
    // RemoveFirstEntry() used to test the iterator with an inverted condition
    // ("if (!ptLink)"), so it removed the link only when there was no link to
    // remove.  On any NON-empty list it therefore returned NULL and removed
    // nothing, silently turning every drain loop built on it into a no-op --
    // most damagingly CRTCPSession::TerminateAllConnections(), which then never
    // stopped connection report timers nor released renderers.
    void testRemoveFirstEntryReturnsSoleEntry()
    {
        int a = 1;
        CTLinkedList<int *> list;

        CPPUNIT_ASSERT(list.AddEntry(&a));

        int *removed = list.RemoveFirstEntry();
        CPPUNIT_ASSERT(removed == &a);
        CPPUNIT_ASSERT_EQUAL((unsigned long)0, list.GetCount());
    }

    // The drain idiom used by CRTCManager::~CRTCManager(),
    // CRTCPSource::~CRTCPSource() and CMsgQueue::FlushMessages():
    // RemoveFirstEntry() once, then RemoveNextEntry() until NULL, all under a
    // single held lock because the four iteration methods share one cursor.
    void testRemoveFirstNextDrainsList()
    {
        int a = 1, b = 2, c = 3;
        int *seen[3];
        int count = 0;
        CTLinkedList<int *> list;

        list.AddEntry(&a);
        list.AddEntry(&b);
        list.AddEntry(&c);

        list.TakeLock();
        int *entry = list.RemoveFirstEntry();
        while (entry != NULL && count < 3)
        {
            seen[count++] = entry;
            entry = list.RemoveNextEntry();
        }
        list.ReleaseLock();

        CPPUNIT_ASSERT_EQUAL(3, count);
        CPPUNIT_ASSERT_EQUAL((unsigned long)0, list.GetCount());

        // Entries come back oldest first
        CPPUNIT_ASSERT(seen[0] == &a);
        CPPUNIT_ASSERT(seen[1] == &b);
        CPPUNIT_ASSERT(seen[2] == &c);
    }

    // The drain idiom used by CRTCPSession::TerminateAllConnections(), which
    // must release the lock between removals so that connection teardown does
    // not run underneath it.  Because the list iterator is shared mutable
    // state, a cursor cannot be carried across the unlocked window, so the
    // FIRST entry is re-read on every pass instead of using RemoveNextEntry().
    void testRepeatedRemoveFirstEntryDrainsList()
    {
        int a = 1, b = 2, c = 3;
        int *seen[3];
        int count = 0;
        CTLinkedList<int *> list;

        list.AddEntry(&a);
        list.AddEntry(&b);
        list.AddEntry(&c);

        while (count < 3)
        {
            list.TakeLock();
            int *entry = list.RemoveFirstEntry();
            list.ReleaseLock();

            if (entry == NULL)
            {
                break;
            }
            seen[count++] = entry;
        }

        CPPUNIT_ASSERT_EQUAL(3, count);
        CPPUNIT_ASSERT_EQUAL((unsigned long)0, list.GetCount());
        CPPUNIT_ASSERT(list.RemoveFirstEntry() == NULL);

        // Removing the first entry repeatedly still yields oldest first
        CPPUNIT_ASSERT(seen[0] == &a);
        CPPUNIT_ASSERT(seen[1] == &b);
        CPPUNIT_ASSERT(seen[2] == &c);
    }

    void testGetFirstNextDoesNotRemove()
    {
        int a = 1, b = 2;
        CTLinkedList<int *> list;

        list.AddEntry(&a);
        list.AddEntry(&b);

        list.TakeLock();
        CPPUNIT_ASSERT(list.GetFirstEntry() == &a);
        CPPUNIT_ASSERT(list.GetNextEntry() == &b);
        CPPUNIT_ASSERT(list.GetNextEntry() == NULL);
        list.ReleaseLock();

        // Walking the list must leave it intact
        CPPUNIT_ASSERT_EQUAL((unsigned long)2, list.GetCount());
    }

    // Draining a list to empty must leave the head/tail pointers in a state
    // that can accept new entries; the RTCP message queue and session lists are
    // all emptied and refilled repeatedly over the life of the process.
    void testListIsReusableAfterDrain()
    {
        int a = 1, b = 2, c = 3;
        CTLinkedList<int *> list;

        list.AddEntry(&a);
        list.AddEntry(&b);

        CPPUNIT_ASSERT(list.RemoveFirstEntry() == &a);
        CPPUNIT_ASSERT(list.RemoveFirstEntry() == &b);
        CPPUNIT_ASSERT_EQUAL((unsigned long)0, list.GetCount());

        // Refill and drain again
        CPPUNIT_ASSERT(list.AddEntry(&c));
        CPPUNIT_ASSERT_EQUAL((unsigned long)1, list.GetCount());
        CPPUNIT_ASSERT(list.RemoveFirstEntry() == &c);
        CPPUNIT_ASSERT_EQUAL((unsigned long)0, list.GetCount());
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TLinkedListTest);

#endif // EXCLUDE_RTCP
