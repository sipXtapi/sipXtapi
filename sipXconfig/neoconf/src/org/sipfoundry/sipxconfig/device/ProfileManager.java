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
package org.sipfoundry.sipxconfig.device;

import java.util.Collection;

public interface ProfileManager {
    /**
     * Generate profile on phones in background
     * 
     * @param deviceIds collection of ids of device (phone or gateway) objects
     */
    public void generateProfilesAndRestart(Collection deviceIds);

    public void generateProfileAndRestart(Integer deviceId);
}
