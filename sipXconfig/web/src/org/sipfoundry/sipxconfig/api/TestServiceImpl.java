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

import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class TestServiceImpl implements TestService {    
    private CoreContext m_coreContext;
    private ParkOrbitContext m_parkOrbitContext;
    private PhoneContext m_phoneContext; 

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setParkOrbitContext(ParkOrbitContext parkOrbitContext) {
        m_parkOrbitContext = parkOrbitContext;
    }
    
    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void resetServices(ResetServices resetServices) throws RemoteException {
        if (Boolean.TRUE.equals(resetServices.getParkOrbit())) {
            m_parkOrbitContext.clear();
        }
        if (Boolean.TRUE.equals(resetServices.getPhone())) {
            m_phoneContext.clear();
        }
        if (Boolean.TRUE.equals(resetServices.getUser())) {
            m_coreContext.clear();            
        }        
    }
}
