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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.CharArrayReader;
import java.io.CharArrayWriter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;

import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

/**
 * Tests file [MAC_ADDRESS.d]/phone.cfg
 */
public class PhoneConfigurationTest extends XMLTestCase {
    
    PolycomPhone phone;
    
    protected void setUp() {
        XMLUnit.setIgnoreWhitespace(true);
        phone = new PolycomPhone(PolycomModel.MODEL_600);
        PhoneTestDriver.supplyTestData(phone);        
    }

    public void testGenerateProfile() throws Exception {
        PhoneConfiguration cfg = new PhoneConfiguration(phone);
        cfg.setVelocityEngine(TestHelper.getVelocityEngine());
        CharArrayWriter out = new CharArrayWriter();
        cfg.generateProfile(phone.getPhoneTemplate(), out);
        
        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-phone.cfg.xml");
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);            
        Reader generatedXml = new CharArrayReader(out.toCharArray());

        // helpful debug
        System.out.println(new String(out.toCharArray()));

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();
        
    }
}
