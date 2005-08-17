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

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class BeanWithSettings extends BeanWithId {

    private Setting m_settings;

    private ValueStorage m_valueStorage;
       
    private Setting m_model;

    /**
     * @return undecorated model - direct representation of XML model description
     */
    public Setting getSettingModel() {
        return m_model;
    }
    
    public void setSettingModel(Setting model) {
        m_model = model;
    }

    /**
     * @return decorated model - use this to modify phone settings
     */
    public Setting getSettings() {
        if (m_settings == null) {
            m_settings = decorateSettings(getSettingModel());
        }

        return m_settings;
    }
    
    protected Setting decorateSettings(Setting settings) {
        if (settings == null) {
            // it's OK for a gateway not to provide settings
            return null;
        }
        
        if (m_valueStorage == null) {
            m_valueStorage = new ValueStorage();
        }

        Setting decorated = m_valueStorage.decorate(settings);
        
        // TODO do we need setDefaults(); here        

        return decorated;
    }
    
    public void setValueStorage(ValueStorage valueStorage) {
        m_valueStorage = valueStorage;
    }

    public ValueStorage getValueStorage() {
        return m_valueStorage;
    }
}
