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

import java.util.Map;

import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class LegacyNotifyServiceImpl implements LegacyNotifyService {

    private static final int ADD_OPERATOR = 101;

    private DialPlanContext m_dialPlanContext;
    private ForwardingContext m_forwardingContext;
    private PhoneContext m_phoneContext;

    public void onInit() {
        // reset to factory defaults
        m_dialPlanContext.clear();
        addOperator();
    }

    public void onApplyPatch(Integer patchId, Map properties_) {
        switch (patchId.intValue()) {
        case ADD_OPERATOR:
            addOperator();
            break;
        default:
            break;
        }
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

    private void addOperator() {
        AutoAttendant operator = AutoAttendant.createOperator();
        m_dialPlanContext.storeAutoAttendant(operator);
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
