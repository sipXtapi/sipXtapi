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
package org.sipfoundry.sipxconfig.bulk.ldap;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class LdapImportManagerImplTest extends TestCase {
    
    private LdapImportManager m_ldapImportManager;

    protected void setUp() throws Exception {
        ApplicationContext ac = TestHelper.getApplicationContext();
        m_ldapImportManager = (LdapImportManager) ac.getBean("ldapImportManager", LdapImportManager.class);
    }

    public void testInsert() {
        assertNotNull(m_ldapImportManager);
        // FIXME: commented out until we mock LDAP
        //m_ldapImportManager.insert();        
    }

}
