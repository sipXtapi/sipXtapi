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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.net.URL;
import java.util.Collection;
import java.util.Iterator;

import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;

import junit.framework.TestCase;

public class ExternalAliasesTest extends TestCase {

    public void testGetAliasMappings() {
        ExternalAliases externalAliases = new ExternalAliases();
        assertTrue(externalAliases.getAliasMappings().isEmpty());
    }
    
    public void testGetAliasMappingsBadProperty() {
        // warning log messages are expected
        ExternalAliases externalAliases = new ExternalAliases();
        externalAliases.setAliasAddins("ds dkkk, sd");
        assertTrue(externalAliases.getAliasMappings().isEmpty());
        externalAliases.setAddinsDirectory("/ff/ff");
        assertTrue(externalAliases.getAliasMappings().isEmpty());
        
        // bad file
        URL resource = getClass().getResource("UserGroupSeed.db.xml");
        externalAliases.setAliasAddins(resource.getFile());
        assertTrue(externalAliases.getAliasMappings().isEmpty());        
    }
    
    public void testRealAliases() {
        ExternalAliases externalAliases = new ExternalAliases();
        URL other = getClass().getResource("alias.test.xml");
        externalAliases.setAliasAddins(other.getFile());        
        Collection aliasMappings = externalAliases.getAliasMappings();
        assertEquals(5, aliasMappings.size());
        for (Iterator i = aliasMappings.iterator(); i.hasNext();) {
            AliasMapping alias = (AliasMapping) i.next();
            assertTrue(alias.getIdentity().startsWith("30"));
            assertTrue(alias.getContact().indexOf("example.org") > 0);
        }
    }   
}
