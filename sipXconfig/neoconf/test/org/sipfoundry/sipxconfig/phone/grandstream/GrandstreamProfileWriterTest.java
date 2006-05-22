/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.io.ByteArrayOutputStream;

import junit.framework.TestCase;

public class GrandstreamProfileWriterTest extends TestCase {
    
    public void testWriteIpAddress() {
        ByteArrayOutputStream actual = new ByteArrayOutputStream();
        GrandstreamProfileWriter wtr = new GrandstreamProfileWriter(null);
        wtr.setOutputStream(actual);
        wtr.writeIpAddress("bird", null);
        assertEquals("bird = " + GrandstreamProfileWriter.LF, actual.toString());
    }

}
