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
package org.sipfoundry.sipxconfig.vendor;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneTestHelper;
import org.sipfoundry.sipxconfig.phone.SettingSet;
import org.sipfoundry.sipxconfig.settings.NetworkSettings;
import org.sipfoundry.sipxconfig.settings.PhoneSettings;

public class PolycomPhoneTest extends TestCase {
        
    public void testFactoryCreation() {
        PhoneTestHelper helper = PhoneTestHelper.createHelper();
        PhoneContext phoneContext = helper.getPhoneContext();
        Endpoint endpoint = new Endpoint();
        endpoint.setPhoneId(PolycomPhone.MODEL_300);
        Phone phone = phoneContext.getPhone(endpoint);
        assertNotNull(phone);
    }
    
    public void testSettings() {
        PolycomPhone phone = new PolycomPhone();
        PhoneSettings settings = (PhoneSettings) phone.getSettings(new SettingSet());
        NetworkSettings network = settings.getNetworkSettings();
        network.getSetting("tftpServer").setValue("hey there");
    }
}
