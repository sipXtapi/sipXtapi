/*
 * $Id: //depot/OPENDEV/sipXconfig/profilewriter/src/com/pingtel/pds/profilewriter/ProfileWriterImpl.java#4 $
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

import java.rmi.Naming;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;

import org.apache.log4j.Category;
import org.apache.log4j.NDC;


import com.pingtel.pds.common.PDSException;

/**
 * The RMI Implementatino file for the Profile Writer
 */
public class ProfileWriterImpl extends UnicastRemoteObject
    implements ProfileWriter {

    private static Category m_logger;

    static {
        m_logger = Category.getInstance( "pds" );
        NDC.pop();
        NDC.push( Thread.currentThread().toString() );
    }

    /** the thread safe worker object that writes the files to disk */
    private ProfileWriterWorker m_profileWorker;

    /**
     * Constructor - binds the name to the registry in the name server
     *
     * @param rmiBoundName
     * @param sdsHostName
     * @param httpBaseDir
     * @param httpServerName
     * @param tftpBaseDir
     * @param tftpServerName
     *
     * @exception RemoteException
     */
    public ProfileWriterImpl ( String rmiBoundName,
                               String sdsHostName,
                               String httpBaseURL,
                               String httpServerName,
                               String publishedHttpBaseURL,
                               String tftpBaseURL,
                               String tftpServerName ) throws RemoteException, PDSException {
        // This line is not necessary however it shows that
        // unicast's superclass exports the object for use
        super();
        try {
            m_logger.debug( "About to bind " + rmiBoundName + " to RMIRegistry" );
            Naming.rebind(rmiBoundName, this);
            m_logger.debug( "Sucessfully bound " + rmiBoundName + " to RMIRegistry" );

            // Map the localHost names to the IP Address
            // Pass along the protocol specific parameters to the Worker
            // where the protocol specific parameters are used
            m_profileWorker =
                new ProfileWriterWorker( sdsHostName,
                                         httpBaseURL,
                                         httpServerName,
                                         publishedHttpBaseURL,
                                         tftpBaseURL,
                                         tftpServerName );

            // Non Pooled Way....
            Thread worker = new Thread( m_profileWorker );
            m_logger.debug( "Starting ProfileWriterWorker" );
            worker.start();
/*
            // Register a runnable PoolEntry with the Pool Manager and get back a handle for it
            Object[] args = new Object[1];
            args[0] = basePath;
            PoolEntryHandle runnableID =
                PoolManager.getInstance().registerPool(
                    ProfileWriterWorker.class, args );
            // This class will be called back when there are threaded exceptions
            ThreadExceptionListener[] listeners = {this};
            PoolManager.getInstance().spawn (runnableID, listeners);
*/
        } catch (Exception ex) {
            if (ex instanceof RemoteException) {
                throw (RemoteException)ex;
            } else {
                throw new RemoteException( ex.getMessage() );
            }
        }
    }


    /**
     * Shutdown the Server implementation - ensures that any worker
     * threads are killed cleanly rather than relying on a finalize
     * method call which is JVM dependent
     */
    public void shutdown() throws RemoteException, PDSException {

        m_logger.info("Shutting down ProfileWriter RMI Server");
        m_profileWorker.shutdown();
        m_logger.info("ProfileWriter RMI Server Shutdown Complete");
    }

    /**
     * This Currently only works for Pingtel Phones as they support a Catalog
     * Cisco Phones get their new (Single versino of software) from a TFTP server
     * upon reboot after a power failure or after a reboot.  The current and target
     * versions must be present in the catalog to succesfully upgrade the phone
     * also the model is in the interface as the mechanism to upgrade a phone
     * may vary depending on the device
     *
     * @param macAddress
     * @param deviceURL
     * @param model
     * @param currentVersion
     * @param targetVersion
     *
     * @exception RemoteException
     */
    public void updatePhoneCoreSoftware ( String macAddress,
                                          String deviceURL,
                                          String model,
                                          String currentVersion,
                                          String targetVersion )
        throws RemoteException {
        throw new java.lang.UnsupportedOperationException();
    }

    /**
     * Installs a Binary release of the software for a specific phone
     *
     * @exception RemoteException
     */
    /** Installs a Binary release of the software for a specific phone */
    public void installCoreSoftware (String manufacturer,
                                     String model,
                                     String leafFileName,
                                     byte[] coreSoftwarePayload )
        throws RemoteException, PDSException {
        m_logger.error( "ProfileWriter does not support installCoreSoftware()" );
        throw new java.lang.UnsupportedOperationException();
    }

    /**
     * Interface to notify a phone to Update its Core Software
     *
     * @param macAddress
     * @param deviceURL
     * @param model
     * @param currentVersion
     * @param targetVersion
     *
     * @exception RemoteException
     */
    public void installCoreSoftwareOnPhone ( String macAddress,
                                             String deviceURL,
                                             String model,
                                             String currentVersion,
                                             String targetVersion )
        throws RemoteException, PDSException {
        m_logger.error( "ProfileWriter does not support installCoreSoftwareOnPhone()" );
        throw new java.lang.UnsupportedOperationException();
    }

    /**
     * The PGS calls this remote RMI method implementation to write
     * a profile to the ProfileWriter.  The profile is just an RMI encoded
     * byte array at this stage
     *
     * @param profileType   one of 4 designated ProfileTypes in PDSDefinitions
     * @param macAddress    the MacAddress of the phone
     * @param macAddress    the SIP Address of the device (used by non enrolling devices)
     * @param manufacturer  the manufacturer
     * @param model         model
     * @param leafFileName  the file name to write to the content delivery server
     * @param profile       the raw XML file byte stream
     *
     * @exception RemoteException
     */
    public void updateDeviceProfile ( int profileType,
                                      String macAddress,
                                      String deviceURL,
                                      String manufacturer,
                                      String model,
                                      String leafFileName,
                                      byte[] profile,
                                      int sequenceNumber )
        throws RemoteException, PDSException {

        m_logger.debug( "Updating device profile for serial# " + macAddress +
                        ", URL " + deviceURL + ", sequence #" + sequenceNumber );
        // Create a delete profile job entry
        ProfileWriterCommand createProfile =
            new CreateProfile (
                profileType,
                macAddress,
                deviceURL,
                manufacturer,
                model,
                leafFileName,
                profile,
                sequenceNumber );
        // queue these on the thread pool for asynchronous processing
        m_profileWorker.sendCommand ( createProfile );
    }

    /**
     * Deletes a Profile from the Content Delivery Server & resets
     * the device.  Ignoring whether the phone received the CheckSync or notify
     * message (as it may or may not be there)
     *
     * @param profileType   Indicates User/Device/Apps etc.
     * @param macAddress    The Serial Number for the Device
     * @param deviceURL     this is an alternative way to addresss the
     *                      phone through the proxy.
     * @param manufacturer  Pingtel or Cisco for now
     * @param model         xpressa or 7960
     * @param leafFileName  the filename of the configuration file that
     *                      needs to be deleted.  In the case of cisco
     *                      this file will mangle the mac address and
     *                      be located in the root folder of the tftp server
     *                      for Pingtel this will be in a special Manufacturer
     *                      /Model/Mac Address/ folder
     *
     * @exception RemoteException
     * @exception PDSException
     */
    public void removeDeviceProfile ( int profileType,
                                      String macAddress,
                                      String deviceURL,
                                      String manufacturer,
                                      String model,
                                      String leafFileName )
        throws RemoteException, PDSException {

        m_logger.debug( "Deleting device profile for serial# " + macAddress +
                        " leaf file name " + leafFileName );
        // Create a delete profile job entry
        ProfileWriterCommand deleteProfile =
            new DeleteProfile (
                profileType,
                macAddress,
                deviceURL,
                manufacturer,
                model,
                leafFileName );
        // queue these on the thread pool for asynchronous processing
        m_profileWorker.sendCommand ( deleteProfile );
    }
}
