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
package org.sipfoundry.sipxconfig.admin.commserver;

import org.sipfoundry.sipxconfig.setting.Setting;

public interface Server {
    Setting getSettings();
    
    String getPresenceServerUri();
    
    /**
     * Saves settings to permanent storage (config files)
     */
    void applySettings();
    
    /**
     * Clears unapplied values, does not resets everything to default.
     */
    void resetSettings();
}
