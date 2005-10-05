package org.sipfoundry.sipxconfig.phone.snom;

import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class SnomTest extends TestCase {

    private SnomPhone m_phone;

    protected void setUp() {
        m_phone = new SnomPhone(SnomModel.MODEL_360);
        new PhoneTestDriver(m_phone, "snom/phone.xml");
    }

    public void testGenerateProfiles() throws Exception {
        StringWriter profile = new StringWriter();
        m_phone.generateProfile(profile);
        String expected = IOUtils.toString(this.getClass()
                .getResourceAsStream("expected-360.cfg"));
        assertEquals(expected, profile.toString());
    }

    public void testGetProfileName() {
        SnomPhone phone = new SnomPhone(SnomModel.MODEL_360);
        // it can be called without serial number
        assertEquals("SnomModel[snom360].htm", phone.getProfileName());
        phone.setSerialNumber("abc123");
        assertEquals("SnomModel[snom360]-ABC123.htm", phone.getProfileName());
    }
}
