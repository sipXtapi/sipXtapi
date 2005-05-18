/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import org.apache.commons.lang.enum.Enum;

public interface SipxProcessContext {
    public static class Process extends Enum {
        public static final Process REGISTRAR = new Process("SIPRegistrar");
        public static final Process AUTH_PROXY = new Process("SIPAuthProxy");
        public static final Process STATUS = new Process("SIPStatus");
        public static final Process PROXY = new Process("SIPProxy");
        public static final Process MEDIA_SERVER = new Process("MediaServer");
        public static final Process PARK_SERVER = new Process("ParkServer");        
        
        public Process(String name) {
            super(name);
        }        
    };
    
    void restart(Process process);
}
