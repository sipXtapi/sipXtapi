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

import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class PhonebookManagerTestDb extends TestHelper.TestCaseDb {

    private PhonebookManager m_context;

    private ApplicationContext m_appContext;

    protected void setUp() throws Exception {
        m_appContext = TestHelper.getApplicationContext();
        m_context = (PhonebookManager) m_appContext.getBean(PhonebookManager.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");        
    }
    
    public void testGetGlobalPhonebook() throws Exception {
        Phonebook p1 = m_context.getGlobalPhonebook();
        assertNull(p1);

        TestHelper.insertFlat("phonebook/PhonebookSeed.db.xml");
        
        Phonebook p2 = m_context.getGlobalPhonebook();
        assertNotNull(p2);
        assertEquals(1, p2.getUserMembers().size());
    }
    
    public void testGetPhonebook() throws Exception {
        TestHelper.insertFlat("phonebook/PhonebookSeed.db.xml");        
        Phonebook p2 = m_context.getPhonebook(1001);
        assertEquals(1, p2.getUserMembers().size());
    }
    
    public void testSavePhonebook() throws Exception {
        Phonebook p = new Phonebook();
        m_context.savePhonebook(p);
        
    }
}
