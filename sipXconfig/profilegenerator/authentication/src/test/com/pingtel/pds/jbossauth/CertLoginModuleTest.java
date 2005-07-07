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
package com.pingtel.pds.jbossauth;

import java.io.IOException;
import java.io.StringReader;
import java.util.List;

import junit.framework.TestCase;

public class CertLoginModuleTest extends TestCase {
   
    public void testReadSingleTrustedClient() throws IOException {
        String clients = "single entry";
        StringReader rdr = new StringReader(clients);
        List actualClients = CertLoginModule.readTrustedClients(rdr);
        assertEquals(1, actualClients.size());
        assertEquals(clients, actualClients.get(0));
    }

    public void testReadVariousLinefeedsTrustedClient() throws IOException {
        String clients = "first\n\nsecond\nthird\rforth\r\nfifth";
        StringReader rdr = new StringReader(clients);
        List actualClients = CertLoginModule.readTrustedClients(rdr);
        assertEquals(5, actualClients.size());
        assertEquals("first", actualClients.get(0));
        assertEquals("second", actualClients.get(1));
        assertEquals("third", actualClients.get(2));
        assertEquals("forth", actualClients.get(3));
        assertEquals("fifth", actualClients.get(4));
    }
}
