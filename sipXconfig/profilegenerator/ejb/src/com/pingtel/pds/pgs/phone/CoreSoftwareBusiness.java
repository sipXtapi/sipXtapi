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

package com.pingtel.pds.pgs.phone;

import java.rmi.RemoteException;

public interface CoreSoftwareBusiness {

    public Integer getID() throws RemoteException;

    public java.sql.Date getInstalledDate() throws RemoteException;

    public String getDescription () throws RemoteException;

    public String getVersion () throws RemoteException;

    public String getName () throws RemoteException;

    public String getExternalID () throws RemoteException;

}