/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.legacy;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;


public class LegacyNotifyServiceImpl implements LegacyNotifyService {
    private DialPlanContext m_dialPlanContext; 
    
    public void onInit() {
        // reset to factory defaults 
        m_dialPlanContext.clear();
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }
}
