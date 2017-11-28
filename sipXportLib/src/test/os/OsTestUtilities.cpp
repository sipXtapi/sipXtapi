//
// Copyright (C) 2007-2017 SIPez LLC  All rights reserved.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <os/OsIntTypes.h>

#include <string>

//Application Includes
#include <os/OsDefs.h>
#include <os/OsFileSystem.h>
#include <sipxunittests.h>
#include <sipxunit/TestUtilities.h>
#include <os/OsTestUtilities.h>

void OsTestUtilities::createTestDir(OsPath& root)
{
    OsStatus stat;

#ifdef ANDROID
    root = "/sdcard";
#else
    OsFileSystem::getWorkingDirectory(root);
#endif
    printf("Test dir root: %s\n", root.data());
    root.append(OsPath::separator).append("OsFileSystemTest");

    if (OsFileSystem::exists(root))
    {
        removeTestDir(root);
    }

    stat = OsFileSystem::createDir(root);
    CPPUNIT_ASSERT_MESSAGE("setup root test dir", stat == OS_SUCCESS);
}

void OsTestUtilities::removeTestDir(OsPath &root)
{
    OsStatus stat;
    if (OsFileSystem::exists(root))
    {
        stat = OsFileSystem::remove(root, TRUE, TRUE);
 
         KNOWN_BUG("Fails randomly on build server and fails everytime, the first time its run on a new machine", 
           "XPL-191");
        
        CPPUNIT_ASSERT_MESSAGE("teardown root test dir", stat == OS_SUCCESS);
    }
}

void OsTestUtilities::initDummyBuffer(char *buff, int size)
{
    printf("OsTestUtilities::initDummyBuffer(%p, %d)\n", buff, size);
    for (int i = 0; i < size; i++)
    {
        buff[i] = (char)(i % 256);
    }
    printf("exit OsTestUtilities::initDummyBuffer\n");
}

UtlBoolean OsTestUtilities::testDummyBuffer(char *buff, unsigned long size, unsigned long position)
{
    for (unsigned long i = 0; i < size; i++)
    {
        char expected = (char)((position + i) % 256);
        if (buff[i] != expected)
        {
            printf("buff[%li] = %i, expected = %i\n", position, buff[i], expected);
            return false;
        }
    }

    return true;
}


OsStatus OsTestUtilities::createDummyFile(OsPath testFile, unsigned long size)
{
    printf("OsTestUtilities::createDummyFile(%s, %ld)\n", testFile.data(), size);
    OsStatus stat;
    char wbuff[10000];
    unsigned long wbuffsize = (unsigned long)sizeof(wbuff);

    OsTestUtilities::initDummyBuffer(wbuff, sizeof(wbuff));
    printf("construct OsFile\n");
    OsFile wfile(testFile);
    printf("opening %s\n", testFile.data());
    stat = wfile.open(OsFile::CREATE);
    printf("created\n");
    UtlString msg("failed to create file: ");
    msg.append(testFile);
    msg.appendFormat(" error: %d", stat);
    CPPUNIT_ASSERT_MESSAGE(msg.data(), stat == OS_SUCCESS);
    if (stat == OS_SUCCESS)
    {
        printf("stat ok\n");
        unsigned long wposition = 0;
        for (int i = 0; stat == OS_SUCCESS && wposition < wbuffsize; i++)
        {
            printf("about to write\n");
            unsigned long remaining = wbuffsize - wposition;
            unsigned long byteswritten = 0;
            stat = wfile.write(wbuff + wposition, remaining, byteswritten);
            printf("write %ld bytes return: %d\n", (long)byteswritten, (int)stat);
            wposition += byteswritten;
        }

        wfile.close();
    }

    printf("exit OsTestUtilities::createDummyFile\n");
    return stat;
}

UtlBoolean OsTestUtilities::verifyDummyFile(OsPath testFile, unsigned long size)
{
    OsStatus stat;
    UtlBoolean ok = false;
    char rbuff[256];
    unsigned long rbuffsize = (unsigned long)sizeof(rbuff);
    OsFile rfile(testFile);
    stat = rfile.open();
    UtlString msg("Failed to create file: ");
    msg.append(testFile);
    CPPUNIT_ASSERT_MESSAGE(testFile.data(), stat == OS_SUCCESS);
    if (stat == OS_SUCCESS)
    {
        unsigned long rposition = 0;
        ok = true;
        for (int i = 0; ok && rposition < size; i++)
        {
            unsigned long remaining = (size - rposition);
            unsigned long readsize = remaining < rbuffsize ? remaining : rbuffsize;
            unsigned long bytesread = 0;
            stat = rfile.read(rbuff, readsize, bytesread);
            CPPUNIT_ASSERT_MESSAGE("Failed to read", stat != 0);
            if (stat != OS_SUCCESS)
            {
                ok = false;
                printf("Error reading file, status = %i", stat);
            }
            else
            {
                ok = OsTestUtilities::testDummyBuffer(rbuff, bytesread, rposition);
                rposition += bytesread;
            }
        }

        rfile.close();
    }

    return ok;
}

