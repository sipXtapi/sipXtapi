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

import java.util.Iterator;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Organization;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingGroup;

public class PolycomPhoneTest extends TestCase {
        
    public void testGenerateProfiles() throws Exception {
        MockControl phoneControl = MockControl.createStrictControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneControl.getMock();
        Organization rootOrg = new Organization();
        rootOrg.setDnsDomain("localhost.localdomain");
        phoneControl.replay();

        PolycomTestHelper helper = PolycomTestHelper.plainEndpointSeed();
        helper.phone[0].generateProfiles(phoneContext, helper.endpoint[0]);
        phoneControl.verify();
        
        // content of profiles is tested in individual base classes of ConfigurationTemplate
    }
    
    public void _testSettingModel() {
        PolycomPhone phone = new PolycomPhone();
        String sysDir = TestHelper.getSysDirProperties().getProperty("sysdir.etc");
        assertNotNull(sysDir);
        phone.setSystemDirectory(sysDir);
        SettingGroup model = phone.getSettingModel(new Endpoint());
        assertNotNull(model);
        
        SettingGroup lines = (SettingGroup) model.getSetting(Phone.LINE_SETTINGS);
        assertNotNull(lines);
        
        Iterator headings = model.getValues().iterator();
        assertEquals("Registration", nextModel(headings).getLabel());
        assertEquals("Lines", nextModel(headings).getLabel());
        assertFalse(headings.hasNext());
    }
    
    private SettingGroup nextModel(Iterator i) {
        return (SettingGroup) i.next();
    }
}


