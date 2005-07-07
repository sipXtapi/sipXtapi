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
package org.sipfoundry.sipxconfig.setting;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.util.HashMap;
import java.util.Map;

/**
 * Adapts any interface to get/set to from settings
 */
public class SettingBeanAdapter implements InvocationHandler {
    
    private Class m_interface;
    
    private Setting m_setting;
    
    private Map m_mapping = new HashMap();
    
    public SettingBeanAdapter() {        
    }
    
    public SettingBeanAdapter(Class interfac) {
        m_interface = interfac;
    }
    
    public void setInterface(Class interface1) {
        m_interface = interface1;
    }
    
    public Class getInterface() {
        return m_interface;
    }
    
    public Object getImplementation() {
        Object impl = Proxy.newProxyInstance(m_interface.getClassLoader(),
                new Class[] { 
                    m_interface 
                },
                this);
        
        return impl; 
    }
    
    public void setSetting(Setting setting) {
        m_setting = setting;
    }
    
    public Setting getSetting() {
        return m_setting;
    }
    
    public Map getMapping() {
        return m_mapping;
    }
    
    public void setMapping(Map mapping) {
        m_mapping = mapping;
    }
    
    public void addMapping(String accessor, String settingPath) {
        m_mapping.put(accessor, settingPath);
    }

    public Object invoke(Object proxy_, Method method, Object[] args) throws Throwable {
        Object setting = null;
        String getter = getAccessor("get", method.getName());
        if (getter != null) {
            setting = getSetting(getter);
        }

        String setter = getAccessor("set", method.getName());
        if (setter != null) {
            setSetting(setter, (String) args[0]);
        }
        
        return setting;
    }
    
    protected void setSetting(String setter, String value) {
        String mapping = (String) m_mapping.get(setter);
        if (mapping != null) {
            Setting setting = m_setting.getSetting(mapping);
            if (setting != null) {
                setting.setValue(value);
            }
        }        
    }
    
    protected String getSetting(String getter) {
        String value = null;
        String mapping = (String) m_mapping.get(getter);
        if (mapping != null) {
            Setting setting = m_setting.getSetting(mapping);
            if (setting != null) {
                value = setting.getValue();
            }
        }
        return value;
    }
    
    static String getAccessor(String prefix, String methodName) {
        String name = null;
        if (methodName.startsWith(prefix)) {
            char c = Character.toLowerCase(methodName.charAt(prefix.length()));
            name = c + methodName.substring(prefix.length() + 1);            
        }
        
        return name;
    }
}
