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

import junit.framework.TestCase;

import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.providers.dao.SaltSource;
import org.acegisecurity.userdetails.UserDetails;
import org.sipfoundry.sipxconfig.common.User;

public class SaltSourceImplTest extends TestCase {
    // SaltSourceImpl.getSalt should return its input as the salt
    public void testGetSalt() {
        SaltSource ss = new SaltSourceImpl();
        UserDetails user = new UserDetailsImpl(new User(), "userNameOrAlias", new GrantedAuthority[0]);
        assertEquals(user, ss.getSalt(user));
    }
}
