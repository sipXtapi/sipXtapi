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
package org.sipfoundry.sipxconfig.vendor.pingtel;

import java.io.File;

import org.sipfoundry.sipxconfig.core.DevicePlugin;

/**
 * Support for SIP Soft Phone and legacy xpressa
 */
public class SipxPhoneDevicePlugin implements DevicePlugin {
    
    private String m_id;
    
    public static final String HARDPHONE = "xpressa_strongarm_vxworks";
    
    public static final String SOFTPHONE = "ixpressa_x86_win32";

    private static final int PROF_TYPE_PHONE = 0;
 
    private static final int PROF_TYPE_USER = 1;
    
    private static final int PROF_TYPE_APPLICATION_REF = 2;

    private static final int PROF_TYPE_UPGRADESCRIPT = 3;
      
    /**
     * XML filename that describes a particular model's definitions
     * 
     * @param model
     * @return filepath to xml file
     */
    public String getDefinitions()
    {
        return new StringBuffer().append('/').append(m_id)
        	.append("-definitions.xml").toString();
    }
    
    public String getPluginId()
    {
        return m_id;
    }

    public void setPluginId(String pluginId) {
        m_id = pluginId;
    }

    public int getProfileCount() {
        return PROF_TYPE_UPGRADESCRIPT + 1;
    }

    public String getProfileFileName(int profileIndex, String macAddress) {
        String profileName = null;
        switch (profileIndex) {
            case PROF_TYPE_PHONE:
                profileName = "pingtel/" + m_id + "/" + macAddress + "/pinger-config";
                break;
            case PROF_TYPE_USER:
                profileName = "pingtel/" + m_id + "/" + macAddress + "/user-config";
                break;
            case PROF_TYPE_APPLICATION_REF:
                profileName = "pingtel/" + m_id + "/" + macAddress + "/app-config";
                break;
            case PROF_TYPE_UPGRADESCRIPT:
                profileName = "pingtel/" + m_id + "/" + macAddress + "/install-script";
                break;
        }

        return profileName.toLowerCase();
    }
}

