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
package org.sipfoundry.sipxconfig.vendor.cisco;

import org.sipfoundry.sipxconfig.core.DevicePlugin;

/**
 * Support for Cisco 7960, 7940, 7905 and 7912 SIP phones.
 */
public class CiscoDevicePlugin implements DevicePlugin {
    
    private String m_id;
    
    public static final String MODEL_7960 = "cisco7960";

    public static final String MODEL_7940 = "cisco7940";
    
    public static final String MODEL_7905 = "cisco7905";
    
    public static final String MODEL_7912 = "cisco7912";
      
    /**
     * XML filename that describes a particular model's definitions
     * 
     * @param model
     * @return filepath to xml file
     */
    public String getDefinitions()
    {
        return new StringBuffer().append('/').append(m_id)
        	.append("definitions.xml").toString();
    }
    
    public String getPluginId()
    {
        return m_id;
    }

    public void setPluginId(String id) {
        m_id = id;
    }    
    
    public int getProfileCount() {
        return 1;
    }

    public String getProfileFileName(int profileIndex, String macAddress) {
        // Goes into TFTP Root dir, no vendor prefix
        return macAddress + ".lnk";
    }
}

