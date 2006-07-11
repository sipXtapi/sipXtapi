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

import java.io.InputStream;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class BeanWithGroupsTest extends TestCase {
    private BeanWithGroups m_bean;
    
    protected void setUp() throws Exception {
        m_bean = new BirdWithGroups();
    }    
    
    static class BirdWithGroups extends BeanWithGroups {
        protected Setting loadSettings() {
            InputStream in = getClass().getResourceAsStream("birds.xml");            
            return TestHelper.loadSettings(in);
        }

        @Override
        public void initialize() {            
        }
    }

    public void testGetSettingValueOneGroup() {        
        Group g1 = new Group();
        Setting s = m_bean.getSettings().getSetting("towhee/rufous-sided");
        g1.setSettingValue(s, new SettingValueImpl("10"), null);
        m_bean.addGroup(g1);
        
        assertEquals("10", m_bean.getSettingValue("towhee/rufous-sided"));
    }

    public void testGetSettingValueMultipleGroup() {
        Group g1 = new Group();
        Setting s = m_bean.getSettings().getSetting("towhee/rufous-sided");
        g1.setSettingValue(s, new SettingValueImpl("10"), null);
        m_bean.addGroup(g1);
        
        Group g2 = new Group();
        g2.setWeight(new Integer(1));
        g2.setSettingValue(s, new SettingValueImpl("20"), null);
        m_bean.addGroup(g2);

        assertEquals("20", m_bean.getSettingValue("towhee/rufous-sided"));
    }
}
