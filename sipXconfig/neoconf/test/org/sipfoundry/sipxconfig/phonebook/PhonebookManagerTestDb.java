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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class PhonebookManagerTestDb extends TestCase {

    private PhonebookManager m_context;

    private ApplicationContext m_appContext;

    protected void setUp() throws Exception {
        m_appContext = TestHelper.getApplicationContext();
        m_context = (PhonebookManager) m_appContext.getBean(PhonebookManager.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");        
    }
    
    public void testGetPhonebook() throws Exception {
        Phonebook p1 = m_context.getPhonebook();
        assertNull(p1);        
        TestHelper.insertFlat("phonebook/PhonebookSeed.db.xml");
        
        Phonebook p2 = m_context.getPhonebook();
        assertEquals(1, p2.getUserMembers().size());
    }
    
    public void testSavePhonebook() throws Exception {
        Phonebook p = new Phonebook();
        m_context.savePhonebook(p);
        
    }
}
