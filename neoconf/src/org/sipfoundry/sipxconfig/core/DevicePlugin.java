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

/**
 * Implement this to add support for new devices to the system
 */
public interface DevicePlugin {

    /**
     * File linking properties to this device
     * 
     * @return Absolute file path to xml file
     */
    public String getDefinitions();
        
    /**
     * Identifying string unique across all plugins 
     * 
     * @return e.g. Cisco - 7960
     */
    public String getPluginId();

    /**
     * How many files make-up the list of files this writer creates
     * 
     * @return zero or greater
     */
    public int getProfileCount();
    
    /**
     * What is the filename of the Nth profile
     * 
     * @param profileIndex
     * @param macAddress
     * 
     * @return No path, just filename
     */
    public String getProfileFileName(int profileIndex, String macAddress);
    
    /**
     * Associate a profile w/a token to distinguish them as part of
     * SIP's SUBSCRIBE messages
     * 
     * @param profileIndex
     * @return e.g. "device-settings" or "user-settings"
     */
    public String getProfileSubscribeToken(int profileIndex);
}
