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

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.sds.ProfileListener;
import org.apache.log4j.Category;
import org.apache.log4j.NDC;

import java.io.*;
import java.net.InetAddress;
import java.net.MalformedURLException;
import java.net.URL;
import java.rmi.Naming;
import java.rmi.NotBoundException;
import java.rmi.RemoteException;
import java.util.ArrayList;

/**
 *  Class that is responsible for dequeueing a set of prerendered profiles
 *  from a worker queue and rendering them according to the specified
 *  rendering algorithm.  This class runs as a thread and can be shutdown
 *  remotely.
 */
public class ProfileWriterWorker implements Runnable {

    /** OS Independent file separator used to creaete the files */
    private static final String m_fileSeparator = System.getProperty ( "file.separator" );

    // directory into which the cisco mini-profiles get created.   They
    // are later coalesced into the real profile.
    private static final String m_partialProfilesDirectory =
            ".." + m_fileSeparator + "data" + m_fileSeparator + "partial" + m_fileSeparator;

    private URL m_internalHttpBaseURL;

    private String m_internalHttpServerName;

    private URL m_tftpBaseURL;

    private String m_tftpServerName;

    // the base part of the HTTP URLs that is sent to the devices via the SDS.
    // This may be different from the location that it is written to from the PW.
    private String m_publishedHttpBaseURL;

    private boolean m_remoteHTTPServer;

    private boolean m_remoteTFTPServer;




    /** This represents the hostname for the location of the SDS Service */
    private String m_sdsHostName = null;

    /** Determine if the document root was sent */
    private boolean m_namingLookupRequired = true;

    /** queue of profiles that need to be written out to the file system */
    private ArrayList m_queueAndLock = new ArrayList();

    /** flag used to cleanly shutdown the service */
    private boolean m_shutdown = false;

    /** the worker thread - used to wait for the thread to finish (join method) */
    private Thread m_workerThread = null;


    /** This RMI listener (server) in the SDS waits for Rendered Profile Updates */
    private ProfileListener m_profileListener = null;

    private static Category m_logger;

    static {
        m_logger = Category.getInstance( "pds" );
    }


    public ProfileWriterWorker ( String sdsHostName,
                                 String internalHttpBaseURL,
                                 String internalHttpServerName,
                                 String publishedHttpBaseURL,
                                 String tftpBaseURL,
                                 String tftpServerName )
        throws IOException {

        boolean atLeastOneRemoteProtocol = false;
        // Determine the Local IP Address as we need to factor this
        // into determining whether this is equivalent to localHost
        String localIPAddress = InetAddress.getLocalHost().getHostAddress();

        // the remove SDS host where we send RMI profile
        // and other update notifications
        m_sdsHostName = sdsHostName;

        // Determine if we support a TFTP server repository for profiles
        // (these are used for Cisco phones)
        if ( (tftpServerName != null) && (tftpBaseURL != null) ) {

            m_logger.info( "Using TFTP content delivery server" );

            if ( !tftpBaseURL.endsWith("/") )
                tftpBaseURL+="/";
            // Ideally this should be file protocol allowing local access
            m_tftpBaseURL = new URL ( tftpBaseURL );
            m_tftpServerName = tftpServerName;

            if ( m_tftpBaseURL.getProtocol().equals("file") ) {
                File file = new File ( m_tftpBaseURL.getFile() );
                if (!file.isDirectory() || !file.canWrite() ) {
                    throw new IOException ( "TFTP Local Folder inaccessible, " +
                                             m_tftpBaseURL.getFile() );
                }
            }
            atLeastOneRemoteProtocol = true;
        }

        // Determine if we support a web server repository for profiles
        // (these are used for Xpressa phones)
        if ( (internalHttpServerName != null) && (internalHttpBaseURL != null) ) {

            m_logger.info( "Using HTTP content delivery server" );

            if ( !internalHttpBaseURL.endsWith("/") )
                internalHttpBaseURL+="/";

            // Ideally this should be file protocol allowing local access
            m_internalHttpBaseURL = new URL ( internalHttpBaseURL );
            m_logger.debug( "HTTP base URL " + internalHttpBaseURL );

            // validate the HTTP Service - BEGIN
            if ( ( internalHttpServerName.equalsIgnoreCase("localhost") ||
                   internalHttpServerName.equalsIgnoreCase("127.0.0.1") ||
                   internalHttpServerName.equalsIgnoreCase(localIPAddress) ) &&
                   internalHttpBaseURL != null ) {
                // Ensure we have an IP Address that we can deliver to the phone
                // this is not needed for TFTP for the present as the TFTP server
                // ip address or location is not sent to the phone
                m_internalHttpServerName = localIPAddress;
                m_logger.debug( "HTTP server is local" );
            } else {
                m_internalHttpServerName = internalHttpServerName;
                m_logger.info( "HTTP server name " + m_internalHttpServerName );
                m_internalHttpBaseURL = new URL (System.getProperty("java.io.tmpdir"));
                m_remoteHTTPServer = true;
            }

            if ( m_internalHttpBaseURL.getProtocol().equals("file") ) {
                File file = new File ( internalHttpBaseURL.substring( internalHttpBaseURL.indexOf("file://") + 6 ) );
                if (!file.isDirectory() || !file.canWrite() ) {
                    throw new IOException ( "HTTP Local Folder inaccessible, " +
                                             m_internalHttpBaseURL.getFile() );
                }
            }
            atLeastOneRemoteProtocol = true;
        }

        m_publishedHttpBaseURL = publishedHttpBaseURL;

        if (!atLeastOneRemoteProtocol) {
            m_logger.fatal( "Profile Storage inaccessible,"+
                            " check configuration file");

            throw new IOException ( "Profile Storage inaccessible, check configuration file");
        }
   }

