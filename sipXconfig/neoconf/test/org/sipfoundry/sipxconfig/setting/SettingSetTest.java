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
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class SettingSetTest extends TestCase {
    private SettingSet m_set;
    private Setting[] m_settings = new Setting[] {
        new SettingImpl("kuku"),
        new SettingSet("bongo"),
        new SettingImpl("kuku1")
    };
    
    protected void setUp() {
        m_set = new SettingSet();
        for (Setting setting : m_settings) {
            m_set.addSetting(setting);
        }
    }

    public void testGetDefaultSetting() {
        assertEquals("kuku", m_set.getDefaultSetting(Setting.class).getName());
        assertEquals("bongo", m_set.getDefaultSetting(SettingSet.class).getName());
    }
    
    public void testVisitSettingGroup() {
        IMocksControl settingVisitorControl = EasyMock.createControl();
        SettingVisitor settingVisitor = settingVisitorControl.createMock(SettingVisitor.class);
        settingVisitor.visitSettingGroup(m_set);
        settingVisitorControl.andReturn(false);
        settingVisitorControl.replay();
        m_set.acceptVisitor(settingVisitor);        
        settingVisitorControl.verify();     
        
        settingVisitorControl.reset();
        settingVisitor.visitSettingGroup(m_set);
        settingVisitorControl.andReturn(true);
        settingVisitor.visitSetting(m_settings[0]);
        settingVisitor.visitSettingGroup(m_settings[1]);
        settingVisitorControl.andReturn(true);
        settingVisitor.visitSetting(m_settings[2]);
        settingVisitorControl.replay();
        m_set.acceptVisitor(settingVisitor);        
        settingVisitorControl.verify();     
    }

}
