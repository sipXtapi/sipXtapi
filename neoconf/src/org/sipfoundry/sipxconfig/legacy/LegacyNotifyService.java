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
package org.sipfoundry.sipxconfig.legacy;

import java.rmi.Remote;
import java.rmi.RemoteException;
import java.util.Map;

/**
 * LegacyNotifyService - used by profilegenerate to notify neoconf about user and system initiated
 * events
 */
public interface LegacyNotifyService extends Remote {

    public static final int BACKUP = 140;
    
    /**
     * Call when user is first setting up system, would not be called on upgrade.
     */
    public void onInit() throws RemoteException;

    /**
     * Call when user explicitly apply patches OR when system is setting up for the first
     * time and there are development patches that need to be applied. 
     */
    public void onApplyPatch(Integer patchId, Map properties) throws RemoteException;

    public void onUserDelete(Integer userId) throws RemoteException;
}
