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

import org.sipfoundry.sipxconfig.common.User;

import net.sf.acegisecurity.GrantedAuthority;
import net.sf.acegisecurity.UserDetails;
import net.sf.acegisecurity.providers.dao.SaltSource;
import junit.framework.TestCase;

public class SaltSourceImplTest extends TestCase {
    // SaltSourceImpl.getSalt should return its input as the salt
    public void testGetSalt() {
        SaltSource ss = new SaltSourceImpl();
        UserDetails user = new UserDetailsImpl(new User(), "userNameOrAlias", new GrantedAuthority[0]);
        assertEquals(user, ss.getSalt(user));
    }
}
