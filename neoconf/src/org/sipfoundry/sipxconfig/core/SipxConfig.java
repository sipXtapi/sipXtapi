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

import java.util.Collections;
import java.util.Map;

/**
 * Context for entire sipXconfig framework. Holder for service layer bean
 * factories.
 */
public class SipxConfig {

    private Map m_plugins;
    
    private DeviceDao m_deviceDao;

    /**
     * @param devicePlugins The devicePlugins to set.
     */
    public void setDevicePlugins(Map plugins) {
        m_plugins = plugins;
    }

    /**
     * @return Returns the immutable collection of devicePlugins.
     */
    public Map getDevicePlugins() {
        return Collections.unmodifiableMap(m_plugins);
    }
    
    public DeviceDao getDeviceDao()
    {
        return m_deviceDao;
    }
    
    public void setDeviceDao(DeviceDao deviceDao) {
        m_deviceDao = deviceDao;
    }
      
}