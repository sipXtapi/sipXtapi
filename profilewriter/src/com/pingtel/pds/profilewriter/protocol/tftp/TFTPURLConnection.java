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

package com.pingtel.pds.profilewriter.protocol.tftp;

import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;

import java.net.URL;
import sun.net.www.URLConnection;
import sun.net.www.MessageHeader;

import com.oroinc.net.ftp.FTP;
import com.oroinc.net.ftp.FTPReply;
import com.oroinc.net.ftp.FTPClient;
import com.oroinc.net.tftp.TFTP;
import com.oroinc.net.tftp.TFTPClient;

/**
 * <p>Title: TftpHandler</p>
 * <p>Description: TFTPURLConnection</p>
 * <p>Copyright: Copyright (c) 2001</p>
 * <p>Company: Pingtel Corp.</p>
 * @author John P. Coffey
 * @version 1.0
 */

public class TFTPURLConnection extends URLConnection {
    String m_host;
    int m_port = 69;
    InputStream m_inputStream = null;
    TFTPClient m_tftpClient = null;

    public TFTPURLConnection(URL url) {
        super(url);
        m_host = url.getHost();
        m_port = url.getPort();
    }

    public void connect() throws java.io.IOException {
        // Connect to the TFTP Server
    	if ( !connected ) {
            // create client instance
            m_tftpClient = new TFTPClient();

            // We want to timeout if a response takes longer than 60 seconds
            m_tftpClient.setDefaultTimeout(60000);

            // Remember we are connected, (this member is in the superclass)
            connected = true;
    	}
    }

    /**
     * Returns an input stream, that streams the bytes read from the TFTP file
     * this method is a little inefficient in that the receive method gets the entire
     * file into a ByteArrayOutputStream (I think) rather than doing it in chunks
     */
    public InputStream getInputStream() throws IOException {
	    if ( !connected ) {
	        connect();
    	}

        // Already opened with an open inputstream so use it
    	if ( m_inputStream != null ) {
	        return m_inputStream;
    	}

        // this is a temporary buffer object used to read the
        // raw data from the remote file
        ByteArrayOutputStream output = new ByteArrayOutputStream();

        // Do our best to guess the type (binary or ascii) of the file
        // type could be null so default to ascii? (hmm is this correct>)
	    String type = guessContentTypeFromName( url.getFile() );

        // Asume ASCII
        int mode = TFTP.ASCII_MODE;

        if ( type.indexOf("binary") >= 0 ) {
            mode = TFTP.BINARY_MODE;
        }

        // Open local connection to the TFTP server (port 69?)
        try {
            m_tftpClient.open();

            // Read the entire file into an output stream
            // and then close the TFTP connection as it locks the file
            m_tftpClient.receiveFile( url.getFile(),
                                      mode,
                                      output,
                                      url.getHost() );

            // The input stream contains all of the remote data (buffered)
            // this is inefficient but we are not using very much data for
            // the remote xml files (200 bytes or so)
            m_inputStream = new ByteArrayInputStream( output.toByteArray() );

            // Only Attempt to read the stream bytes if the
            // Input stream supports random access
            if ( type == null && m_inputStream.markSupported() ) {
                type = guessContentTypeFromStream( m_inputStream );
            }

            // Store message headers in the Connection
            MessageHeader msgh = new MessageHeader();

            if (type != null) {
                msgh.add("content-type", type);
            }
            setProperties( msgh );
        } finally {
            // The TFTP connection is no longer required
            m_tftpClient.close();
        }

        // return the open buffered input stream
    	return m_inputStream;
    }

    /**
     * Over ride super class to ensure we clean up open connections
     * and resources in netcomponents.  This class calls the superclass to
     * null the URL and then it closes the streams and tftpClient connection
     */
    public void close() {
        // call the super class first
        super.close();

        if (connected) {
            // Close local socket (note pool manager may be able to keep this open)
            if (m_inputStream != null) {
                try { m_inputStream.close(); } catch (IOException ex) {}
                m_inputStream = null;
            }

            // close open connection to tftp server
            if (m_tftpClient != null) {
                m_tftpClient.close();
                m_tftpClient = null;
            }
            connected = false;
        }
    }
}