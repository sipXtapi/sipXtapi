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
package org.sipfoundry.sipxconfig.settings;

import junit.framework.TestCase;

import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

public class SettingsDaoImplTestDb extends TestCase {
    
    private SettingsDao m_dao;

    /*
     * @see TestCase#setUp()
     */
    protected void setUp() throws Exception {
        super.setUp();
        
        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bf = bfl.useBeanFactory("unittest-db");
        // now use some bean from factory 
        m_dao = (SettingsDao) bf.getFactory().getBean("settingsDao");
        assertNotNull(m_dao);
    }
    
    public void testSettings() {
        SettingSet root = new SettingSet("root");       
        SettingSet subset = new SettingSet("subset");
        root.addSetting(subset);
        Setting setting = new Setting("subsetting", "value");
        subset.addSetting(setting);
        
        m_dao.storeSetting(root, SettingsDao.CASCADE);
        
        SettingSet rootRead = m_dao.loadSettings(root.getId());
        assertNotNull(rootRead);
        assertEquals(1, rootRead.getSettings().size());
    }
}
