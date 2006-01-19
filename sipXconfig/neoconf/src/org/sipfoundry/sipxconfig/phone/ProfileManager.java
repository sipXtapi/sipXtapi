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
package org.sipfoundry.sipxconfig.phone;

import java.util.Collection;

public interface ProfileManager {
    /**
     * Generate profile on phones in background
     * 
     * @param phones collection of phone objects
     */
    public void generateProfilesAndRestart(Collection phones);
    
    public void generateProfileAndRestart(Integer phoneId);
}
