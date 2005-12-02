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
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

public class SettingBeanAdapterTest extends TestCase {
    
    SettingBeanAdapter adapter;
    
    Bird bird;
    
    Setting birdSettings;
    
    protected void setUp() {
        adapter = new SettingBeanAdapter(Bird.class);
        adapter.addMapping("wingSpan", "wingSpan");
        
        birdSettings = new SettingSet("birds");
        birdSettings.addSetting(new SettingImpl("wingSpan"));
        adapter.setSetting(birdSettings);
        
        bird = (Bird) adapter.getImplementation();
    }

    public void testSetSetting() {
        bird.setWingSpan("100");
        assertEquals("100", birdSettings.getSetting("wingSpan").getValue());        
    }

    public void testGetSetting() {
        birdSettings.getSetting("wingSpan").setValue("50");
        assertEquals("50", bird.getWingSpan());        
    }

    public void testGetAccessor() {
        assertEquals("wingSpan", SettingBeanAdapter.getAccessor("get", "getWingSpan"));
    }
    
    public static interface Bird {
        
        public void setWingSpan(String wingSpan);
        
        public String getWingSpan();
    }
}

