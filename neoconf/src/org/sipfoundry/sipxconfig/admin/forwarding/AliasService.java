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
package org.sipfoundry.sipxconfig.admin.forwarding;

import java.rmi.Remote;
import java.rmi.RemoteException;

public interface AliasService extends Remote {
    public static final String TYPE_AUTHEXCEPTION = "authexception";
    public static final String TYPE_ALIAS = "alias";
    
    public String getForwardingAliases() throws RemoteException;
    public String getForwardingAuthExceptions() throws RemoteException;
}
