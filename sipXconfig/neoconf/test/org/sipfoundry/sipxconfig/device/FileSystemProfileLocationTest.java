/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.OutputStream;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class FileSystemProfileLocationTest extends TestCase {
    private File m_parentDir;

    protected void setUp() throws Exception {
        m_parentDir = new File(TestHelper.getTestDirectory(), getClass().getName());
    }

    protected void tearDown() throws Exception {
        m_parentDir.delete();
    }

    public void testGetOutput() throws IOException {
        FileSystemProfileLocation location = new FileSystemProfileLocation();

        location.setParentDir(m_parentDir.getPath());

        OutputStream output = location.getOutput("abc.txt");
        int len = 4;

        for (byte b = 0; b < 4; b++) {
            output.write(2 * b);
        }
        output.close();

        File profile = new File(m_parentDir, "abc.txt");

        assertTrue(profile.exists());
        FileInputStream profileStream = new FileInputStream(profile);
        byte[] content = new byte[len];
        profileStream.read(content);
        for (int i = 0; i < len; i++) {
            assertEquals(2 * i, content[i]);
        }

        assertEquals(-1, profileStream.read());
        profileStream.close();

        location.removeProfile("abc.txt");
        assertFalse(profile.exists());
    }
}
