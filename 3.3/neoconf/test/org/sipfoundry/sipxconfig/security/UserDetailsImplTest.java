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
import org.acegisecurity.GrantedAuthorityImpl;
import org.acegisecurity.userdetails.UserDetails;
import org.sipfoundry.sipxconfig.common.User;

public class UserDetailsImplTest extends TestCase {
    public void testUserDetailsImpl() {
        final User user = new User();
        final String userName = "angelina";
        user.setUserName(userName);
        final String pintoken = "lara";
        user.setPintoken(pintoken);
        GrantedAuthority[] authorities = new GrantedAuthority[1];
        GrantedAuthority party = new GrantedAuthorityImpl("party");
        authorities[0] = party;
        UserDetails details = new UserDetailsImpl(user, userName, authorities);
        
        assertTrue(details.isAccountNonExpired());
        assertTrue(details.isAccountNonLocked());
        assertTrue(details.isCredentialsNonExpired());
        assertTrue(details.isEnabled());
        GrantedAuthority[] actualAuthorities = details.getAuthorities();
        assertEquals(1, actualAuthorities.length);
        assertEquals(party, actualAuthorities[0]);
        assertEquals(userName, details.getUsername());
        assertEquals(pintoken, details.getPassword());
    }
}
