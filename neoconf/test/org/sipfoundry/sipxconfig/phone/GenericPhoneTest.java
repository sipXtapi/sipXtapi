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
package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.settings.NetworkSettings;
import org.sipfoundry.sipxconfig.settings.PhoneSettings;

/**
 * Comments
 */
public class GenericPhoneTest extends TestCase {
    
    public void testFactoryCreation() {
        PhoneTestHelper helper = PhoneTestHelper.createHelper();
        PhoneContext phoneContext = helper.getPhoneContext();
        Endpoint endpoint = new Endpoint();
        endpoint.setPhoneId(GenericPhone.GENERIC_PHONE_ID);
        Phone phone = phoneContext.getPhone(endpoint);
        assertNotNull(phone);
    }
    
    public void testSettings() {
        GenericPhone phone = new GenericPhone();
        PhoneSettings settings = (PhoneSettings) phone.getSettings(new SettingSet());
        NetworkSettings network = settings.getNetworkSettings();
        network.defaultSetting("proxy", "localhost");
    }
}
