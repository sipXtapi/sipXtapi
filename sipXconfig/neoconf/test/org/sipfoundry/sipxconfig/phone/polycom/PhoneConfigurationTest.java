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
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

/**
 * Tests file [MAC_ADDRESS.d]/phone.cfg
 */
public class PhoneConfigurationTest extends XMLTestCase {
    
    private PolycomPhone phone;
    private ProfileGenerator m_pg;
    
    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);
        phone = new PolycomPhone();
        phone.getModel().setMaxLineCount(6);
        PhoneTestDriver.supplyTestData(phone);
        
        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;        
    }

    public void testGenerateProfileVersion16() throws Exception {
        phone.setDeviceVersion(PolycomModel.VER_1_6);
        assertExpectedProfile("expected-phone.cfg.xml");
    }
    
    public void testGenerateProfileVersion20() throws Exception {
        assertExpectedProfile("expected-phone-2.0.cfg.xml");
    }
    
    private void assertExpectedProfile(String expected) throws Exception {
        PhoneConfiguration cfg = new PhoneConfiguration(phone);
        CharArrayWriter out = new CharArrayWriter();
        m_pg.generate(cfg, phone.getPhoneTemplate(), out);

        // helpful debug
        //System.out.println(new String(out.toCharArray()));
        
        InputStream expectedPhoneStream = getClass().getResourceAsStream(expected);
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);            
        Reader generatedXml = new CharArrayReader(out.toCharArray());

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();        
    }
}
