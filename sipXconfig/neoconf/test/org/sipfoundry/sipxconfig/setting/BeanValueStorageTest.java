/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.IOException;
import java.io.InputStream;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class BeanValueStorageTest extends TestCase {
    
    BeanValueStorage m_vs;
    Setting m_birds;
    
    protected void setUp() throws IOException {
        BeanValueStorageTestBean bean = new BeanValueStorageTestBean();
        m_vs = new BeanValueStorage(bean);
        InputStream in = getClass().getResourceAsStream("birds.xml");
        m_birds = TestHelper.loadSettings(in);                
    }
    
    public void testGetSettingValue() {
        Setting s = m_birds.getSetting("towhee/canyon");
        assertEquals("14 inches", m_vs.getSettingValue(s).getValue());
    }
    
    public void testGetSettingValuePathArray() {
        Setting s1 = m_birds.getSetting("flycatcher/peewee");
        assertEquals(".25 inches", m_vs.getSettingValue(s1).getValue());
        Setting s2 = m_birds.getSetting("flycatcher/willow");
        assertEquals(".25 inches", m_vs.getSettingValue(s2).getValue());
    }

    static class BeanValueStorageTestBean {
        
        @SettingEntry(path = "towhee/canyon")
        public String getFunkyLittleBird() {
            return "14 inches";
        }       
        
        @SettingEntry(paths = { "flycatcher/peewee", "flycatcher/willow" })
        public String getFlycathers() {
            return ".25 inches";
        }
    }
}
