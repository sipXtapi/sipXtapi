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

import java.util.Collection;
import java.util.Iterator;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingGroup;


public class ProfileModelTest extends TestCase {

    public void testModel() {
        Endpoint endpoint = new Endpoint();
        endpoint.setSerialNumber("123abc");
        endpoint.setPhoneId(Polycom.MODEL_600.getModelId());
        PolycomPhone phone = new PolycomPhone();
        phone.setModelId(Polycom.MODEL_600.getModelId());
        phone.setEndpoint(endpoint);
        phone.setSystemDirectory(TestHelper.getSysDirProperties().getProperty("sysdir.etc"));
        SettingGroup reg = endpoint.getSettings(phone);
        reg.getSetting(PolycomPhone.REGISTRATION_SETTINGS).getSetting("displayName").setValue("homer");
        
        ProfileModel model = new ProfileModel(phone, endpoint);
        Collection registrations = model.getRegistrations();
        assertEquals(Polycom.MODEL_600.getMaxLines(), registrations.size());
        
        Iterator i = registrations.iterator();
        Collection settingsCollection = (Collection) i.next();
        Setting[] settings = (Setting[]) settingsCollection.toArray(new Setting[0]);

        assertEquals("displayName", settings[0].getProfileName());
        assertEquals("homer", settings[0].getProfileValue());

        assertEquals("address", settings[1].getProfileName());
        assertEquals(null, settings[1].getProfileValue());

        assertEquals("label", settings[2].getProfileName());
        assertEquals(null, settings[2].getProfileValue());

        assertEquals("type", settings[3].getProfileName());
        assertEquals("private", settings[3].getProfileValue());
    }
}
