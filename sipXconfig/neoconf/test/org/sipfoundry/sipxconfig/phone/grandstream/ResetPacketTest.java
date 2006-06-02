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
package org.sipfoundry.sipxconfig.phone.grandstream;

import junit.framework.TestCase;

import org.apache.commons.codec.binary.Hex;

public class ResetPacketTest extends TestCase {

    public void testGetBytes() {
        byte[] bytes = new ResetPacketWithIdenticalTimeCode("passwd", "000000000000").getResetMessage();
        String actual = new String(Hex.encodeHex(bytes));
        String expected = "0001000000bc614e0ae4c46d9ab8a7b14d3ef6cefb4f9f88";
        assertEquals(expected, actual);
    }
    
    public void testGetTimeCode() throws Exception {
        // admittingly, not much of a test, not sure what could be done here
        int t1 = new ResetPacket("foo", "0000").getTimeCode();
        int t2 = new ResetPacket("foo", "0000").getTimeCode();
        assertTrue(t2 >= t1);
    }
    
    class ResetPacketWithIdenticalTimeCode extends ResetPacket {
        ResetPacketWithIdenticalTimeCode(String password, String mac) {
            super(password, mac);
        }
        protected int getTimeCode() {
            return 12345678;
        }
    }
}
