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

import java.io.OutputStream;
import java.io.Reader;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;

public class MemoryProfileLocationTest extends TestCase {
    public void testGetOutput() throws Exception {
        MemoryProfileLocation location = new MemoryProfileLocation();
        
        OutputStream output = location.getOutput("abc.txt");
        for (char c = 'a'; c < 'e'; c++) {
            output.write(c);
        }
        output.close();        
        assertEquals("abcd", location.toString());
        
        Reader reader = location.getReader();
        assertEquals("abcd", IOUtils.toString(reader));
    }
}
