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
package com.pingtel.pds.sipxfacade;

import com.pingtel.pds.common.PDSDefinitions;

/**
 * Legacy code that is being replaced by neoconf package
 */
public class SipxConfigFacadeImpl implements SipxConfigFacade {

    /* (non-Javadoc)
     * @see com.pingtel.pds.sipxfacade.SipxConfigFacade#getDeviceProfleName(java.lang.String, java.lang.String, java.lang.String, java.lang.String)
     */
    public String getDeviceProfleName(int profileType, String vendor, String model,
            String macAddress) {
        
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
                case PDSDefinitions.PROF_TYPE_APPLICATION_REF:
                    profileName = vendor + "/" + model + "/" + macAddress + "/" + "app-config";
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

}
