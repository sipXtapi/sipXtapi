/*
 * $Id: //depot/OPENDEV/sipXconfig/profilepublisher/src/com/pingtel/pds/sds/ProfileListenerImpl.java#6 $
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

import java.io.File;
import java.io.IOException;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.Properties;
import java.util.Map;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.net.InetAddress;
import java.rmi.Naming;
import java.rmi.RemoteException;
import java.rmi.server.UnicastRemoteObject;

import org.apache.log4j.Category;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.URLPinger;
import com.pingtel.pds.sds.cache.SessionCache;
import com.pingtel.pds.sds.cache.SessionCacheException;
import com.pingtel.pds.sds.cmd.*;
import com.pingtel.pds.sds.sip.SipSessionContext;
import com.pingtel.pds.sds.sip.SipProfileInfo;


/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class ProfileListenerImpl extends UnicastRemoteObject implements ProfileListener {
    /** Filename for last working document root cache */
    private static String DOCROOT_PROPSFILE = "docroot.props";

    private static Category m_logger;

    static {
        m_logger = Category.getInstance( "pds" );
    }

    /**
     * this url points to a catalog server with the rules of how to upgrade from
     * one version of softwware to another version.
     */
    private String m_xpressaCatalogURL;

    /** the thread safe worker that sends notifications to the phones */
    private ProfileListenerWorker m_worker;

    /** This is needed to look up session information to send to phone */
    private SessionCache m_sessionCache;

    /** @param bindName indicates the RMI name that this is known as */
    public ProfileListenerImpl ( String bindName,
                                 SessionCache sessionCache,
                                 String xpressaCatalogURL,
                                 Object mutexLock )
        throws RemoteException {

        // This line is not necessary however it expemplifies that
        // unicast's superclass exports the object for use
        super();

        m_logger.debug( "creating ProfileListenerImpl " + bindName );
        m_sessionCache = sessionCache;
        try {
            Naming.rebind(bindName, this);
            m_logger.debug( "bound ProfileListenerImpl to RMIRegistry" );
            m_worker = new ProfileListenerWorker( sessionCache, mutexLock );
            // Now try to see if we can restore the persosted documentroot
            String lastDocumentRoot = getLastWorkingDocumentRoot();
            m_worker.setDocumentRoot( lastDocumentRoot );
            m_logger.info( "setting document root as " + lastDocumentRoot );
            Thread thread = new Thread (m_worker);
            thread.start();
        } catch (java.net.MalformedURLException ex) {
            throw new RemoteException ( ex.getMessage() );
        }
    }

    /** This method is called over RMI when the profile Writer wants to reset a device */
    public void resetPhone ( String macAddress, String sipToURL )
        throws RemoteException {

        m_logger.debug( "got reset phone message from Config Server: " +
                        "serial# " + macAddress + " URL: " + sipToURL );
        try {
            Command command = null;
            if ( macAddress != null ) {
                // Pingtel Device (this is how we tell via MAC address)
                // Fetch its database context information, there may be multiple contexts
                // however we are only interested in the first one here since we are going to
                // reset the phone
                HashMap deviceContexts = m_sessionCache.findSessionContexts( macAddress );
                Iterator iter = deviceContexts.entrySet().iterator();
                if ( iter.hasNext() ) {
                    Map.Entry mapEntry = (Map.Entry) iter.next();
                    Integer contextID = (Integer) mapEntry.getKey();
                    SipSessionContext context = (SipSessionContext) mapEntry.getValue();

                    String toAddress = context.getToAddress();
                    String contactAddress = toAddress;
                    // Strip off the trailing name value pairs
                    if ( contactAddress.indexOf(';') > 0 ) {
                        contactAddress = contactAddress.substring(0, contactAddress.indexOf(';'));
                    }
                    // Format a restart command
                    command = new RestartXpressaCommand (
                        XpressaReceiver.getInstance(),
                        context.getFromAddress(),
                        toAddress,
                        context.getCallId(),
                        contactAddress,
                        context.getSequenceNumber());
                    // for a given device there is only need
                    // to send a single command per mac address
                    // so break out of the loop after enqueing this command
                    m_worker.enqueueCommand( command );
                } else {
                    m_logger.info( "Unable to reset device with Mac:" +
                                    macAddress + " sipToURL: " + sipToURL +
                                    ", Device not enrolled" );
                }
            } else { /* 7960 */
                if (sipToURL != null) {
                    command =
                            new RestartCiscoCommand (
                                    CiscoReceiver.getInstance(),
                                    macAddress,
                                    sipToURL );

                    m_worker.enqueueCommand(command);
                }
            }
        } catch (SessionCacheException ex) {
            throw new RemoteException (ex.getMessage());
        }
    }

    /**
     * Update Core software on Phone Interface (note Catalog
     * URL is pingtelSpecific) @todo change catalog to properties object.
     */
    public void updatePhoneCoreSoftware ( String macAddress,
                                          String deviceURL,
                                          String model,
                                          String currentVersion,
                                          String targetVersion,
                                          Properties vendorProperties )
        throws RemoteException {

        m_logger.debug( "updating phone core software: serial# " + macAddress +
                        " URL " + deviceURL + " model " + model +
                        " current version " + currentVersion +
                        " target version " + targetVersion );
        Command command;
        if ( model.equalsIgnoreCase( PDSDefinitions.MODEL_HARDPHONE_CISCO_7960 ) ) {
            // Updating the core software for Cisco means reboot
            // the phone, the software image must be available on
            // the tftp server and the syncinfo.xml's SYNC="3" value
            // must be changed prior to this.
            command = new UpgradeCiscoCommand ( CiscoReceiver.getInstance(),
                                                macAddress,
                                                deviceURL );
        } else {
            // This command is for a phone that supports upgrade via
            // the Notify message.  We pass in both the Model
            // and vendor specific properties in order that we can support
            // all phones
            command = new UpgradeXpressaCommand( XpressaReceiver.getInstance(),
                                                 macAddress,
                                                 deviceURL,
                                                 currentVersion,
                                                 targetVersion,
                                                 m_xpressaCatalogURL );
        }
        m_worker.enqueueCommand(command);
    }


    /**
     * When a profile is rendered by the profile writer the profile writer
     * calls this method to notify the phone with the updated URL.
     * This method must check to ensure that the URL is pingable and then
     * update the Database with the new profile information.  Finally
     * a command containing the URL will be enqueued for sending to the device.
     */
    public void updatePhoneProfile ( int profileType,
                                     String macAddress,
                                     String deviceURL,
                                     String model,
                                     String url,
                                     int sequenceNumber )
        throws RemoteException {

        m_logger.debug( "updating phone profile: serial# " + macAddress +
                        "URL " + deviceURL + " model " + model +
                        " sequence# " + sequenceNumber );

        // EnQueue the new command on the worker queue to update the
        // phone core software
        Command command;
        if ( model.equalsIgnoreCase ( PDSDefinitions.MODEL_HARDPHONE_CISCO_7960 ) ||
                model.equalsIgnoreCase ( PDSDefinitions.MODEL_HARDPHONE_CISCO_7940 )) {
            // Updating the core software for Cisco means reboot
            // the phone, the software image must be available on
            // the tftp server root prior to this
            command = new UpdateCiscoProfileCommand (
                CiscoReceiver.getInstance(), macAddress, deviceURL );
            m_worker.enqueueCommand( command );
        } else { // Pingtel Phone (Supports Notify Update mechanism)
            try {
                // Find all the device contexts associated with the
                // mac address.  There should be one per Event type
                // @JC BUG unfortunately the CSequence number is the same
                // for them all!!! The workaround is to ensure that
                // updating the updateSessionContextProfileInfo also
                // increments the CSeq amd we locally increment it here
                HashMap deviceContexts =
                    m_sessionCache.findSessionContexts( macAddress );

                if ( !deviceContexts.isEmpty() ) {
                    m_logger.debug ( "device with MAC: " + macAddress + " is enrolled ");
                    Iterator iter = deviceContexts.entrySet().iterator();
                    int nextOutputCseq = -1;
                    while ( iter.hasNext() ) {
                        Map.Entry mapEntry = (Map.Entry) iter.next();
                        Integer contextID = (Integer) mapEntry.getKey();
                        SipSessionContext context = (SipSessionContext) mapEntry.getValue();

                        // the SDS database stores the last URL and sequence numbers
                        // that were sent to the phone.  These may be different
                        SipProfileInfo sipProfileInfo =
                            new SipProfileInfo ( profileType, url, sequenceNumber );

                        // @JC unavoidable Hack
                        if ( nextOutputCseq < 0 )
                            nextOutputCseq = context.getSequenceNumber();
                        else
                            nextOutputCseq+=1;

                        m_logger.debug ( "increased cseq to " + nextOutputCseq + " for MAC: " + macAddress + " is enrolled ");

                        // Because we changed the sequence number for the URL
                        // we must also update the database table row and
                        // the CSequence number
                        m_sessionCache.updateSessionContextProfileInfo (
                            contextID, sipProfileInfo );

                        m_logger.debug ( "updated sipProfileInfo for MAC: " + macAddress );

                        ArrayList newProfile = new ArrayList(1);
                        newProfile.add( sipProfileInfo );

                        command = new UpdateXpressaProfileCommand (
                            XpressaReceiver.getInstance(),
                            macAddress,
                            context.getToAddress(),
                            context.getFromAddress(),
                            context.getCallId(),
                            nextOutputCseq,
                            m_worker.getDocumentRoot(),
                            newProfile );

                        m_logger.debug ( "created new xpressa command for MAC: " + macAddress );
                        m_worker.enqueueCommand( command );
                    }
                } else {
                    m_logger.info( "Unable to update macAddress: " +
                                    macAddress + " deviceURL: " + deviceURL +
                                    " as not enrolled" );
                }
            } catch (SessionCacheException ex) {
                m_logger.fatal("Problem with SesssionCache", ex );

                throw new RemoteException (ex.getMessage());
            }
        }
    }

    /**
     * Profile Writer calls this interface to indicate that
     * the document rooot changed, we firstly verify that we can read
     * the URL before saving the docroot to a temp file
     */
    public void setDocumentRoot ( String documentRoot )
        throws RemoteException {

        // ensre that we can ping the specified address
        //if ( URLPinger.isValid( documentRoot ) ) {
            // Create or Update the DOCROOT_PROPSFILE persistent file
            try {
                // Make Sure we convert the documentRoot into something
                // that the phone can use (localhost or 127.0.0.1 will not do)
                // we know that the address is pingable so we do not have to reping
                String ipAddress = InetAddress.getLocalHost().getHostAddress();

                // Fix up the document root if it contains either the
                // localhost or loopback address
                if ( (documentRoot.toLowerCase().indexOf("localhost") >=0) ||
                     (documentRoot.toLowerCase().indexOf("127.0.0.1") >=0) ) {
                    int idx1= documentRoot.toLowerCase().indexOf("localhost");
                    int idx2 = documentRoot.toLowerCase().indexOf("127.0.0.1");
                    int startIndex = (idx1 >=0)? idx1 : idx2;
                    StringBuffer sb = new StringBuffer(documentRoot.toLowerCase());
                    // substitute 'localhost' or '127.0.0.1' with the ip address
                    sb.replace (startIndex, startIndex + "localhost".length(), ipAddress);
                    documentRoot = sb.toString();
                }

                FileOutputStream stream = new FileOutputStream ( DOCROOT_PROPSFILE );
                Properties docrootProps = new Properties();
                docrootProps.setProperty( "docroot", documentRoot );
                docrootProps.store(stream, "docroot properties file");
                stream.close() ;
                // Finally copy the document root to the woker thread
                // which is where the root reference is stored
                m_worker.setDocumentRoot (documentRoot);
            } catch ( IOException ex ) {
                m_logger.error( "Unable to persist " + DOCROOT_PROPSFILE +
                                " file, check directory permissions" ,
                                ex );
            }
        //} else {
        //    m_logger.fatal( "invalid document root: " + documentRoot );
        //    throw new RemoteException ("invalid document root: " + documentRoot);
        //}
    }

    /** getter for the document root */
    public String getDocumentRoot () {
        return m_worker.getDocumentRoot();
    }

    /** Remove Method to terminate the SDS RMI service */
    public void shutdown ()
        throws RemoteException {
        m_worker.shutdown();
    }

    /**
     * Helper method to open the last known properties file containing
     * the document root url.  The last known document root is also
     * pinged to ensure that the web server is up and running
     * @return null if we cannot find a working document root
     */
    private String getLastWorkingDocumentRoot() {
        String lastWorkingDocumnetRoot = null;
        Properties docrootProps = new Properties();
        try {
            FileInputStream docRootFile =
                new FileInputStream ( DOCROOT_PROPSFILE );
            docrootProps.load( docRootFile );
            docRootFile.close() ;
        } catch ( IOException ex ) {
            // File does not exist, create an http://localhost:80/
            // document root.  This localhost string will be translated to
            // the IP Address of the local host (provided it is pingable)
            try {
                setDocumentRoot( "http://localhost:80/" );
            } catch ( IOException ex1 ) {
                return null;
            }
        }

        // There are a couple of situations where the docroot may come
        // back as unpingable but be still valid, this would be the case
        // if directory navivation was set to false so assume that the
        // docroot is valid if the docrootProps has a non null value
        String documentRoot = docrootProps.getProperty("docroot");
        if (documentRoot != null) {
            documentRoot = documentRoot.trim();
            // @JC if ( URLPinger.isValid( documentRoot ) )
            return documentRoot;
        }
        return null;
    }
}