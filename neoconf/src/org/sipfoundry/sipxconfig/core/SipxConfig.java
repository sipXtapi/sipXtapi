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
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public class SipxConfig {

    private Map m_plugins;

    private DeviceDao m_deviceDao;

    /**
     * @param devicePlugins The devicePlugins to set.
     */
    public void setDevicePlugins(Set plugins) {
        m_plugins = new HashMap(plugins.size());
        Iterator i = plugins.iterator();
        while (i.hasNext()) {
            DevicePlugin plugin = (DevicePlugin) i.next();
            m_plugins.put(plugin.getPluginId(), plugin);
        }
    }
    
    public int getPluginCount() {
        return m_plugins.size();
    }
    
    public Set getPluginIds() {
        return m_plugins.keySet();
    }

    /**
     * @return null if no plugin by that id exists
     */
    public DevicePlugin getDevicePlugin(String pluginId) {
        return (DevicePlugin) m_plugins.get(pluginId);
    }

    public DeviceDao getDeviceDao() {
        return m_deviceDao;
    }

    public void setDeviceDao(DeviceDao deviceDao) {
        m_deviceDao = deviceDao;
    }

}
