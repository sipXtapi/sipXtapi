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

public class SettingModel extends SettingDecorator implements SettingValueHandler {

    private String m_resource;

    private String m_root;
    
    public SettingModel() {
        super();
    }
    
    public String getRoot() {
        return m_root;
    }
    
    /**
     * Specify which setting the model start with, Only used if a model doesn't start at the
     * top-most setting
     * 
     * @param root
     */
    public void setRoot(String root) {
        m_root = root;
    }

    /**
     * File path relative to ModelFilesContext's system root directory
     * 
     * @param resource
     */
    public void setResource(String resource) {
        m_resource = resource;
    }

    public void setModelFilesContext(ModelFilesContext modelFileContext) {
        Setting model = modelFileContext.loadModelFile(m_resource);
        if (m_root != null) {
            model = model.getSetting(m_root);
        }
        setDelegate(model);
    }

    // FIXME: temporary - SettingModel should not be a decorator
    public Setting getRealSetting() {
        return getDelegate();
    }

    public SettingValue2 getSettingValue(Setting setting) {
        SettingValue2 value = null;
        Setting s = getSetting(setting.getPath());
        if (s != null) {
            value = new SettingValueImpl(s.getValue());
        }
        
        return value;
    }
}
