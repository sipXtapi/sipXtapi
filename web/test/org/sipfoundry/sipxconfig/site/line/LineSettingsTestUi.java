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
package org.sipfoundry.sipxconfig.site.line;

import junit.framework.Test;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.phone.PhoneTestUiBase;


public class LineSettingsTestUi extends PhoneTestUiBase {

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(LineSettingsTestUi.class);
    }
    
    public void testEditLine() {
        seedNewPhone();
        clickLink("ManagePhones");        
        clickLinkWithText("000000000000");
        // NOTE: Polycom only setting 
        clickLinkWithText("SIP");        
    }
}
