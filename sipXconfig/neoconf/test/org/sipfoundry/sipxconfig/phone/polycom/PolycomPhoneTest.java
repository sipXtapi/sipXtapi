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

import java.io.File;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.RestartException;
import org.sipfoundry.sipxconfig.setting.Setting;

public class PolycomPhoneTest extends TestCase {

    private PolycomPhone m_phone;

    private PhoneTestDriver m_tester;

    protected void setUp() {
        m_phone = new PolycomPhone();
        m_tester = PhoneTestDriver.supplyTestData(m_phone);
    }
    
    public void testVersionArray() {
        assertSame(new PolycomModel().getVersions()[0], PolycomModel.VER_1_6);
    }

    public void testGenerateProfiles() throws Exception {
        m_phone.setVelocityEngine(TestHelper.getVelocityEngine());
        ApplicationConfiguration cfg = new ApplicationConfiguration(m_phone);
        m_phone.generateProfiles();

        // content of profiles is tested in individual base classes of ConfigurationTemplate
        File file = new File(m_phone.getTftpRoot(), cfg.getAppFilename());
        assertTrue(file.exists());

        m_phone.removeProfiles();
        assertFalse(file.exists());
    }

    public void testRestartFailureNoLine() throws Exception {
        m_phone.getLines().clear();
        try {
            m_phone.restart();
            fail();
        } catch (RestartException re) {
            assertTrue(true);
        }
    }

    public void testRestart() throws Exception {
        
        String uri = m_phone.getLine(0).getUri();
        assertEquals("\"Joe User\"<sip:juser@sipfoundry.org>", uri);
        
        m_phone.restart();        
        m_tester.sipControl.verify();
    }

    public void testLineDefaults() throws Exception {
        Setting settings = m_tester.getPrimaryLine().getSettings();
        Setting address = settings.getSetting("reg/server/1/address");
        assertEquals("sipfoundry.org", address.getValue());
    }

    public void testLineDefaultsNoUser() throws Exception {
        Line secondLine = m_phone.createLine();
        m_phone.addLine(secondLine);
        Setting settings = secondLine.getSettings();
        Setting userId = settings.getSetting("reg/auth.userId");
        assertEquals(null, userId.getValue());
    }
    
    public void testFormat() throws Exception {
        InputStream in = getClass().getResourceAsStream("unformatted.xml");
        Reader rdr = new InputStreamReader(in);
        Writer wtr = new StringWriter();
        PolycomPhone.format(rdr, wtr);
        Reader expected = new InputStreamReader(getClass().getResourceAsStream("formatted.xml"));
        Reader actual = new StringReader(wtr.toString());
        IOUtils.contentEquals(expected, actual);
    }
    
    public void testDayOfWeek() {
        assertEquals(2, PolycomPhoneDefaults.dayOfWeek(0));
        assertEquals(1, PolycomPhoneDefaults.dayOfWeek(6));
    }
}
