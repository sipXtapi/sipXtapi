/*
 * $Id: //depot/OPENDEV/sipXconfig/profilewriter/src/com/pingtel/pds/profilewriter/protocol/tftp/TFTPStreamHandlerFactory.java#4 $
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

import java.net.URLStreamHandler;
import java.net.URLStreamHandlerFactory;

/**
 * <p>Title: TftpHandler</p>
 * <p>Description: TftpProtocolHandler</p>
 * <p>Copyright: Copyright (c) 2001</p>
 * <p>Company: Pingtel Corp.</p>
 * @author John P. Coffey
 * @version 1.0
 */

public class TFTPStreamHandlerFactory implements URLStreamHandlerFactory {
    /**
     * There can only be one factory associated with UTL's this one knoew about tftp
     */
    public TFTPStreamHandlerFactory() {}

    /**
     * Creates a TFTP Stream Handler (if the protocol indicated TFTP)
     * @param protocol this should be set to "tftp"
     */
    public URLStreamHandler createURLStreamHandler( String protocol ) {
        if( protocol.equals("tftp")){
            return new TFTPStreamHandler();
        }
        return null;
    }
}
