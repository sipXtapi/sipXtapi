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
 * 
 * @author dhubler
 *
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
    
}
