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
package org.sipfoundry.sipxconfig.common;

import java.util.List;

import org.sipfoundry.sipxconfig.phone.Organization;
import org.sipfoundry.sipxconfig.phone.User;

/**
 * CoreContext
 */
public interface CoreContext {
    public static final String CONTEXT_BEAN_NAME = "coreContext";

    public Organization loadRootOrganization();
    
    public void saveUser(User user);
    
    public void deleteUser(User user);

    public User loadUser(int id);
    
    public List loadUserByTemplateUser(User template);

    public User loadUserByDisplayId(String displayId);        
}
