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
package org.sipfoundry.sipxconfig.api;

/**
 * Any change to users gets replicated in batches to sipx family 
 * of servers asynchronously.  Changes to phones does not happen
 * automatically so you will need to explicity send profiles to
 * rellevant phones.
 */
public interface UserService {    

    /**
     * Create a new user 
     * @param displayName e.g. jsmith or 160
     * @param pin uncrypted pin value used for voicemail access and web end user portal
     *    e.g. 1234.  alpha-numeric is ok and stronger password
     *    to break, but voicemail authentication from phone dialpad would not be possible 
     */
    public void createUser(String userName, String pin);
    
    /**
     * Delete user and all related information by username
     */
    public void deleteUser(String userName);

    /**
     * Delete user by username
     */
    public void setUserProperty(String userName, String property, Object value);
}
