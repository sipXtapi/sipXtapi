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
package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;

public class SipUriTest extends TestCase {
    private static final String USERNAME = "username";

    public void testFormatUser() {
        User user = new User();
        user.setUserName(USERNAME);
        SipUri uri = new SipUri(user, "mycomp.com");

        assertEquals("sip:" + USERNAME + "@mycomp.com", uri.toString());

        user.setLastName("Last");
        uri = new SipUri(user, "mycomp.com");

        assertEquals("Last<sip:" + USERNAME + "@mycomp.com>", uri.toString());

        user.setFirstName("First");
        uri = new SipUri(user, "mycomp.com");

        assertEquals("First Last<sip:" + USERNAME + "@mycomp.com>", uri.toString());
    }
    
    public void testFormatNameDomainPort() {
        SipUri uri = new SipUri("name", "sipfoundry.org", 33);
        
        assertEquals("sip:name@sipfoundry.org:33", uri.toString());        
    }
    
    public void testFormatNameDomain() {
        SipUri uri = new SipUri("name", "sipfoundry.org", false);
        
        assertEquals("sip:name@sipfoundry.org", uri.toString());        
    }
    
    public void testFormatNameDomainQuote() {
        SipUri uri = new SipUri("name", "sipfoundry.org", true);
        
        assertEquals("<sip:name@sipfoundry.org>", uri.toString());        
    }        
}
