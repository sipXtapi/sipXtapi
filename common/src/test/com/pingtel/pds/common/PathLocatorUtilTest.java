/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package com.pingtel.pds.common;

import java.io.ByteArrayInputStream;
import java.io.IOException;

import junit.framework.TestCase;

/**
 * PathLocatorUtilTest
 */
public class PathLocatorUtilTest extends TestCase {
    private PathLocatorUtil m_plu;
    
    protected void setUp() throws Exception {
        m_plu = new PathLocatorUtil(new MockVersionInfo());
    }

    public void testData() {
        assertEquals("/opt/work-main/sipx/share/sipxpbx",m_plu.data().toString());
    }

    public void testConf() {
        assertEquals("/opt/work-main/sipx/etc/sipxpbx",m_plu.conf().toString());
    }

    public void testLog() {
        assertEquals("/opt/work-main/sipx/var/log/sipxpbx",m_plu.log().toString());
    }

    public void testTmp() {
        assertEquals("/opt/work-main/sipx/var/tmp",m_plu.tmp().toString());
    }

    public void testPhone() {
        assertEquals("/opt/work-main/sipx/var/sipxdata/configserver/phone",m_plu.phone().toString());
    }

    public void testGetPath() {
        String path;
        path = m_plu.getPathSilent(PathLocatorUtil.XSLT_FOLDER);
        assertEquals("/opt/work-main/sipx/share/sipxpbx/xslt/",path);
        
        path = m_plu.getPathSilent(PathLocatorUtil.LOGS_FOLDER);
        assertEquals("/opt/work-main/sipx/var/log/sipxpbx/",path);
        
        path = m_plu.getPathSilent(PathLocatorUtil.CONFIG_FOLDER);
        assertEquals("/opt/work-main/sipx/etc/sipxpbx/",path);
        
        path = m_plu.getPathSilent(PathLocatorUtil.PHONEDEFS_FOLDER);
        assertEquals("/opt/work-main/sipx/share/sipxpbx/devicedefs/",path);

        path = m_plu.getPathSilent(PathLocatorUtil.PGS_LOGS_FOLDER);
        assertEquals("/opt/work-main/sipx/var/log/sipxpbx/",path);        
        path = m_plu.getPathSilent(PathLocatorUtil.DATA_FOLDER);
        assertEquals("/opt/work-main/sipx/var/sipxdata/configserver/phone/",path);        

        /* TODO: remove these constants - we do not use them any more        
        path = m_plu.getPathSilent(PathLocatorUtil.LICENSE_FOLDER);
        assertEquals("/opt/work-main/sipx/share/sipxpbx/",path);
        path = m_plu.getPathSilent(PathLocatorUtil.PATCHES_FOLDER);
        assertEquals("/opt/work-main/sipx/var/sipxdata/configserver/phone",path);
        path = m_plu.getPathSilent(PathLocatorUtil.DATASET_ADDINS_FOLDER);
        assertEquals("/opt/work-main/sipx/var/sipxdata/configserver/phone",path);
        */
    }

    public static String VERSION_INFO =
        "#Thu Feb 05 01:00:14 EST 2004\n"+
        "sysdir.version=2.6.0\n"+
        "sysdir.build_number=0000\n"+
        "sysdir.comment=opendev\n"+
        "sysdir.etc=/opt/work-main/sipx/etc/sipxpbx\n" + 
        "sysdir.share=/opt/work-main/sipx/share/sipxpbx\n" +
        "sysdir.tmp=/opt/work-main/sipx/var/tmp\n" +
        "sysdir.phone=/opt/work-main/sipx/var/sipxdata/configserver/phone\n" +
        "sysdir.log=/opt/work-main/sipx/var/log/sipxpbx\n";

    private class MockVersionInfo extends VersionInfo
    {
        public MockVersionInfo() throws IOException {
            super(new ByteArrayInputStream(VERSION_INFO.getBytes()));
        }
    }
}
