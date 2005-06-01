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

import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class LegacyNotifyServiceImpl implements LegacyNotifyService {

    private static final int ADD_OPERATOR = 101;

    private static final int CREATE_DEFAULT_DIALPLAN = 120;

    private DialPlanContext m_dialPlanContext;
    private ForwardingContext m_forwardingContext;
    private PhoneContext m_phoneContext;
    private CallGroupContext m_callGroupContext;

    private Set m_patches = new HashSet();

    /** brand new system */
    public void onInit() {
        applyPatchIfNotAlreadyApplied(ADD_OPERATOR, null);
        applyPatchIfNotAlreadyApplied(CREATE_DEFAULT_DIALPLAN, null);
    }

    public void onApplyPatch(Integer patchId, Map properties_) {
        switch (patchId.intValue()) {
        case ADD_OPERATOR:
            AutoAttendant operator = AutoAttendant.createOperator();
            m_dialPlanContext.storeAutoAttendant(operator);
            break;
        case CREATE_DEFAULT_DIALPLAN:
            // 2.8 users may have already created dialplans so reseting
            // dial plans shouldn't be done blindly
            if (m_dialPlanContext.isDialPlanEmpty()) {
                // Create the initial dialing rules to match mappingrules.xml, et. al
                m_dialPlanContext.resetToFactoryDefault();
            }
            break;
        default:
            break;
        }
        m_patches.add(patchId);
    }

    private void applyPatchIfNotAlreadyApplied(int patchIdInt, Map properties) {
        Integer patchId = new Integer(patchIdInt);
        if (!m_patches.contains(patchId)) {
            onApplyPatch(patchId, properties);
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

        // delete form call groups
        m_callGroupContext.removeUser(userId);
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

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }
}
