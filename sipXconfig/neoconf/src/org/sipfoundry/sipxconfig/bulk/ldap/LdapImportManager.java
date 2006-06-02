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
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.User;

public interface LdapImportManager {
    void insert();
    
    /**
     * Retriece an example of the user and its groups from currently configured LDAP
     * @param user object to be filled with imported data
     * @param groupNames collection of group names created for this user
     */
    void getExample(User user, Collection<String> groupNames);
}
