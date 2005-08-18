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

import org.sipfoundry.sipxconfig.TestHelper;

import junit.framework.TestCase;

public class SettingModelTest extends TestCase {
    
    SettingModel model;
    
    Setting settings;
    
    protected void setUp() {
        settings = TestHelper.loadSettings("../test/org/sipfoundry/sipxconfig/setting/simplemodel.xml");
        model = new SettingModel();
        model.setDelegate(settings);        
    }
    
    public void testCopy() {
        SettingModel copy = (SettingModel) model.copy();
        assertNotSame(settings, copy.getDelegate());
    }
    
    public void testGroupDecorate() {
        SettingModel copy = (SettingModel) model.copy();
        
        Group g = new Group();
        g.decorate(copy);
        
        copy.getSetting("group/setting").setValue("foo");
        
        String decoratedValue = copy.getSetting("group/setting").getValue();
        String originalValue = settings.getSetting("group/setting").getValue();
        assertNotSame(originalValue, decoratedValue);
    }
}
