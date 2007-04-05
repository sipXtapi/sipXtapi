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
package org.sipfoundry.sipxconfig.gateway;

import junit.framework.TestCase;

public class GatewayTest extends TestCase {

    public void testGetCallPattern() {
        Gateway gateway = new Gateway();
        assertEquals("123", gateway.getCallPattern("123"));
        gateway.setPrefix("99");
        assertEquals("99123", gateway.getCallPattern("123"));
    }    
}
