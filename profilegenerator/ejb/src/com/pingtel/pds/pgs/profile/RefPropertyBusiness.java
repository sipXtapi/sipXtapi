/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/profile/RefPropertyBusiness.java#4 $
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


public interface RefPropertyBusiness {

    public Integer getID () throws RemoteException;

    public String getName () throws RemoteException;

    public String getContent () throws RemoteException;

    public void setContent ( String doc ) throws RemoteException;

    public int getProfileType () throws RemoteException;

    public String getExternalID () throws RemoteException;

    public String getCode() throws RemoteException;
}