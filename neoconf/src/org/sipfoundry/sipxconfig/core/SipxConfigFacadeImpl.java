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
package org.sipfoundry.sipxconfig.core;

import com.pingtel.pds.sipxfacade.SipxConfigFacade;


/**
 * Called from code running on profilegenerator, profilewriter and 
 * profilepublisher
 */
public class SipxConfigFacadeImpl implements SipxConfigFacade {
    
    private SipxConfig m_sipx;
    
    public SipxConfig getSipxConfig() {
        return m_sipx;
    }
    
    public void setSipxConfig(SipxConfig sipx) {
        m_sipx = sipx;
    }    

    public String getDeviceProfileName(int profileType, String vendor, 
            String model, String macAddress) {

        // Convert vendor model pair to plugin id key 
        String device = vendor + " - " + model; 
        
        DevicePlugin plugin = m_sipx.getDevicePlugin(device);
        if (plugin == null)
        {
            throw new IllegalArgumentException("No such plugin " + device);        
        }
        
        if (profileType > plugin.getProfileCount())
        {
            throw new IllegalArgumentException("Profile Type: " +
                    profileType + " not supported for by: " + model);
        }
        
        return plugin.getProfileFileName(profileType, macAddress);
    }
}
