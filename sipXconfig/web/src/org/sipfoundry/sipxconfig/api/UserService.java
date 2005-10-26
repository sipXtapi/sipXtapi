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
    
    public void nop();
}
