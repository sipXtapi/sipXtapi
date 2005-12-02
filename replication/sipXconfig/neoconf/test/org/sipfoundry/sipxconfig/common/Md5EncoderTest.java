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

public class Md5EncoderTest extends TestCase {

    /*
     * Class under test for String digestPassword(String, String, String)
     */
    public void testDigestPassword() {
        String actual = Md5Encoder.digestPassword("first_last", "auth.mycomp.com", "password");
        assertEquals("8f44d1d713b8aa4e56c6fd78e8ef1b1a", actual);
    }
}
