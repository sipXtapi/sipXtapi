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

import java.rmi.RemoteException;

import org.jdom.Element;

import com.pingtel.pds.common.PDSException;


public interface JobManagerBusiness {

    public void updateJobProgress ( int jobID, String progress )
            throws PDSException, RemoteException;

    public void updateJobStatus ( int jobID, String status, String exceptionMessage )
            throws PDSException, RemoteException;

    public Element getCurrentJobStatuses () throws PDSException, RemoteException;

    public Element getAllJobStatuses () throws PDSException, RemoteException;

    public void flushJobs (  ) throws PDSException, RemoteException;

    public int createJob ( int jobType, String details, String progress )
            throws PDSException, RemoteException;

    public Element getExceptionMessage ( int jobID )
            throws PDSException, RemoteException;

}
