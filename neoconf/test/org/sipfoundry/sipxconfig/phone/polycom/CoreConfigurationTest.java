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


public class CoreConfigurationTest extends XMLTestCase {
    
    PolycomPhone phone;
    
    PhoneTestDriver tester;
    
    protected void setUp() {
        XMLUnit.setIgnoreWhitespace(true);
        phone = new PolycomPhone(PolycomModel.MODEL_600);
        tester = new PhoneTestDriver(phone, "polycom/phone.xml");        
    }

    public void testGenerateProfile() throws Exception {
        // sampling of settings 
        phone.getSettings().getSetting("call/rejectBusyOnDnd").setValue("0");
        
        CoreConfiguration cfg = new CoreConfiguration(phone);
        cfg.setVelocityEngine(TestHelper.getVelocityEngine());        
        CharArrayWriter out = new CharArrayWriter();
        cfg.generateProfile(phone.getCoreTemplate(), out);       
        
        InputStream expectedPhoneStream = getClass().getResourceAsStream("expected-ipmid.cfg");
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);            
        Reader generatedXml = new CharArrayReader(out.toCharArray());

        // helpful debug
        System.out.println(new String(out.toCharArray()));

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();        
    }
}
