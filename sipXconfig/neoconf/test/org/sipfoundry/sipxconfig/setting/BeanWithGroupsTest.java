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

import org.sipfoundry.sipxconfig.TestHelper;

import junit.framework.TestCase;

public class BeanWithGroupsTest extends TestCase {
    private BeanWithGroups m_bean;
    
    protected void setUp() throws Exception {
        m_bean = new BeanWithGroups();
        InputStream in = getClass().getResourceAsStream("birds.xml");
        m_bean.setSettingModel(TestHelper.loadSettings(in));        
    }    

    public void testGetSettingValueOneGroup() {        
        Group g1 = new Group();
        Setting s = m_bean.getSettings().getSetting("towhee/rufous-sided");
        g1.setValue(s, "10");
        m_bean.addGroup(g1);
        
        assertEquals("10", m_bean.getSettingModel2().getSettingValue("towhee/rufous-sided"));
    }

    public void testGetSettingValueMultipleGroup() {
        Group g1 = new Group();
        Setting s = m_bean.getSettings().getSetting("towhee/rufous-sided");
        g1.setValue(s, "10");
        m_bean.addGroup(g1);
        
        Group g2 = new Group();
        g2.setWeight(new Integer(1));
        g1.setValue(s, "20");
        m_bean.addGroup(g2);

        assertEquals("20", m_bean.getSettingModel2().getSettingValue("towhee/rufous-sided"));
    }
}
