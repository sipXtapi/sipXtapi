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

import org.sipfoundry.sipxconfig.login.LoginContext;

public class PasswordEncoderImpl implements net.sf.acegisecurity.providers.encoding.PasswordEncoder {
    private LoginContext m_loginContext;

    public boolean isPasswordValid(String encPass, String rawPass, Object salt) {
        // dummy admin user is enabled only when running tests
        if (AuthenticationDaoImpl.isDummyAdminUserEnabled()) {
            return true;
        }
        
        String encodedPassword = encodePassword(rawPass, salt);
        return encodedPassword.equals(encPass);
    }

    public String encodePassword(String rawPass, Object salt) {
        UserDetailsImpl userDetails = (UserDetailsImpl) salt;
        String encodedPassword =
            m_loginContext.getEncodedPassword(userDetails.getCanonicalUserName(), rawPass);
        return encodedPassword;
    }

    public void setLoginContext(LoginContext loginContext) {
        m_loginContext = loginContext;
    }
}
