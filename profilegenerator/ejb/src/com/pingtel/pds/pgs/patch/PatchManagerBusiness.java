/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/patch/PatchManagerBusiness.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
package com.pingtel.pds.pgs.patch;


import java.rmi.RemoteException;

import com.pingtel.pds.common.PDSException;

import org.jdom.Element;

public interface PatchManagerBusiness {

    public String upgradeNeeded ()
        throws PDSException, RemoteException;

    public Element getUpgradeInfo ()
        throws PDSException, RemoteException;

    public void processPatches ( Integer targetVersion )
        throws PDSException, RemoteException;

    public Integer getDatabaseVersion ()
        throws RemoteException;

    public String getPatchStatus ( Integer patchNumber )
        throws PDSException, RemoteException;

    public void setPatchStatus ( String status, Integer patchNumber )
        throws PDSException, RemoteException;

    public String getPatchDescription ( Integer patchNumber )
        throws PDSException, RemoteException;

}