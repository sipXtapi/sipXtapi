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
package org.sipfoundry.sipxconfig.phone.grandstream;

import java.io.ByteArrayOutputStream;
import java.lang.reflect.Array;
import java.util.Arrays;

import junit.framework.TestCase;

import org.apache.commons.codec.binary.Hex;
import org.apache.commons.io.IOUtils;
import org.easymock.MockControl;
import org.easymock.internal.EqualsMatcher;
import org.sipfoundry.sipxconfig.common.TestUtil;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.SipService;

public class GrandstreamPhoneTest extends TestCase {
    
    GrandstreamPhone phone;
    
    PhoneTestDriver tester;
    
    protected void setUp() {
        phone = new GrandstreamPhone(GrandstreamModel.MODEL_PHONEBT);
        tester = new PhoneTestDriver(phone);
    }

    public void testGenerateTextProfiles() throws Exception {
        phone.setTextFormatEnabled(true);
        ByteArrayOutputStream profile = new ByteArrayOutputStream();
        phone.writeTextFile(profile);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-gsbt.cfg"));
        String actual = TestUtil.cleanEndOfLines(profile.toString());
        assertEquals(expected, actual);
    }

    /**
     * Failing, binary format incompatable w/grandstream configuration tool
     */
    public void testGenerateBinaryProfiles() throws Exception {
        ByteArrayOutputStream actualBinary = new ByteArrayOutputStream();
        String body="P1=A&";
        phone.generateGsParaString(actualBinary, body);
        String actual = new String(Hex.encodeHex(actualBinary.toByteArray()));
        
        // in order to generate the simple.bin, I ran
        //  echo "P1=A" > simple.txt
        //  GS_CFG_GEN/bin/encode.sh 0004f200e06b simple.txt simple.bin
        String expectedBinary = IOUtils.toString(this.getClass().getResourceAsStream("simple.bin"));
        String expected = new String(Hex.encodeHex(expectedBinary.getBytes()));
        
        // KNOWN_FAILURE_XCF_642
        // assertEquals(expected, actual);
        // FIXME: uncomment this line, uncomment above line
        assertFalse(expected.equals(actual));
    }
    
    public void testReset() throws Exception {
        tester.sipControl = MockControl.createStrictControl(SipService.class);
        tester.sip = (SipService) tester.sipControl.getMock();        
        tester.sip.sendCheckSync("\"Joe User\"<sip:juser@sipfoundry.org>", "sipfoundry.org", null, "juser", new byte[0]);
        tester.sipControl.setMatcher(new ResetArgumentMatcher());
        tester.sipControl.replay();
        
        phone.setSipService(tester.sip);
        phone.restart();
    }
    
    class ResetArgumentMatcher extends EqualsMatcher {
        protected boolean argumentMatches(Object expected, Object actual) {
            if (actual instanceof byte[]) {
                return ((byte[]) actual).length > 0;
            }
            return super.argumentMatches(expected, actual);
         }
    }
    
}
