/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.setting.Setting;

public class UnmanagedService extends ConfiguredService {
    public static final String BEAN_ID = "unmanagedService";
    
    public static final ServiceDescriptor NTP = new ServiceDescriptor(BEAN_ID, "ntpService", "NTP");
    
    public UnmanagedService() {        
        super(BEAN_ID);
    }
    
    public final ServiceDescriptor ntp() {
        return NTP;
    }

    @Override
    protected Setting loadSettings() {
        return null;
    }
}
