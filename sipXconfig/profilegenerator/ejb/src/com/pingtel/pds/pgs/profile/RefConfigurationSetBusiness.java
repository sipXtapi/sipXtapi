/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
package com.pingtel.pds.pgs.profile;

import java.rmi.RemoteException;

public interface RefConfigurationSetBusiness {

    public Integer getID () throws RemoteException;

    public String getName() throws RemoteException;

    public Integer getOrganizationID () throws RemoteException;

    public String getExternalID () throws RemoteException;
}