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


package com.pingtel.pds.pgs.user;


import java.rmi.RemoteException;

import javax.ejb.EJBObject;

import com.pingtel.pds.common.PDSException;


public interface UserGroupBusiness {

    public static final int MAX_NAME_LEN = 60;

    public Integer getID () throws RemoteException;

    public Integer getOrganizationID() throws RemoteException;

    public String getName () throws RemoteException;

    public void setName ( String name ) throws RemoteException, PDSException;

    public Integer getParentID() throws RemoteException;

    public void setParentID ( Integer parentID )
        throws RemoteException;

    public Integer getRefConfigSetID () throws RemoteException;

    public void setRefConfigSetID ( Integer refConfigSetID )
        throws RemoteException;

    public String getExternalID () throws PDSException, RemoteException;

}
