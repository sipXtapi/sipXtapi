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
package org.sipfoundry.sipxconfig.security;

import net.sf.acegisecurity.UserDetails;
import net.sf.acegisecurity.providers.dao.SaltSource;

public class SaltSourceImpl implements SaltSource {
    
    public Object getSalt(UserDetails userDetails) {
        return userDetails;
    }

}
