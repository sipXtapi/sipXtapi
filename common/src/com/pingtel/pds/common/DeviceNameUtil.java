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

package com.pingtel.pds.common;

import com.pingtel.pds.sipxfacade.SipxConfigFacadeFactory;

public class DeviceNameUtil {
    private static DeviceNameUtil g_instance;
    private DeviceNameUtil() {}
    public static DeviceNameUtil getInstance() {
        if (g_instance == null)
            g_instance = new DeviceNameUtil ();
        return g_instance;
    }

    /** Device Specific Profile file offset information stored here */
    public String getDeviceProfileName ( int profileType,
                                         String vendor,
                                         String model,
                                         String macAddress ) {
        
        return SipxConfigFacadeFactory.getFacade().getDeviceProfileName(
                profileType, vendor, model, macAddress);
    }

    public String getDeviceProfileToken(int profileType, String vendor) {

        return SipxConfigFacadeFactory.getFacade().getDeviceProfileToken(
                profileType, vendor);
        
    }
}