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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public class PhoneGroups extends BasePage {
    
    public static final String PAGE = "PhoneGroups";
    
    public void editPhoneDefaults(IRequestCycle cycle) {
        EditPhoneDefaults page = (EditPhoneDefaults) cycle.getPage(EditPhoneDefaults.PAGE);
        String factoryId = (String) TapestryUtils.assertParameter(String.class, cycle.getServiceParameters(), 0);
        page.editPhoneSettings(factoryId);
        cycle.activate(page);
    }    
}
