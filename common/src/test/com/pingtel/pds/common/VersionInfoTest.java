/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.common;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;

import junit.framework.TestCase;

public class VersionInfoTest extends TestCase {
    public static String VERSION_INFO =
        "#Thu Feb 05 01:00:14 EST 2004\n"+
        "version=2.6.0\n"+
        "build_number=0000\n"+
        "comment=opendev\n"+
        "sipxconf=/usr/local/sipx/etc/sipxpbx\n" + 
        "sipxdata=/usr/local/sipx/share/sipxpbx\n" +
        "sipxtmp=/usr/local/sipx/var/tmp\n" +
        "sipxlog=/usr/local/sipx/var/log/sipxpbx\n";

    public void testVersionInfo() throws IOException {
        InputStream stream = new ByteArrayInputStream(VERSION_INFO.getBytes());
        VersionInfo info = new VersionInfo(stream);
        assertEquals("Version: 2.6.0 (opendev) Build #: 0000",info.getVersion());
        assertEquals("opendev",info.getProperty("comment"));
	}
	
}