    /**
     * the worker thread PreRenderedProfile objects from a shared
     * queue object and writes them out to disk.  This worker thread
     * eliminates the bottleneck
     */
    public void run () {
        // Capture the thread information
        m_workerThread = Thread.currentThread();

        // Name the thread so it is easier to find in the debugger
        m_workerThread.setName("ProfileWriterWorker");

        NDC.push( "ProfileWriter" );
        NDC.push( m_workerThread.toString() );

        while ( !m_shutdown ) {
            synchronized ( m_queueAndLock ) {
                // Firstly we must ensure that the document root is
                // sent to the SDS othwerise subscriptions notifications
                // will be rejected by the SDS, we do not depend on the document root
                if ( m_namingLookupRequired ) {
                    try {
                        sendDocumentRootToSDS();
                        // Successfully sent so toggle the bind flag
                        m_namingLookupRequired = false;
                    } catch ( MalformedURLException ex ) {
                        m_namingLookupRequired = false;
                        m_logger.fatal( "Terminating worker thread " + ex.toString() );
                        return;
                    } catch ( RemoteException ex ) {
                        // runtime error so remote object should be ok
                        m_namingLookupRequired = false;
                        m_logger.error( "Cannot send docroot to SDS, is SDS started? " +
                                        ex.toString(), ex );
                    } catch ( NotBoundException ex ) {
                        m_namingLookupRequired = true;
                        m_logger.info( "Cannot send docroot to SDS, is SDS started? " +
                                        ex.toString());
                        try {
                            Thread.currentThread().sleep(1000);
                        } catch (InterruptedException ex1) {
                            break;
                        }
                    }
                }

                // Block weaiting for Command posting
                while ( m_queueAndLock.isEmpty() ) {
                    try {
                        m_logger.debug("in run: right before wait()");
                        m_queueAndLock.wait();
                        m_logger.debug("in run: right after wait()");
                    } catch (InterruptedException iex) {
                        m_logger.debug("in run: got interruped");
                        break;
                    }
                }

                // we should have a non-empty queue here with a Command entry
                // either a create or delete profile command Command
                if ( !m_queueAndLock.isEmpty() && !m_shutdown ) {
                    m_logger.debug("in run: m_queueAndLock is not empty");
                    // remove work element from head of Commandqueue
                    Object nextCmd = m_queueAndLock.get(0);
                    m_queueAndLock.remove(0);
                    m_logger.debug("in run:removed command from m_queueAndLock");

                    try {
                        // What are we trying to do?
                        switch ( ((ProfileWriterCommand)nextCmd).getOperation() ) {

                            /////////////////////////////////////////
                            case ProfileWriterCommand.CREATE_PROFILE: {
                            /////////////////////////////////////////
                                handleCreateProfileCommand(nextCmd);
                                break;
                            }
                            /////////////////////////////////////////
                            case ProfileWriterCommand.DELETE_PROFILE: {
                            /////////////////////////////////////////
                                handleDeleteProfileCommand(nextCmd);
                                break;
                            }
                            ///////////////////////////////////////////
                            case ProfileWriterCommand.UPGRADE_SOFTWARE: {
                            ///////////////////////////////////////////
                                UpgradeSoftware upgradeSoftwareCommand = (UpgradeSoftware)nextCmd;
                                break;
                            }
                            ////////
                            default: {
                            ////////
                                m_logger.error("Unrecognized Command object containing command id: " +
                                               ((ProfileWriterCommand)nextCmd).getOperation());
                            }
                        }   // switch
                    }
                    catch (IOException ioex) {
                        // Cannot do very much here as we should have write access to the DB
                        m_logger.error( "exception occured " + ioex.getMessage(), ioex );
                    }
                }
            }
        }
        m_logger.info( "Shutdown Received by Worker Thread" );
    }

