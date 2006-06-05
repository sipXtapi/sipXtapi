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

import java.util.Map;
import java.util.TreeMap;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;

public class SipUriTest extends TestCase {
    private static final String USERNAME = "username";
    
    public void testFormatFullUri() {
        String uri = SipUri.formatIgnoreDefaultPort("First Last", USERNAME, "example.com", 5060);
        assertEquals("\"First Last\"<sip:" + USERNAME + "@example.com>", uri);

        String uri2 = SipUri.formatIgnoreDefaultPort("First Last", USERNAME, "example.com", 5070);
        assertEquals("\"First Last\"<sip:" + USERNAME + "@example.com:5070>", uri2);
    }
    
    public void testParsePort() {
        assertEquals(5060, SipUri.parsePort("5060", 5070));
        assertEquals(5070, SipUri.parsePort("5070", 5060));
        assertEquals(5080, SipUri.parsePort(StringUtils.EMPTY, 5080));
        assertEquals(5080, SipUri.parsePort(null, 5080));
    }

    public void testFormatUser() {
        User user = new User();
        user.setUserName(USERNAME);
        SipUri uri = new SipUri(user, "mycomp.com");

        assertEquals("sip:" + USERNAME + "@mycomp.com", uri.toString());

        user.setLastName("Last");
        uri = new SipUri(user, "mycomp.com");

        assertEquals("\"Last\"<sip:" + USERNAME + "@mycomp.com>", uri.toString());

        user.setFirstName("First");
        uri = new SipUri(user, "mycomp.com");

        assertEquals("\"First Last\"<sip:" + USERNAME + "@mycomp.com>", uri.toString());
    }

    public void testFormatNameDomainPort() {
        SipUri uri = new SipUri("name", "sipfoundry.org", 33);

        assertEquals("sip:name@sipfoundry.org:33", uri.toString());
    }

    public void testFormatDomainPort() {
        SipUri uri = new SipUri("sipfoundry.org", 34);

        assertEquals("sip:sipfoundry.org:34", uri.toString());
    }

    public void testFormatNameDomain() {
        SipUri uri = new SipUri("name", "sipfoundry.org", false);

        assertEquals("sip:name@sipfoundry.org", uri.toString());
    }

    public void testFormatNameDomainQuote() {
        SipUri uri = new SipUri("name", "sipfoundry.org", true);

        assertEquals("<sip:name@sipfoundry.org>", uri.toString());
    }

    public void testExtractUser() {
        assertNull(SipUri.extractUser("name"));
        assertEquals("name", SipUri.extractUser("sip:name@sipfoundry.org"));
        assertEquals("name", SipUri.extractUser("<sip:name@sipfoundry.org>"));
        assertEquals("name", SipUri.extractUser("   <sip:name@sipfoundry.org>"));
        assertEquals("name", SipUri.extractUser("name@sipfoundry.org"));
        assertEquals("name", SipUri.extractUser("name@sipfoundry@.org"));
    }
    
    public void testExtractUserFromFullUser() {
        assertNull(SipUri.extractUser("name"));
        assertEquals("name", SipUri.extractUser("\"Joe \"<sip:name@sipfoundry.org>"));
        assertEquals("name", SipUri.extractUser("\"Joe Macy\"<sip:name@sipfoundry.org>"));
    }

    public void testExtractFullUser() {
        assertNull(SipUri.extractFullUser("name"));
        assertEquals("name", SipUri.extractFullUser("sip:name@sipfoundry.org"));
        assertEquals("name", SipUri.extractFullUser("<sip:name@sipfoundry.org>"));
        assertEquals("name", SipUri.extractFullUser("   <sip:name@sipfoundry.org>"));
        assertEquals("name", SipUri.extractFullUser("name@sipfoundry.org"));
        assertEquals("name", SipUri.extractFullUser("name@sipfoundry@.org"));
        
        assertEquals("first last - name", SipUri.extractFullUser("\"first last\"<sip:name@sipfoundry.org>"));        
    }
    

    public void testUrlParams() {
        Map urlParams = new TreeMap();
        urlParams.put("a-key", "aa");
        urlParams.put("b-key", null);
        urlParams.put("c-key", "cc");
        assertEquals("<sip:name@domain.com;a-key=aa;b-key;c-key=cc>", SipUri.format("name",
                "domain.com", urlParams));
    }
}
