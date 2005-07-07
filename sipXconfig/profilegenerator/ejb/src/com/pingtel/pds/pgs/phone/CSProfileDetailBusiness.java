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


public interface CSProfileDetailBusiness {

    public Integer getID() throws RemoteException;

    public Integer getCoreSoftwareDetailsID () throws RemoteException;

    public Integer getProfileType () throws RemoteException;

    public String getFileName () throws RemoteException;

    public void setFileName ( String fileName ) throws RemoteException;

    public String getRenderClassName () throws RemoteException;

    public void setRenderClassName ( String renderClassName )
        throws RemoteException;

    public String getProjectionClassName () throws RemoteException;

    public void setProjectionClassName ( String projectionClassName )
        throws RemoteException;

    public String getXSLTURL() throws RemoteException;

    public void setXSLTURL ( String xsltURL ) throws RemoteException;

    public String getExternalID () throws RemoteException;

}
