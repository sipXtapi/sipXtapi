/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.jbossauth;

import javax.security.auth.login.LoginException;

/**
 * SIPxchangeAuthPlugin defines the SIPxchange authentication plug-in
 * mechanism.  It should be implemented if you wish to use an external
 * security source to authenticate users.
 *
 * Note: this only makes sense for END_USER and ADMIN
 * <b>not SUPERADMIN</b> users.
 */
public interface SIPxchangeAuthPlugin {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////


//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * isValidUser implementations should query the external security
     * source and return true if the given userName and password are
     * valid.
     *
     * @param userName the name of the user whose security credentails
     * you are going to evaluate
     * @param password the plain text password for the user.
     * @return true if the userName and password match a valid user
     * in the external security source, otherwise false.
     * @throws LoginException is thrown for all exceptions.
     */
    boolean isValidUser(String userName, String password)
            throws LoginException;


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////


}
