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

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.Organization;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

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
}


