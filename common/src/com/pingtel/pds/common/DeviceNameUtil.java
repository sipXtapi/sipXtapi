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
        if ( vendor.equalsIgnoreCase("Pingtel") ) {
            switch (profileType) {
            case PDSDefinitions.PROF_TYPE_PHONE:
                return "x-xpressa-device";
            case PDSDefinitions.PROF_TYPE_USER:
                return "x-xpressa-user";
            case PDSDefinitions.PROF_TYPE_APPLICATION_REF:
                return "x-xpressa-apps";
            case PDSDefinitions.PROF_TYPE_UPGRADESCRIPT:
                return "x-xpressa-install";
            default:
                throw new IllegalArgumentException("No token for profile type: " +
                    profileType + " using " + vendor + "'s phone");
            }
        } else {
            throw new IllegalArgumentException(
                "Subscribe unsupported by: " + vendor + "'s phone");
        }
    }
}