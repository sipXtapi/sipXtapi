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
package org.sipfoundry.sipxconfig.site.phone;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;

public class ManagePhonesTestUi extends PhoneTestUiBase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ManagePhonesTestUi.class);
    }

    public void testGenerateProfiles() {
        seedNewPhone();

        clickLink("ManagePhones");          
        checkCheckbox("selectedRow");
        clickButton("phone:sendProfiles");
        // present if no exception
        assertLinkPresent("AddPhone");
    }    
}
