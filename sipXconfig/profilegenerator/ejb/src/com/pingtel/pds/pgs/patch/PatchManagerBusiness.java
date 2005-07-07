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
package com.pingtel.pds.pgs.patch;

import java.rmi.RemoteException;

import org.jdom.Element;

public interface PatchManagerBusiness {
    public void initialUpgrade() throws RemoteException;

    public Element getUpgradeInfo() throws RemoteException;    

    public void processPatches(Integer targetVersion) throws RemoteException;

    public Integer getDatabaseVersion() throws RemoteException;

    public String getPatchStatus(Integer patchNumber) throws RemoteException;

    public void setPatchStatus(String status, Integer patchNumber) throws RemoteException;

    public String getPatchDescription(Integer patchNumber) throws RemoteException;
}
