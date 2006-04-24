/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;

public class VersionInfoTest extends TestCase {

    public void testGetLongVersionStringEmpty() {
        // no package information during tests
        VersionInfo info = new VersionInfo();
        assertEquals(" ()", info.getVersionString());
    }
    
    public void testGetVersionIds() {
        Integer[] ids = VersionInfo.versionStringToVersionIds("12.34.56");
        assertEquals(3, ids.length);
        assertEquals(12, ids[0].intValue());
        assertEquals(34, ids[1].intValue());
        assertEquals(56, ids[2].intValue());
    }

    public void testGetLongVersionString() {
        VersionInfo info = new VersionInfo() {
            public String getBuild() {
                return "build";
            }

            public String getTitle() {
                return "title";
            }

            public String getVersion() {
                return "version";
            }
        };
        assertEquals("version (build)", info.getVersionString());
    }
}
