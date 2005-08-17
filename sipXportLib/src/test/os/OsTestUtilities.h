//
// Copyright (C) 2004, 2005 Pingtel Corp.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsTestUtilities_h_
#define _OsTestUtilities_h_


/**
 * Common utility functions for unittests
 */
class OsTestUtilities
{
 public:

    /**
     * Create root dir for tests
     */
    static void createTestDir(OsPath &root);
    
    /**
     * Destroy root dir for tests
     */
    static void removeTestDir(OsPath &root);

    /** 
     * create a simple buffer w/all possible chars 
     */
    static void initDummyBuffer(char *buff, int size);

    /** 
     * test that simple buffer is read correctly 
     */
    static UtlBoolean testDummyBuffer(char *buff, unsigned long size, 
            unsigned long position);

    /**
     * Create a dummy file with predicatable contents
     */
    static OsStatus createDummyFile(OsPath testFile, unsigned long size);

    /**
     * Test the contents of dummy files
     */
    static UtlBoolean verifyDummyFile(OsPath testFile, unsigned long size);
};

#endif