    private void handleDeleteProfileCommand(Object nextCmd) throws IOException {
        DeleteProfile deleteProfileCommand = (DeleteProfile)nextCmd;
        String profileFileName = null;

        URL baseURLFolder = null;

        // Are we dealing with a Cisco Phone?  If so bump the
        // sequence number in the syncinfo.xml file in on the TFTP root


        if ( !deleteProfileCommand.getModel().equalsIgnoreCase (
                PDSDefinitions.MODEL_HARDPHONE_CISCO_7960 ) &&
                !deleteProfileCommand.getModel().equalsIgnoreCase (
                PDSDefinitions.MODEL_HARDPHONE_CISCO_7940 )) {

            baseURLFolder = m_internalHttpBaseURL;

            // @JC Put a change in here to ensure that the
            // fully qualified path to the profile is lower case.
            // Note that we do not attempt to lowercase the URLBaseFolder
            // as this is content delivery server specific
            profileFileName = ( deleteProfileCommand.getManufacturer() + m_fileSeparator  +
                                deleteProfileCommand.getModel() + m_fileSeparator  +
                                deleteProfileCommand.getMacAddress() + m_fileSeparator +
                                deleteProfileCommand.getLeafFileName() ).toLowerCase();

            m_logger.debug (  "deleting profile " + baseURLFolder.getFile() +
                        deleteProfileCommand.getManufacturer() + m_fileSeparator +
                        deleteProfileCommand.getModel() + m_fileSeparator  +
                        deleteProfileCommand.getMacAddress() + m_fileSeparator +
                        deleteProfileCommand.getLeafFileName()  );

            File deleteFile =
                new File ( ( baseURLFolder.getFile() + deleteProfileCommand.getManufacturer() + m_fileSeparator +
                            deleteProfileCommand.getModel() + m_fileSeparator  +
                            deleteProfileCommand.getMacAddress() + m_fileSeparator +
                            deleteProfileCommand.getLeafFileName() ).toLowerCase() );

            if ( deleteFile.exists() ) {
                m_logger.debug (  "deleting profile file exists" );
                deleteFile.delete();
                m_logger.debug (  "deleting profile deleted file" );

                File parentDir = new File ( ( baseURLFolder.getFile() + deleteProfileCommand.getManufacturer() + m_fileSeparator +
                            deleteProfileCommand.getModel() + m_fileSeparator  +
                            deleteProfileCommand.getMacAddress() ).toLowerCase() );

                deleteEmptyParentFolder(parentDir);

            }
            else {
                m_logger.debug (  "deleting profile file doesn't exist" );
            }
        }
        else { // we are dealing with a 79xx device
            m_logger.error ( "Deleting profile for Cisco Phone" );

            profileFileName = ( deleteProfileCommand.getManufacturer() + "/"  +
                                    deleteProfileCommand.getModel() + "/"  +
                                    deleteProfileCommand.getMacAddress() + "/" +
                                    deleteProfileCommand.getLeafFileName() );

            File profile = new File ( m_partialProfilesDirectory + profileFileName );

            if ( profile.exists() ) {
                profile.delete();
            }

            File parentDir =
                    new File ( m_partialProfilesDirectory +
                                deleteProfileCommand.getManufacturer() + "/"  +
                                deleteProfileCommand.getModel() + "/"  +
                                deleteProfileCommand.getMacAddress() );

            boolean isEmpty = deleteEmptyParentFolder(parentDir);

            if (isEmpty) {
                // all the mini-profiles have been deleted at this point
                // (this will happen only when a device has been
                // explicitly deleted) therefore we should delete the real
                // profile too.
                String profileName = calculateCiscoProfileName(deleteProfileCommand.getMacAddress());
                File profileFile = new File (profileName);
                if (profileFile.exists()) {
                    profileFile.delete();
                    m_logger.debug( "deleted profile: " + profileName);
                }
            } else {
                // this looks like a bug/bad code but it isn't.  We really DO want
                // to create a new profile with a CreateProfile command.   Look
                // at how coalesceMiniProfiles() works.
                CreateProfile createCommand =
                        new CreateProfile ( deleteProfileCommand.getProfileType(),
                                deleteProfileCommand.getMacAddress(),
                                deleteProfileCommand.getDeviceURL(),
                                deleteProfileCommand.getManufacturer(),
                                deleteProfileCommand.getModel(),
                                deleteProfileCommand.getLeafFileName(),
                                null, // no byte stream needed
                                0 ); // no sequence number needed

                coalesceMiniProfiles ( createCommand );
            }
        }
    }

