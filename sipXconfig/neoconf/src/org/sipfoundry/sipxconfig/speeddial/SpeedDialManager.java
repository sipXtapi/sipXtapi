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

public interface SpeedDialManager {
    String CONTEXT_BEAN_NAME = "speedDialManager";

    SpeedDial getSpeedDialForUserId(Integer userId);

    void saveSpeedDial(SpeedDial speedDial);
}
