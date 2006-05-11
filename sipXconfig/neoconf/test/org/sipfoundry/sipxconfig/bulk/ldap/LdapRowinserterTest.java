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


import junit.framework.TestCase;

public class LdapRowinserterTest extends TestCase {

    public void testMapInvariant() throws Exception {
        AttrMap map = new AttrMap();
        assertTrue(map.invariant());        
    }
    
    public void testMapGetLdapAttibutes() throws Exception {
        AttrMap map = new AttrMap();
        Collection<String> ldapAttributes = map.getLdapAttributes();
        for (String name : ldapAttributes) {
            assertNotNull(name);
        }
    }
    
}
