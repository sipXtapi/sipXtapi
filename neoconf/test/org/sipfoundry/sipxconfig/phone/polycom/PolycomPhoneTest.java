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

import org.sipfoundry.sipxconfig.common.Organization;

public class PolycomPhoneTest extends TestCase {
        
    public void testGenerateProfiles() throws Exception {
        Organization rootOrg = new Organization();
        rootOrg.setDnsDomain("localhost.localdomain");

        PolycomTestHelper helper = PolycomTestHelper.plainEndpointSeed();
        helper.phone[0].generateProfiles(helper.endpoint[0]);
        
        // content of profiles is tested in individual base classes of ConfigurationTemplate
    }
}


