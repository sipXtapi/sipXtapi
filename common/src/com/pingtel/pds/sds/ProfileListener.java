/*
 * $Id: //depot/OPENDEV/sipXconfig/common/src/com/pingtel/pds/sds/ProfileListener.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
package com.pingtel.pds.sds;

import java.net.URL;
import java.util.Properties;
import java.rmi.Remote;
import java.rmi.RemoteException;

/**
 * The SDS exposes this interface to the ProfileWriter
 */
public interface ProfileListener extends Remote {
    /** Public Method to write a profile object */
    public void updatePhoneProfile ( int profileType,
                                     String macAddress,
                                     String deviceURL,
                                     String model,
                                     String url,
                                     int sequenceNumber )
        throws RemoteException;

    /** Update Core software on Phone Interface (note Catalog URL is pingtelSpecific) */
    public void updatePhoneCoreSoftware ( String macAddress,
                                          String deviceURL,
                                          String model,
                                          String currentVersion,
                                          String targetVersion,
                                          Properties vendorProperties )
        throws RemoteException;

    /** Resets Phone Interface */
    public void resetPhone ( String macAddress,
                             String deviceURL )
        throws RemoteException;

    /** Public Method to notify the SDS that the Document Root changed */
    public void setDocumentRoot ( String documentRoot )
        throws RemoteException;

    /** Public Method to shutdown the service queue */
    public void shutdown()
        throws RemoteException;
}