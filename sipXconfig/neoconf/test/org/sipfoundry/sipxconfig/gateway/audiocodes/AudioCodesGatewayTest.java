/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import junit.framework.TestCase;

public class AudioCodesGatewayTest extends TestCase {

    public void testGetProfileFilename() {
        AudioCodesGateway gateway = new AudioCodesGateway() {            
        };
        
        gateway.setSerialNumber("00908f03f59B");
        assertEquals("00908F03F59B.ini", gateway.getProfileFilename());

        gateway.setSerialNumber(null);
        assertNull(gateway.getProfileFilename());        
    }

}
