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
        "sysdir.version=2.6.0\n"+
        "sysdir.build_number=0000\n"+
        "sysdir.comment=opendev\n"+
        "sysdir.etc=/usr/local/sipx/etc/sipxpbx\n" + 
        "sysdir.share=/usr/local/sipx/share/sipxpbx\n" +
        "sysdir.tmp=/usr/local/sipx/var/tmp\n" +
        "sysdir.phone=/opt/work-main/sipx/var/sipxdata/configserver/phone\n" +
        "sysdir.log=/usr/local/sipx/var/log/sipxpbx\n";

    public void testVersionInfo() throws IOException {
        InputStream stream = new ByteArrayInputStream(VERSION_INFO.getBytes());
        VersionInfo info = new VersionInfo(stream);
        assertEquals("Version: 2.6.0",info.getVersion());
        assertEquals("opendev",info.getProperty("comment"));
        assertEquals("Version: 2.6.0 (opendev) Build #: 0000",info.getVersion(VersionInfo.LONG_VERSION_INFO));
        assertEquals("opendev",info.getProperty("comment"));
        assertEquals("Version: 2.6.0",info.getVersion(VersionInfo.SHORT_VERSION_INFO));
        assertEquals("opendev",info.getProperty("comment"));
	}
	
}
