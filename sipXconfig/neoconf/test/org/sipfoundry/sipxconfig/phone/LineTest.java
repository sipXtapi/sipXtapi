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
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.setting.Group;

public class LineTest extends TestCase {

    /**
     * This tests that line groups are ignored in favor of using the phone groups instead
     */
    public void testLineGroupsIgnored() {
        Phone phone = new Phone();
        Group phoneGroup = new Group();
        phone.addGroup(phoneGroup);

        Line line = phone.createLine();
        Group lineGroup = new Group();
        line.addGroup(lineGroup);
        
        assertNotSame(lineGroup, line.getGroups().iterator().next());        
    }
}
