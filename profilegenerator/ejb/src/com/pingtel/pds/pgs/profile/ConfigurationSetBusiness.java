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

import com.pingtel.pds.common.PDSException;


public interface ConfigurationSetBusiness {

    public Integer getID () throws RemoteException;

    public Integer getRefConfigSetID () throws RemoteException;

    public void setRefConfigSetID ( Integer refConfigSetID )
        throws RemoteException;

    public String getContent () throws RemoteException;

    public void setContent ( String doc ) throws RemoteException;

    public int getProfileType ()  throws RemoteException;

    public String getExternalID () throws RemoteException;
}