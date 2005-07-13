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
    
    /**
     * @return undecorated model - direct representation of XML model description
     */
    public Setting getSettingModel() {
        return null;
    }

    /**
     * @return decorated model - use this to modify phone settings
     */
    public Setting getSettings() {
        if (m_settings == null) {
            Setting settings = getSettingModel();
            if (settings == null) {
                // it's OK for a gateway not to provide settings
                return null;
            }
            if (m_valueStorage == null) {
                m_valueStorage = new ValueStorage();
            }

            m_settings = m_valueStorage.decorate(settings);
            // TODO do we need setDefaults(); here
        }

        return m_settings;
    }

    public void setValueStorage(ValueStorage valueStorage) {
        m_valueStorage = valueStorage;
    }

    public ValueStorage getValueStorage() {
        return m_valueStorage;
    }


}
