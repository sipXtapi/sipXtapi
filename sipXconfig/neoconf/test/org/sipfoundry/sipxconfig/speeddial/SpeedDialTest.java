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
package org.sipfoundry.sipxconfig.speeddial;

import org.sipfoundry.sipxconfig.common.User;

import junit.framework.TestCase;

public class SpeedDialTest extends TestCase {

    public void testGetResourceListId() {
        User user = new User() {
            public Integer getId() {
                return 115;
            }
        };

        SpeedDial sd = new SpeedDial();
        sd.setUser(user);
        assertEquals("~~rl~115", sd.getResourceListId(false));
        assertEquals("~~rl~115c", sd.getResourceListId(true));
    }
}
