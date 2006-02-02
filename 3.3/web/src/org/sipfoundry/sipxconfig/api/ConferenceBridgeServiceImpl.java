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
import java.util.Collection;

import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;

public class ConferenceBridgeServiceImpl implements ConferenceBridgeService {
    
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private SimpleBeanBuilder m_conferenceBridgeBuilder;
    
    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    public void setConferenceBridgeBuilder(SimpleBeanBuilder builder) {
        m_conferenceBridgeBuilder = builder;
    }

    public GetConferenceBridgesResponse getConferenceBridges() throws RemoteException {
        GetConferenceBridgesResponse response = new GetConferenceBridgesResponse();
        Collection bridgesColl = m_conferenceBridgeContext.getBridges();
        org.sipfoundry.sipxconfig.conference.Bridge[] bridges =
            (org.sipfoundry.sipxconfig.conference.Bridge[]) bridgesColl
                .toArray(new org.sipfoundry.sipxconfig.conference.Bridge[bridgesColl.size()]);
        ConferenceBridge[] arrayOfConferenceBridges =
            (ConferenceBridge[]) ApiBeanUtil.toApiArray(m_conferenceBridgeBuilder, bridges, ConferenceBridge.class);
        response.setConferenceBridges(arrayOfConferenceBridges);
        return response;
    }

    public void addConferenceBridge(AddConferenceBridge acb) throws RemoteException {
        org.sipfoundry.sipxconfig.conference.Bridge myBridge =
            new org.sipfoundry.sipxconfig.conference.Bridge();
        ConferenceBridge apiBridge = acb.getConferenceBridge();
        ApiBeanUtil.toMyObject(m_conferenceBridgeBuilder, myBridge, apiBridge);
        m_conferenceBridgeContext.store(myBridge);
    }

}
