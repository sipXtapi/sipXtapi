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

package com.pingtel.pds.pgs.jobs;

import javax.ejb.EJBHome;

public interface JobManagerHome extends EJBHome {
    JobManager create() throws javax.ejb.CreateException, java.rmi.RemoteException;
}