    private boolean deleteEmptyParentFolder(File parentDir) {
        boolean isEmpty = false;
        File [] remainingFiles = parentDir.listFiles();

        if (!parentDir.exists()) {
            isEmpty = true;
        }
        else if (remainingFiles != null && remainingFiles.length == 0) {
            m_logger.debug ( "parent directory is empty going to delete it" );
            parentDir.delete();
            isEmpty = true;
        }

        return isEmpty;
    }


    /**
     * logic for create profile primitive for the profile writer.
     * @param nextCmd command taken from the work queue in the run() method.
     * @throws IOException
     */
    private void handleCreateProfileCommand(Object nextCmd) throws IOException {

        CreateProfile createProfileCommand = (CreateProfile)nextCmd;

        URL targetBaseURLFolder = null;
        // Are we dealing with a Cisco Phone?  If so bump the
        // sequence number in the syncinfo.xml file in on the TFTP root
        if ( createProfileCommand.getModel().equalsIgnoreCase(
                PDSDefinitions.MODEL_HARDPHONE_CISCO_7960 ) ||
                    createProfileCommand.getModel().equalsIgnoreCase (
                        PDSDefinitions.MODEL_HARDPHONE_CISCO_7940 )) {
            try {
                targetBaseURLFolder = m_tftpBaseURL;
            } catch (Exception ex) {
                // Log the event to the log file
                m_logger.error( "Unable to Update Cisco Catalog XML file", ex );
            }
        }
        else {
            targetBaseURLFolder = m_internalHttpBaseURL;
        }

        String profileFileName = createProfileCommand.getLeafFileName();
        // Xpressa and other pingtel phones use a special Manufacturer
        // /Model/Serial(aka MAC) folder, also the file separator must
        // be encoded specially here, we must ensure that the
        // file separator is "/" as this is sent as a URL offset to the SDS
        if ( createProfileCommand.getModel().equalsIgnoreCase (
                PDSDefinitions.MODEL_HARDPHONE_XPRESSA ) ||
                    createProfileCommand.getModel().equalsIgnoreCase (
                        PDSDefinitions.MODEL_SOFTPHONE_WIN ) ) {

            // @JC Put a change in here to ensure that the
            // fully qualified path to the profile is lower case.
            // Note that we do not attempt to lowercase the URLBaseFolder
            // as this is content delivery server specific
//            profileFileName = ( createProfileCommand.getManufacturer() + m_fileSeparator  +
//                                createProfileCommand.getModel() + m_fileSeparator  +
//                                createProfileCommand.getMacAddress() + m_fileSeparator +
//                                profileFileName ).toLowerCase();

            profileFileName = ( createProfileCommand.getManufacturer() + "/"  +
                                createProfileCommand.getModel() + "/"  +
                                createProfileCommand.getMacAddress() + "/" +
                                profileFileName ).toLowerCase();

            m_logger.debug ( "Creating profile, filename " + profileFileName );

            // Writes the profile to the designated location
            // at this stage only local file writes supported
            writeProfile ( targetBaseURLFolder,
                           profileFileName,
                           createProfileCommand.getProfileData() );

            // Log the event to the log file
            m_logger.debug ( "Wrote Profile for : " +
                createProfileCommand.getMacAddress() +
                " to disk" );

        }
        else if ( createProfileCommand.getModel().equalsIgnoreCase(
                PDSDefinitions.MODEL_HARDPHONE_CISCO_7960 ) ||
                    createProfileCommand.getModel().equalsIgnoreCase (
                        PDSDefinitions.MODEL_HARDPHONE_CISCO_7940 ) ) {

            profileFileName = ( createProfileCommand.getManufacturer() + "/"  +
                                createProfileCommand.getModel() + "/"  +
                                createProfileCommand.getMacAddress() + "/" +
                                profileFileName );

            m_logger.debug ( "Creating profile, filename " + profileFileName );

            writePartialProfile ( profileFileName, createProfileCommand.getProfileData() );

            m_logger.debug ( "Wrote Profile for : " +
                createProfileCommand.getMacAddress() +
                " to disk" );

            coalesceMiniProfiles ( createProfileCommand );
        }
        else {
            //m_logger.error ( "Cannot create profile for Cisco Phone" );
            //profileFileName = createProfileCommand.getLeafFileName();
            //continue;
            //assert!!!
        }

        // the SDS must receive the profileFileName
        // as a URL offset to add thet the base url
        // this MUST be in Unix file separator format
        String profileUrlOffset = profileFileName;
        if ( m_fileSeparator != "/" ) {
            profileUrlOffset = profileUrlOffset.replace('\\', '/');
        }

        // Send the profile's url to the SDS so it can
        // be send to the device over SIP
        sendProfileToSDS (
            createProfileCommand.getProfileType(),
            createProfileCommand.getMacAddress(),
            createProfileCommand.getDeviceURL(), // For Devices that do not support enrollment (e.g cisco)
            createProfileCommand.getModel(),
            profileUrlOffset,  // this is the offset from the 'docroot' (must contain '/' file separators)
            createProfileCommand.getSequenceNumber() );
    }


