/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;

public interface SpeedDialManager extends DialingRuleProvider {
    String CONTEXT_BEAN_NAME = "speedDialManager";

    SpeedDial getSpeedDialForUserId(Integer userId, boolean create);

    void saveSpeedDial(SpeedDial speedDial);

    void activateResourceList();
}
