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

import java.util.Collection;

import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

import junit.framework.TestCase;

public class AttrMapTest extends TestCase {

    private AttrMap m_map;

    protected void setUp() throws Exception {
        ApplicationContext ac = TestHelper.getApplicationContext();
        m_map = (AttrMap) ac.getBean("attrMap", AttrMap.class);
    }
    
    public void testGetLdapAttributesArray() throws Exception {        
        String[] ldapAttributesArray = m_map.getLdapAttributesArray();        
        for (String attr : ldapAttributesArray) {
            assertNotNull(attr);
        }
    }
    
    public void testGetLdapAttibutes() throws Exception {
        for (String name : m_map.getLdapAttributes()) {
            assertNotNull(name);
        }
    }
    
    public void testIdentityAttribute() {
        Collection<String> ldapAttributes = m_map.getLdapAttributes();
        assertTrue(ldapAttributes.contains(m_map.getIdentityAttributeName()));
    }
}
