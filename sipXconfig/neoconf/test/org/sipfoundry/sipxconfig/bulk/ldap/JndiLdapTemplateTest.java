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

import javax.naming.Context;

import junit.framework.TestCase;

public class JndiLdapTemplateTest extends TestCase {

    public void testSetSecurityAuthentication() {
        JndiLdapTemplate template = new JndiLdapTemplate();
        template.setSecurityAuthentication("none");
        assertEquals("none", template.getEnvironment().getProperty(Context.SECURITY_AUTHENTICATION));
        template.setSecurityAuthentication(null);
        assertFalse(template.getEnvironment().containsKey(Context.SECURITY_AUTHENTICATION));
    }
}
