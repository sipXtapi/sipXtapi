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
package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;

/**
 * Comments
 */
public class TabsTest extends TestCase {
    
    public void testTabs() {
        Tabs tabs = new Tabs();
        String activeTab = "active tab"; 
        tabs.setActiveTab(activeTab);
        assertEquals(Tabs.ACTIVE_CLASS, tabs.getCssClass(activeTab));
        assertEquals(Tabs.INACTIVE_CLASS, tabs.getCssClass("inactive tab"));
    }
}
