package org.sipfoundry.sipxconfig.phone.snom;

import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class SnomTest extends TestCase {

	SnomPhone phone;

	SnomLine line;

	PhoneTestDriver tester;

	protected void setUp() {
		phone = new SnomPhone();
		line = new SnomLine();
		tester = new PhoneTestDriver(phone, SnomModel.MODEL_360.getModelId(), line, SnomLine.FACTORY_ID);
	}

	public void testGenerateProfiles() throws Exception {
		StringWriter profile = new StringWriter();
		phone.generateProfile(profile);
		String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-360.cfg"));
		assertEquals(expected, profile.toString());
	}
}
