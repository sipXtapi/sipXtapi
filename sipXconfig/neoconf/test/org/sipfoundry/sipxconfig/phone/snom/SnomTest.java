package org.sipfoundry.sipxconfig.phone.snom;

import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class SnomTest extends TestCase {

    private Phone m_phone;

    protected void setUp() {
        m_phone = new SnomPhone(SnomModel.MODEL_360);
        PhoneTestDriver.supplyTestData(m_phone);
    }

    public void testGenerateProfiles() throws Exception {
        StringWriter profile = new StringWriter();
        m_phone.generateProfile(profile);
        String expected = IOUtils.toString(this.getClass()
                .getResourceAsStream("expected-360.cfg"));
        assertEquals(expected, profile.toString());
    }

    public void testGetProfileName() {
        Phone phone = new SnomPhone(SnomModel.MODEL_360);
        phone.setWebDirectory("web");
        // it can be called without serial number
        assertEquals("web/snom360.htm", phone.getPhoneFilename());
        phone.setSerialNumber("abc123");
        assertEquals("web/snom360-ABC123.htm", phone.getPhoneFilename());
    }
}
