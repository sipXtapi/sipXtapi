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

public class LegacyNotifyServiceImpl implements LegacyNotifyService {
    
    private static final int ADD_OPERATOR = 101;

    private DialPlanContext m_dialPlanContext;

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
    
    public void setDialPlanContext(DialPlanContext dialPlanContext) {
        m_dialPlanContext = dialPlanContext;
    }    
    
    private void addOperator() {
        AutoAttendant operator = AutoAttendant.createOperator();
        m_dialPlanContext.storeAutoAttendant(operator);
    }
}
