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

import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;

public class ParkOrbitServiceImpl implements ParkOrbitService {
    
    private ParkOrbitContext m_parkOrbitContext;
    private SimpleBeanBuilder m_parkOrbitBuilder;
    
    public void setParkOrbitContext(ParkOrbitContext parkOrbitContext) {
        m_parkOrbitContext = parkOrbitContext;
    }

    public void setParkOrbitBuilder(SimpleBeanBuilder builder) {
        m_parkOrbitBuilder = builder;
    }

    public GetParkOrbitsResponse getParkOrbits() throws RemoteException {
        GetParkOrbitsResponse response = new GetParkOrbitsResponse();
        Collection orbitsColl = m_parkOrbitContext.getParkOrbits();
        org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit[] orbits =
            (org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit[]) orbitsColl
                .toArray(new org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit[orbitsColl.size()]);
        ParkOrbit[] arrayOfParkOrbits =
            (ParkOrbit[]) ApiBeanUtil.toApiArray(m_parkOrbitBuilder, orbits, ParkOrbit.class);
        response.setParkOrbits(arrayOfParkOrbits);
        return response;
    }

    public void addParkOrbit(AddParkOrbit apo) throws RemoteException {
        org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit myOrbit =
            new org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit();
        ParkOrbit apiOrbit = apo.getParkOrbit();
        ApiBeanUtil.toMyObject(m_parkOrbitBuilder, myOrbit, apiOrbit);
        m_parkOrbitContext.storeParkOrbit(myOrbit);
    }

}
