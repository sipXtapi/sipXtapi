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

import java.rmi.RemoteException;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class TestServiceImpl implements TestService {    
    private CoreContext m_coreContext;
    private PhoneContext m_phoneContext;    

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void resetService(ResetService resetService) throws RemoteException {
        if (Boolean.TRUE.equals(resetService.getPhone())) {
            m_phoneContext.clear();
        }
        if (Boolean.TRUE.equals(resetService.getUser())) {
            m_coreContext.clear();            
        }        
    }
}
