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
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

public class PhoneDefaultsTest extends TestCase {
    
    public void testGetProfileRotUrl() {
        PhoneDefaults defaults = new PhoneDefaults();
        defaults.setDomainName("sipfoundry.org");
        String actual = defaults.getProfileRootUrl();
        assertEquals("http://sipfoundry.org:8090/phone/profile/docroot", actual);
        
        String expected = "http://blah/profile";
        defaults.setProfileRootUrl(expected);
        assertEquals(expected, defaults.getProfileRootUrl());
    }
}