    /** Initializes the ProfileWriter to SDS interface */
    private void sendDocumentRootToSDS ()
        throws RemoteException, NotBoundException, MalformedURLException {
        // The SDS will use this parameter as a prefix string to
        // generate the profiles url's to send to the phone
        if ( m_namingLookupRequired == true )  {
            m_profileListener = lookupSDSInterface();
        }

        m_logger.debug( "setting docroot" +
                        m_publishedHttpBaseURL + " to SDS" );

        m_profileListener.setDocumentRoot( m_publishedHttpBaseURL );

        m_logger.debug( "set docroot" );
    }

    /**
     * Sends context information to the SDS indicating that there is a
     * rendered profile for a parcicular mac address
     */
    private void sendProfileToSDS ( int profileType,
                                    String macAddress,
                                    String deviceURL,
                                    String model,
                                    String urlOffset,
                                    int sequenceNumber )
        throws IOException {
        // First Usage Looks up the SDS listener interface
        try {
            m_logger.debug( "Sending profile to SDS, " +
                            "MAC " + macAddress +
                            ",URL " + deviceURL +
                            ",model " + model +
                            ",URL offset " + urlOffset +
                            ",sequence #" + sequenceNumber );
            // Make sure we have a listening SDS
            if ( m_profileListener == null )  {
                m_logger.debug( "Setting docroot" +
                        "http://" + m_internalHttpServerName +
                        ":80/" + " to SDS" );

                m_profileListener = lookupSDSInterface();

                m_logger.debug( "Located SDS RMI Server" );

                // Ensure we send the docroot again
                m_profileListener.setDocumentRoot ( "http://" + m_internalHttpServerName + ":80/" );

                m_logger.debug( "Successfully sent Document Root to SDS" );
            }

            String newURLOffset = urlOffset;

            // the URL Offset must contain forward slashes (windoze only problem)
            if ( m_fileSeparator.equals("\\") ) {
                int index, lastIndex = 0;
                String str = "";
                while ( (index = urlOffset.indexOf("\\", lastIndex) ) != -1 ) {
                    str += urlOffset.substring(lastIndex, index) + "/";
                    lastIndex = index + 1;
                }
            }

            // Notify the SDS that we have a new profile. If the SDS
            // has this phone currrently subscribed it will immediately
            // send the updated profile to the phone, the 2nd parameter
            // here is relative to the document root on the web server
            m_profileListener.updatePhoneProfile(
                profileType, macAddress,
                deviceURL, model,
                urlOffset, sequenceNumber );

            m_logger.debug( "Profile Sent" );
        } catch (NotBoundException ex) {
            m_namingLookupRequired = true;
            m_logger.error( "Error: SDS not bound in RMI Registry: " +
                                        ex.toString());

            throw new IOException ("Error: SDS not bound in RMI Registry: " + ex.getMessage());
        } catch (RemoteException ex) {
            m_namingLookupRequired = true;
            m_logger.error( "could not send profile to SDS: " +
                                        ex.toString());

            throw new IOException ("could not send profile to SDS: " + ex.getMessage());
        }
    }

