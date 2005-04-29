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
        if ( model.equalsIgnoreCase(PDSDefinitions.MODEL_HARDPHONE_CISCO_7960 ) ) {
            switch ( profileType ) {
            case PDSDefinitions.PROF_TYPE_PHONE:
                return macAddress + ".lnk";
            default:
                throw new IllegalArgumentException("Profile Type: " +
                    profileType + " not supported for by: " + model);
            }
        } else {
            // Normal Pingtel or similar phone, must retrurn lowercase filenames
            String profileName = null;
            switch ( profileType ) {
                case PDSDefinitions.PROF_TYPE_PHONE:
                    profileName = vendor + "/" + model + "/" + macAddress + "/" + "pinger-config";
                    break;
                case PDSDefinitions.PROF_TYPE_USER:
                    profileName = vendor + "/" + model + "/" + macAddress + "/" + "user-config";
                    break;
                case PDSDefinitions.PROF_TYPE_UPGRADESCRIPT:
                    profileName = vendor + "/" + model + "/" + macAddress + "/" + "install-script";
                    break;
                default:
                    throw new IllegalArgumentException("Profile Type: " +
                        profileType + " not supported for by: " + model);
            }
            return profileName.toLowerCase();
        }
    }

    public String getDeviceProfileToken(int profileType, String vendor) {
        if ( vendor.equalsIgnoreCase("Pingtel") ) {
            switch (profileType) {
            case PDSDefinitions.PROF_TYPE_PHONE:
                return "x-xpressa-device";
            case PDSDefinitions.PROF_TYPE_USER:
                return "x-xpressa-user";
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
