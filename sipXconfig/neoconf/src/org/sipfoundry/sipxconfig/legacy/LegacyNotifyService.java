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

/**
 * LegacyNotifyService - used by profilegenerate to notify neoconf about user and system initiated events
 */
public interface LegacyNotifyService extends Remote {
    public void onInit() throws RemoteException;
}