    private ProfileListener lookupSDSInterface()
            throws NotBoundException, MalformedURLException, RemoteException {

        return (ProfileListener) Naming.lookup( "rmi://" + m_sdsHostName +
                ":2001/SDSProfileListener" );
    }

    /** Queues a profile object for writing to disk */
    public void sendCommand ( ProfileWriterCommand command ) {
        synchronized ( m_queueAndLock ) {
            m_logger.debug("in sendCommand: adding profile for " + command.getMacAddress());
            // Append the object to the end of the queue
            m_queueAndLock.add ( command );
            m_logger.debug("in sendCommand: added profile");
            m_queueAndLock.notifyAll();
            m_logger.debug("in sendCommand: notified all");
        }
    }

    /** Shutdown the worker thread and wait for it to terminate */
    public void shutdown() {
        m_logger.debug( "Shutting down ProfileWriterWorker" );
        m_shutdown = true;
        m_workerThread.interrupt();
        // Wait for the worker thread to accept the shutdown signal
        // and terminate (wait up to one second to shutdown)
        try { m_workerThread.join(1000); } catch (InterruptedException iex) {}

        m_logger.debug( "ProfileWriterWorker Shutdown complete" );
    }

    /**
     *  Writes the profile to disk, this happens in the context
     *  of the worker thread
     *  @param profile prerendered profile object
     *  @exception java.io.IOException
     */
    private void writeProfile ( URL baseDirectoryURL,
                                String profileFileName,
                                byte[] profile )
        throws IOException {

        m_logger.debug( "Writing profile: URL " +
                        baseDirectoryURL + profileFileName );

        // Upload via TFTP if the base directory for profiles is remote
        if ( !baseDirectoryURL.getProtocol().equals("file") ) {
            ByteArrayInputStream inputStream = new ByteArrayInputStream (profile);
            FileTransport.getInstance().
                uploadFile( FileTransport.PROTOCOL_TFTP,
                            profileFileName,  baseDirectoryURL.getHost(),
                            false, inputStream);
        } else {
            try {
                String outputFileName = baseDirectoryURL.getFile() + profileFileName;
                copyBytesToFile ( outputFileName, profile );
            }
            catch (java.io.IOException ex) {
                throw new RemoteException (ex.getMessage());
            }
        }
    }


    /**
     * writes a mini-profile (one which is 'coalesced' into a real profile) to a
     * work location.   This is used for device types that have only one real profile
     * but have settings which petain to users and devices.   Cicso 79xx is an example
     * of this.
     * @param profileFileName fully qualified file name of the mini-profile to be
     * created.
     * @param profile the actual payload for the mini-profile.
     * @throws IOException
     */
    private void writePartialProfile ( String profileFileName, byte[] profile )
        throws IOException {

        File basePath =
                new File ( m_partialProfilesDirectory + getBasePath ( profileFileName ) );

        if ( !basePath.exists() ) {
            basePath.mkdirs();
        }

        // make File object elligable for GC
        basePath = null;

        copyBytesToFile ( m_partialProfilesDirectory + profileFileName, profile );
    }


