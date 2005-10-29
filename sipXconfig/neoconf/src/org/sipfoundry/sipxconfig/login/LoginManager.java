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
package org.sipfoundry.sipxconfig.login;

import org.sipfoundry.sipxconfig.common.User;

public interface LoginManager {
    
    public User checkCredentials(String userName, String password);
    
    public boolean isAdmin(Integer userId);
    public boolean isAdmin(User user);
}
