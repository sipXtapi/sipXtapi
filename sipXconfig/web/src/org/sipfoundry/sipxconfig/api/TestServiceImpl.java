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

import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class TestServiceImpl implements TestService {    
    private CallGroupContext m_callGroupContext;
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private CoreContext m_coreContext;
    private ParkOrbitContext m_parkOrbitContext;
    private PhoneContext m_phoneContext; 

    public void setCallGroupContext(CallGroupContext callGroupContext) {
        m_callGroupContext = callGroupContext;
    }

    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

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
        // Clear the call group context not only when requested, but also when the
        // core context is going to be cleared.  Otherwise we get database integrity
        // problems, because the user ring objects in a call group reference users.
        if (Boolean.TRUE.equals(resetServices.getCallGroup())
            || Boolean.TRUE.equals(resetServices.getUser())) {
            m_callGroupContext.clear();
        }
        if (Boolean.TRUE.equals(resetServices.getConferenceBridge())) {
            m_conferenceBridgeContext.clear();
        }
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
