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

import java.util.HashMap;
import java.util.Map;

import com.pingtel.pds.sipxfacade.SipxConfigFacade;
import org.sipfoundry.sipxconfig.vendor.cisco.CiscoDevicePlugin;
import org.sipfoundry.sipxconfig.vendor.pingtel.SipxPhoneDevicePlugin;


/**
 * Called from code running on profilegenerator, profilewriter and 
 * profilepublisher
 */
public class SipxConfigFacadeImpl implements SipxConfigFacade {
    
    private SipxConfig m_sipx;
    
    private Map m_pluginIdMap;
    
    public SipxConfigFacadeImpl() {
        m_pluginIdMap = new HashMap();

        // Map vendor and models to plugin id.  When only vendor
        // is avail, then given plugin acts as spokesman for all
        // phones for that vendor
        m_pluginIdMap.put("7960", CiscoDevicePlugin.MODEL_7960);
        m_pluginIdMap.put("7940", CiscoDevicePlugin.MODEL_7940);
        m_pluginIdMap.put("Cisco", CiscoDevicePlugin.MODEL_7960);
        m_pluginIdMap.put("Pingtel", SipxPhoneDevicePlugin.SOFTPHONE);        
    }
    
    
    public SipxConfig getSipxConfig() {
        return m_sipx;
    }
    
    public void setSipxConfig(SipxConfig sipx) {
        m_sipx = sipx;
    }    

    public String getDeviceProfileName(int profileType, String vendor, 
            String model, String macAddress) {

        DevicePlugin plugin = getPluginId(vendor);
        if (profileType > plugin.getProfileCount()) {
            throw new IllegalArgumentException("Profile Type: "
                    + profileType + " not supported by " + vendor
                    + " model " + model);
        }
        
        return plugin.getProfileFileName(profileType, macAddress);
    }
    
    public String getDeviceProfileToken(int profileType, String vendor) {
        DevicePlugin plugin = getPluginId(vendor);
        if (profileType > plugin.getProfileCount()) {
            throw new IllegalArgumentException("Profile Token: "
                    + profileType + " not supported by vendor " + vendor);
        }
        
        return plugin.getProfileSubscribeToken(profileType);        
    }
    
    public String getDeviceNotifyUrl(int deviceId, int profileType, String model)
    {
        return "TODO deviceId=" + deviceId + ", profileType=" + profileType
                + "model=" + model;
    }

    public int getDeviceSequenceNumber(int deviceId, int profileType, String model)
    {
        System.out.println("TODO x deviceId=" + deviceId + ", x profileType=" 
                + profileType + "x model=" + model);
        return -1;        
    }
    
    DevicePlugin getPluginId(String modelOrVersion) {
        String pluginId = modelOrVersion;

        String device = (String) m_pluginIdMap.get(modelOrVersion); 
        if (device != null) {
            pluginId = device;
        }
        
        DevicePlugin plugin = m_sipx.getDevicePlugin(pluginId);
        if (plugin == null) {
            throw new IllegalArgumentException("No such plugin " + device);
        }

        return plugin;
    }
    
}
