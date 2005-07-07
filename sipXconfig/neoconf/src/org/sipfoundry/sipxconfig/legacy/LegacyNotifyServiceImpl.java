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
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class LegacyNotifyServiceImpl implements LegacyNotifyService {
    private DialPlanContext m_dialPlanContext;
    private ForwardingContext m_forwardingContext;
    private PhoneContext m_phoneContext;

    public void onInit() {
        // reset to factory defaults
        m_dialPlanContext.clear();
    }

    /**
     * Called when user is deleted.
     * 
     * Please note: there is no need for triggering alias generation here since deleting user
     * already would trigger it.
     * 
     */
    public void onUserDelete(Integer userId) {
        // delete call sequence for the user
        m_forwardingContext.removeCallSequenceForUserId(userId, false);

        // delete lines for the user
        m_phoneContext.deleteLinesForUser(userId);
    }

    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }

    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }
}
