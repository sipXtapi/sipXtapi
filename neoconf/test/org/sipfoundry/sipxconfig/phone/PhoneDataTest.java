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
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

public class PhoneDataTest extends TestCase {
    
    PhoneData phone = new PhoneData();
    
    public void testSetSerialNumber() {
        phone.setSerialNumber("123456789012");
        assertEquals("123456789012", phone.getSerialNumber());        
        phone.setSerialNumber("1234 5678 9012");
        assertEquals("123456789012", phone.getSerialNumber());        
        phone.setSerialNumber("12:34:56:78:90:12");
        assertEquals("123456789012", phone.getSerialNumber());        
        phone.setSerialNumber("AABBCCDDEEFF");
        assertEquals("aabbccddeeff", phone.getSerialNumber());        
        phone.setSerialNumber("totallybogus");
        assertEquals("totallybogus", phone.getSerialNumber());        
    }

}
