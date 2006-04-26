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
package org.sipfoundry.sipxconfig.presence;

import org.sipfoundry.sipxconfig.common.User;

public interface PresenceServer {
    
    public void signIn(User user);
    
    public void signOut(User user);

    public PresenceStatus getStatus(User user);
}
