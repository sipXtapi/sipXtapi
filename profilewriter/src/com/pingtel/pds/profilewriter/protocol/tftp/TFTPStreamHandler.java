/*
 * $Id: //depot/OPENDEV/sipXconfig/profilewriter/src/com/pingtel/pds/profilewriter/protocol/tftp/TFTPStreamHandler.java#4 $
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

import java.net.URL;
import java.net.URLStreamHandler;
import java.net.URLConnection;
import java.io.IOException;


/**
 * <p>Title: TftpHandler</p>
 * <p>Description: TftpProtocolHandler</p>
 * <p>Copyright: Copyright (c) 2001</p>
 * <p>Company: Pingtel Corp.</p> 
 * @author John P. Coffey
 * @version 1.0
 */

public class TFTPStreamHandler extends URLStreamHandler {
    public URLConnection openConnection(URL url) throws IOException {
        return new TFTPURLConnection(url);
    }

    /** Not really required but here is where to customize it */
    public void parseURL(URL u, String spec, int start, int limit) {
        super.parseURL(u, spec, start, limit);
    }
}

