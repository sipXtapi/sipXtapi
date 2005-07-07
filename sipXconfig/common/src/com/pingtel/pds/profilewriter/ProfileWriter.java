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
package com.pingtel.pds.profilewriter;

import java.rmi.Remote;
import java.rmi.RemoteException;

import com.pingtel.pds.common.PDSException;

/** Remote RMI interface business method signatures for writing profiles */
public interface ProfileWriter extends Remote {

    /** interface to update the a single profile on the Phone */
    public void updateDeviceProfile ( int profileType,
                                      String macAddress,
                                      String deviceURL,
                                      String manufacturer,
                                      String model,
                                      String leafFileName,
                                      byte[] profile,
                                      int sequenceNumber )
        throws RemoteException, PDSException;

    /** Interface to notify a phone to Update its Core Software */
    public void installCoreSoftwareOnPhone ( String macAddress,
                                             String deviceURL,
                                             String model,
                                             String currentVersion,
                                             String targetVersion )
        throws RemoteException, PDSException;

    /**
     *  Deletes a Profile from the Content Delivery Server and resets
     *  phone ignoring whether the phone received the CheckSync or notify
     *  message (as it may or may not be there)
     */
    public void removeDeviceProfile ( int profileType,
                                      String macAddress,
                                      String deviceURL,
                                      String manufacturer,
                                      String model,
                                      String leafFileName )
        throws RemoteException, PDSException;

    /** Installs a Binary release of the software for a specific phone */
    public void installCoreSoftware (String manufacturer,
                                     String model,
                                     String leafFileName,
                                     byte[] coreSoftwarePayload )
        throws RemoteException, PDSException;

    /** Public Method to shutdown the worker queue */
    public void shutdown()
        throws RemoteException, PDSException;
}
