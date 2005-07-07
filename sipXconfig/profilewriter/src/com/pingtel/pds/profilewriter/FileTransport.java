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

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import org.apache.log4j.Category;

import com.oroinc.net.ftp.FTPClient;
import com.oroinc.net.ftp.FTPReply;
import com.oroinc.net.tftp.TFTP;
import com.oroinc.net.tftp.TFTPClient;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company:
 * @author
 * @version 1.0
 */

public class FileTransport {
    /** Enumerate the protocols - FTP */
    public static final String PROTOCOL_FTP = "FTP";
    /** Enumerate the protocols - TFTP */
    public static final String PROTOCOL_TFTP = "TFTP";

    private static Category m_logger;

    static {
        m_logger = Category.getInstance( "pds" );
    }

    /** Singleton */
    private static FileTransport g_instance = null;


    /** Constructor */
    private FileTransport() {}

    /** Singleton Accessor */
    public static FileTransport getInstance() {
        if (g_instance == null) {
            g_instance = new FileTransport ();
        }
        return g_instance;
    }

    /**
     * Sends a file to a remote server via TFTP or FTP (todo).
     *
     * @param protocol FTP or TFTP as supported by NetComponents
     * @param hostname the name of the remote server
     * @param isBinary indicates the transfer mode
     * @param input input file stream to send to server
     */
    public void uploadFile( final String protocol,
                            String fileName,
                            String hostName,
                            boolean isBinary,
                            InputStream input ) throws IOException {

        m_logger.debug( "uploading file: filename " + fileName +
                        "host name " + hostName );

        // ensure we support the protocol
        if ( protocol.equalsIgnoreCase( PROTOCOL_TFTP ) ) {
            // create client instance
            m_logger.debug( "creating TFTP client" );
            TFTPClient tftp = new TFTPClient();

            // We want to timeout if a response takes longer than 60 seconds
            tftp.setDefaultTimeout(60000);

            // Open local UDP socket (port 67?)
            tftp.open();
            m_logger.debug( "opened TFTP client" );

            int transferMode = (isBinary == true)? TFTP.BINARY_MODE : TFTP.ASCII_MODE;

            //if (fileName.startsWith("/")) fileName = fileName.substring(1);
            // Upload the file (Note that the remove filename is the same as the local one)
            // @todo experiment with a directory hierarchy here also
            // Try to send local file via TFTP
            tftp.sendFile(fileName, transferMode, input, hostName);
            m_logger.debug( "send file " + fileName );

            // Close local socket (note pool manager may be able to keep this open)
            tftp.close();
            m_logger.debug ( "closed TFTP client" );
        } else {
            throw new UnsupportedOperationException("I/O Protocol unsupported: " + protocol);
        }
    }

    /**
     * Download a file from a remote TFTP or FTP (todo) server.
     *
     * @param protocol FTP or TFTP as supported by NetComponents
     * @param hostname the name of the remote server
     * @param isBinary indicates the transfer mode
     * @param input input file stream to send to server
     */
    public void downloadFile( final String protocol,
                              String fileName,
                              String hostName,
                              boolean isBinary,
                              OutputStream output ) throws IOException {
        // ensure we support the protocol
        m_logger.debug( "downloading file: " + fileName + " host " + hostName +
                        "protocol " + protocol );
        if ( protocol.equalsIgnoreCase( PROTOCOL_TFTP ) ) {
            // create client instance
            m_logger.debug( "creating TFTP client" );
            TFTPClient tftp = new TFTPClient();

            // We want to timeout if a response takes longer than 60 seconds
            tftp.setDefaultTimeout(60000);

            // Open local UDP socket (port 67?)
            tftp.open();
            m_logger.debug( "opened TFTP client" );

            int transferMode = (isBinary == true)? TFTP.BINARY_MODE : TFTP.ASCII_MODE;

            tftp.receiveFile(fileName, transferMode, output, hostName);
            m_logger.debug( "received file " + fileName + " from " + hostName );
            // Close local socket (note pool manager may be able to keep this open)
            tftp.close();
            m_logger.debug ( "closed TFTP client" );
        } else if (protocol.equalsIgnoreCase( PROTOCOL_FTP )) {
            m_logger.debug( "creating FTP client" );
            FTPClient ftp = new FTPClient();
            try {
                // Show output to console
                // ftp.addProtocolCommandListener(
                //    new PrintCommandListener( new PrintWriter(System.out) ) );

                ftp.connect( hostName );
                m_logger.debug( "connected to FTP server " + hostName );
                // After connection attempt, much check reply code for success.
                if(FTPReply.isPositiveCompletion( ftp.getReplyCode()) ) {
                    if( ftp.login( "anonymous", "anonymous@pingtel.com" )) {
                    	InputStream input = new FileInputStream("/test.dat");
                    	ftp.storeFile(fileName, input);
                    }
                    ftp.logout();
                    m_logger.debug( "logged off FTP server " + hostName );
                } else {
                    ftp.disconnect();
                    m_logger.debug( "disconnected from FTP server " + hostName );
                    throw new IOException ("FTP server refused connection.");
                }
            } finally {
                if( ftp.isConnected() ) {
                    try {
                        ftp.disconnect();
                        m_logger.debug( "disconnected from FTP server " + hostName );
                    }
                    catch(IOException ex) {}
                }
            }
        } else  {
            throw new UnsupportedOperationException("I/O Protocol unsupported: " + protocol);
        }
    }
}