    private void copyBytesToFile ( String profileFileName, byte[] profile)
            throws RemoteException {

        BufferedInputStream in = null;
        BufferedOutputStream out = null;
        try {
            byte[] buffer = new byte[1024];

            in = new BufferedInputStream(new ByteArrayInputStream ( profile ));

            File outputDirectory = new File ( getBasePath( profileFileName ) );
            if ( !outputDirectory.exists() ) {
                outputDirectory.mkdirs();
            }

            out = new BufferedOutputStream (new FileOutputStream( profileFileName ));
            int numBytesRead;
            while ( (numBytesRead = in.read(buffer)) != -1 ) {
                out.write(buffer, 0, numBytesRead);
            }
            out.flush();
        } catch (java.io.IOException ex) {
            throw new RemoteException (ex.getMessage());
        } finally {
            if (in != null)
                try {in.close();} catch (IOException ex) {}
            if (out != null)
                try {out.close();} catch (IOException ex) {}
        }
    }


    private String getBasePath(String pathAndFile) {
        String basePath = pathAndFile.substring ( 0,
            pathAndFile.lastIndexOf( '/' ) + 1);
        return basePath;
    }

    /**
     * coalesceMiniProfiles takes all of the mini-profiles (user/device) for
     * a Cisco device
     * @param createProfileCommand command object that has been popped of the
     * work queue
     * @throws IOException
     */
    private void coalesceMiniProfiles ( CreateProfile createProfileCommand )
            throws IOException {

        String partialDir = ( m_partialProfilesDirectory +
                                createProfileCommand.getManufacturer() + "/"  +
                                createProfileCommand.getModel() + "/"  +
                                createProfileCommand.getMacAddress()  );

        File workingDir = new File ( partialDir );

        m_logger.debug ( "looking for partial profiles in: " + partialDir );

        if ( workingDir.exists() ) {
            File [] miniProfiles = workingDir.listFiles();
            int numberOfMinis = miniProfiles.length;

            if ( numberOfMinis > 0 ) {

                // only dealing with local tftp servers for now
                if ( m_tftpBaseURL.getProtocol().equals("file") ) {
                    BufferedWriter bw = null;

                    try {
                        // create real profile
                        // the following naming convention if for Cisco 79xx devices
                        String deviceSerialNumber = createProfileCommand.getMacAddress();

                        String profileName = calculateCiscoProfileName(deviceSerialNumber);

                        m_logger.debug( "creating profile: " + profileName );
                        bw = new BufferedWriter ( new FileWriter ( profileName ) );

                        for ( int i = 0; i < numberOfMinis; ++i ) {
                            BufferedReader br = null;
                            try {
                                m_logger.debug( "adding content from file: " +
                                        miniProfiles [ i ].getAbsoluteFile() + " to " +
                                            profileName );

                                br = new BufferedReader ( new FileReader ( miniProfiles [ i ] ) );

                                String buffer = null;

                                while ( ( buffer = br.readLine() ) != null ) {
                                    bw.write( buffer );
                                    bw.write ( "\n" );
                                }
                            }
                            finally {
                                br.close();
                            }
                        }
                    }
                    finally {
                        if ( bw != null ) {
                            bw.close();
                        }
                    } // finally

                } // if tftp URL is file

            } // if numer minis > 0
            else {
                m_logger.debug ( "no partial profiles in directory" );
            }

        } // if working dir exists
        else {
            m_logger.debug ( "working dir does not exist" );
        }

    }

    private String calculateCiscoProfileName(String deviceSerialNumber) {
        String profileName =
                m_tftpBaseURL.getFile() + "SIP" + deviceSerialNumber + ".cnf";
        return profileName;
    }

    /**
     * ThreadEntryCleanip interface implementatino callback - Can be used to do
     * thread cleanup after the run method genreates a checked exception
     * Implementing this method is optional
     */
    public void cleanupOnException (Throwable t) {
        m_logger.debug("PoolManager Exception callback handled");
    }
}