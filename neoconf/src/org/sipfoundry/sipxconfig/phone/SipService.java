/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.io.IOException;
import java.text.DateFormat;

/**
 * Sip utility service functions complete w/server current configuration
 */
public interface SipService {
    
    public String getServerVia();
    
    public String getServerUri();
    
    public void send(String to, int port, String sipMsg) throws IOException;
    
    public String getCurrentDate();
    
    /**
     * To format your own date in SIP RFC format
     * Example:  Tue, 15 Nov 1994 08:12:31 GMT 
     */
    public DateFormat getDateFormat();
    
    public String generateCallId();
    
}
