/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/user/ExtensionPoolAdvocateHome.java#4 $
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

import javax.ejb.EJBHome;

public interface ExtensionPoolAdvocateHome extends EJBHome {

    ExtensionPoolAdvocate create()
            throws javax.ejb.CreateException, java.rmi.RemoteException;
}
