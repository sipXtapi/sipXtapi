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

public interface RestartManager {

    /**
     * Restart phones in background
     * 
     * @param phoneIds collection of phone ids to be restarted
     */
    public void restart(Collection phoneIds);
    
    public void restart(Integer phoneId);
}
