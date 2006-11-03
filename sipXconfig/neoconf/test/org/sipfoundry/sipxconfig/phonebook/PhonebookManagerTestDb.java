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
package org.sipfoundry.sipxconfig.phonebook;

import java.util.List;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.context.ApplicationContext;

public class PhonebookManagerTestDb extends TestHelper.TestCaseDb {
    private PhonebookManager m_context;
    private SettingDao m_settingDao;
    private ApplicationContext m_appContext;

    protected void setUp() throws Exception {
        m_appContext = TestHelper.getApplicationContext();
        m_context = (PhonebookManager) m_appContext.getBean(PhonebookManager.CONTEXT_BEAN_NAME);
        m_settingDao = (SettingDao) m_appContext.getBean(SettingDao.CONTEXT_NAME);
        TestHelper.cleanInsert("ClearDb.xml");        
    }
    
    public void testGetGlobalPhonebook() throws Exception {
        Phonebook p1 = m_context.getGlobalPhonebook();
        assertNull(p1);

        TestHelper.insertFlat("phonebook/PhonebookSeed.db.xml");
        
        Phonebook p2 = m_context.getGlobalPhonebook();
        assertNotNull(p2);
        assertEquals(1, p2.getMembers().size());
    }
    
    public void testGetPhonebook() throws Exception {
        TestHelper.insertFlat("phonebook/PhonebookSeed.db.xml");        
        Phonebook p2 = m_context.getPhonebook(1001);
        assertEquals(1, p2.getMembers().size());
    }
    
    public void testSavePhonebook() throws Exception {
        Phonebook p = new Phonebook();
        m_context.savePhonebook(p);
        
    }
    
    public void testUpdatePhonebookWithMemberGroups() throws Exception {
        Phonebook p = new Phonebook();
        List<Group> groups = m_settingDao.getGroupsByString(User.GROUP_RESOURCE_ID,
                "phonebook-users", true);
        p.replaceMembers(groups);
        m_context.savePhonebook(p);
        assertEquals(1, TestHelper.getConnection().getRowCount("phonebook_member"));
    }
}
