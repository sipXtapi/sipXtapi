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
        assertEquals("  ()", info.getLongVersionString());
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
        assertEquals("title version (build)", info.getLongVersionString());
    }
}
