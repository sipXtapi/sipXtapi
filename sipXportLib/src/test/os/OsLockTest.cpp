//
// Copyright (C) 2007-2010 SIPez LLC  All rights reserved.
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

#include <os/OsBSem.h>
#include <os/OsRWMutex.h>
#include <os/OsLock.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <sipxunittests.h>

class OsLockTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(OsLockTest);
    CPPUNIT_TEST(testLockBasicSemaphore);
    CPPUNIT_TEST(testLockMutex);
    CPPUNIT_TEST_SUITE_END();


public:

    /**
     * Locking of  basic semaphores
     */
    void testLockBasicSemaphore()
    {
        // Create a binary semaphore for use with an OsLock object
        OsBSem *pBSem = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);

        // Acquire semaphore at the start of the block, release it on exit block
        {
            int ret = pBSem->tryAcquire();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", OS_SUCCESS, ret);
            ret = pBSem->release();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", OS_SUCCESS, ret);

            OsLock lock(*pBSem);
            // if this were a real guarded method, we'd do useful work here
            // destroying the OsReadLock variable that has been allocated on the stack
            // should release the reader lock automatically when we leave block

            // Should not be available
            ret = pBSem->tryAcquire();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", OS_BUSY, ret);
        }

        delete pBSem;
    }

    /**
     * Locking of mutex'es
     */
    void testLockMutex()
    {
        // Create an OsRWMutex for use with OsReadLock and OsWriteLock objects
        OsRWMutex *pRWMutex = new OsRWMutex(OsRWMutex::Q_FIFO);

        int ret = 0;

        // Acquire read lock at the start of the block, release it on exit
        {
            ret = pRWMutex->tryAcquireRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", OS_SUCCESS, ret);

            // Multiple read should be allowed
            ret = pRWMutex->tryAcquireRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", OS_SUCCESS, ret);

            // Should not be available
            ret = pRWMutex->tryAcquireWrite();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should not be available", OS_BUSY, ret);

            ret = pRWMutex->releaseRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", OS_SUCCESS, ret);
            ret = pRWMutex->releaseRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", OS_SUCCESS, ret);

            OsReadLock lock(*pRWMutex);

            // Should not be available
            ret = pRWMutex->tryAcquireWrite();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should not be available", OS_BUSY, ret);

            // Multiple read should be allowed
            ret = pRWMutex->tryAcquireRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", OS_SUCCESS, ret);
            ret = pRWMutex->releaseRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", OS_SUCCESS, ret);

        }

        // Should be available
        ret = pRWMutex->tryAcquireWrite();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", OS_SUCCESS, ret);
        ret = pRWMutex->releaseWrite();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should release", OS_SUCCESS, ret);

        // Acquire write lock at the start of the block, release it on exit
        {
            ret = pRWMutex->tryAcquireRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", OS_SUCCESS, ret);
            ret = pRWMutex->releaseRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should release", OS_SUCCESS, ret);


            // Should be available
            ret = pRWMutex->tryAcquireWrite();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", OS_SUCCESS, ret);
            ret = pRWMutex->releaseWrite();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should release", OS_SUCCESS, ret);


            OsWriteLock lock(*pRWMutex);

            // Should not be available
            ret = pRWMutex->tryAcquireWrite();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", OS_BUSY, ret);
            ret = pRWMutex->tryAcquireRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", OS_BUSY, ret);
        }

        // Should be available
        ret = pRWMutex->tryAcquireWrite();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", OS_SUCCESS, ret);
        ret = pRWMutex->releaseWrite();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should release", OS_SUCCESS, ret);

        delete pRWMutex;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsLockTest);

