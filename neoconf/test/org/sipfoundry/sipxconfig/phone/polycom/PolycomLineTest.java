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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.phone.LineData;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.setting.Setting;


public class PolycomLineTest extends TestCase {
    
    PolycomPhone phone;
    
    PolycomLine line;

    PhoneTestDriver tester;
    
    protected void setUp() {
        phone = new PolycomPhone();
        line = new PolycomLine();
        tester = new PhoneTestDriver(phone, PolycomModel.MODEL_600.getName(), line, 
                PolycomLine.FACTORY_ID);
        phone.setDefaults(tester.defaults);
        line.setDefaults(tester.defaults);
    }

    public void testDefaults() throws Exception {
        Setting settings = line.getSettings();
        Setting address = settings.getSetting("reg/server/1/address");
        assertEquals("registrar.sipfoundry.org", address.getValue());
    }

    public void testDefaultsNoUser() throws Exception {
        PolycomLine secondLine = (PolycomLine) phone.createLine(new LineData());
        secondLine.setDefaults(tester.defaults);
        phone.addLine(secondLine);
        Setting settings = secondLine.getSettings();        
        Setting userId = settings.getSetting("reg/auth.userId");
        assertEquals(Setting.NULL_VALUE, userId.getValue());
    }
}
