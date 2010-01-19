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
#include <os/OsCSem.h>
#include <os/OsRWMutex.h>
#include <os/OsMutex.h>
#include <os/OsLock.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <sipxunittests.h>

class LockOnMutexMember
{
    public: 
    LockOnMutexMember() :
       mMutex(OsMutex::Q_PRIORITY + OsMutex::INVERSION_SAFE + OsMutex::DELETE_SAFE),
       mCounter(OsMutex::Q_PRIORITY + OsMutex::INVERSION_SAFE + OsMutex::DELETE_SAFE)
       //mCounter(OsCSem::Q_PRIORITY, 1000, 1000, &mMutex)
    {
        //printf("LockOnMutexMember constructed\n");
    };

    void doLockedStuff()
    {
        {
            OsLock lock(mMutex);
        }
    }

    OsMutex mMutex;
    OsMutex mCounter;
};

class OsLockTest : public SIPX_UNIT_BASE_CLASS
{
    CPPUNIT_TEST_SUITE(OsLockTest);
    CPPUNIT_TEST(testLockBasicSemaphore);
    CPPUNIT_TEST(testLockMutex);
    CPPUNIT_TEST(testLockedMember);
    CPPUNIT_TEST_SUITE_END();


public:

    /**
     * Locking of  basic semaphores
     */
    void testLockBasicSemaphore()
    {
        // Create a binary semaphore for use with an OsLock object
        OsBSem *pBSem = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
        int ret = 0;

        // Acquire semaphore at the start of the block, release it on exit block
        {
            ret = pBSem->tryAcquire();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", (int)OS_SUCCESS, ret);
            ret = pBSem->release();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", (int)OS_SUCCESS, ret);

            OsLock lock(*pBSem);
            // if this were a real guarded method, we'd do useful work here
            // destroying the OsReadLock variable that has been allocated on the stack
            // should release the reader lock automatically when we leave block

            // Should not be available
            ret = pBSem->tryAcquire();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", (int)OS_BUSY, ret);
        }

        // Should be available
        ret = pBSem->tryAcquire();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", (int)OS_SUCCESS, ret);

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
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", (int)OS_SUCCESS, ret);

            // Multiple read should be allowed
            ret = pRWMutex->tryAcquireRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", (int)OS_SUCCESS, ret);

            // Should not be available
            ret = pRWMutex->tryAcquireWrite();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should not be available", (int)OS_BUSY, ret);

            ret = pRWMutex->releaseRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", (int)OS_SUCCESS, ret);
            ret = pRWMutex->releaseRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", (int)OS_SUCCESS, ret);

            OsReadLock lock(*pRWMutex);

            // Should not be available
            ret = pRWMutex->tryAcquireWrite();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should not be available", (int)OS_BUSY, ret);

            // Multiple read should be allowed
            ret = pRWMutex->tryAcquireRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", (int)OS_SUCCESS, ret);
            ret = pRWMutex->releaseRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", (int)OS_SUCCESS, ret);

        }

        // Should be available
        ret = pRWMutex->tryAcquireWrite();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", (int)OS_SUCCESS, ret);
        ret = pRWMutex->releaseWrite();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should release", (int)OS_SUCCESS, ret);

        // Acquire write lock at the start of the block, release it on exit
        {
            ret = pRWMutex->tryAcquireRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", (int)OS_SUCCESS, ret);
            ret = pRWMutex->releaseRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should release", (int)OS_SUCCESS, ret);


            // Should be available
            ret = pRWMutex->tryAcquireWrite();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", (int)OS_SUCCESS, ret);
            ret = pRWMutex->releaseWrite();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("should release", (int)OS_SUCCESS, ret);


            OsWriteLock lock(*pRWMutex);

            // Should not be available
            ret = pRWMutex->tryAcquireWrite();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", (int)OS_BUSY, ret);
            ret = pRWMutex->tryAcquireRead();
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Semaphor should be available", (int)OS_BUSY, ret);
        }

        // Should be available
        ret = pRWMutex->tryAcquireWrite();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should be available", (int)OS_SUCCESS, ret);
        ret = pRWMutex->releaseWrite();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("should release", (int)OS_SUCCESS, ret);

        delete pRWMutex;
    }

    void testLockedMember()
    {
        int ret = 0;
        OsMutex localMutex(OsMutex::Q_PRIORITY + OsMutex::INVERSION_SAFE + OsMutex::DELETE_SAFE);
        localMutex.OsMutexShow();
        ret = localMutex.tryAcquire();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("mutex should be available, it was just initialized", (int) OS_SUCCESS, ret);

        LockOnMutexMember lockingClass;

        ret = lockingClass.mMutex.tryAcquire();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("mutex should be available, it was just initialized", (int) OS_SUCCESS, ret);

        ret = lockingClass.mMutex.release();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("mutext should release", (int) OS_SUCCESS, ret);

        // Not a very friendly test.  This test will block if lock cannot be taken
        lockingClass.doLockedStuff();
        CPPUNIT_ASSERT_MESSAGE("the fact that we got here is a pass of the test", true);

    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsLockTest);

