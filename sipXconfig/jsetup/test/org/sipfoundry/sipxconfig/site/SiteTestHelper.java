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
package org.sipfoundry.sipxconfig.site;

import org.sipfoundry.sipxconfig.common.TestUtil;


public class SiteTestHelper {

    private static String s_buildDir;

    public static String getBuildDirectory() {
        if (s_buildDir == null) {
            s_buildDir = TestUtil.getBuildDirectory("jsetup");
        }

        return s_buildDir;
    }

}
