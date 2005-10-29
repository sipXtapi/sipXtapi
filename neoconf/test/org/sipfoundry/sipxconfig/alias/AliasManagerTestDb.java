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
package org.sipfoundry.sipxconfig.alias;

import java.util.Collection;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class AliasManagerTestDb extends SipxDatabaseTestCase {
    private AliasManager m_aliasManager;
    
    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_aliasManager = (AliasManager) app.getBean(AliasManager.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    // There are four AliasOwners (excluding AliasManager itself): CallGroupManagerImpl,
    // ConferenceBridgeManagerImpl, CoreManagerImpl, and DialPlanManagerImpl.
    public void testGetAliasOwners() {
        Collection aliasOwners = m_aliasManager.getAliasOwners();
        assertTrue(aliasOwners.size() >= 4);    // allow for more AliasOwners to be added
    }
    
    // See CoreManagerImplTestDb.testIsAliasInUse
    public void testIsUserAliasInUse() throws Exception {
        TestHelper.cleanInsertFlat("common/SampleUsersSeed.xml");
        assertTrue(m_aliasManager.isAliasInUse("janus"));       // a user ID
        assertTrue(m_aliasManager.isAliasInUse("dweezil"));     // a user alias
        assertFalse(m_aliasManager.isAliasInUse("jessica"));    // a first name        
    }
    
    // See DialPlanManagerTestDb,DialPlanManagerTestDb
    public void testIsAutoAttendantAliasInUse() throws Exception {
        TestHelper.cleanInsert("admin/dialplan/seedDialPlanWithAttendant.xml");
        assertTrue(m_aliasManager.isAliasInUse("1234"));     // auto attendant extension
        assertTrue(m_aliasManager.isAliasInUse("alias1"));   // auto attendant alias
        assertTrue(m_aliasManager.isAliasInUse("alias2"));   // auto attendant alias
        assertTrue(m_aliasManager.isAliasInUse("100"));      // voicemail extension
        assertFalse(m_aliasManager.isAliasInUse("200"));     // a random extension that should not be in use
    }
    
}
