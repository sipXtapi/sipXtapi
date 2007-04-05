/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Collection;

public class BeanWithGroupsModel extends BeanWithSettingsModel {
    private MulticastSettingValueHandler m_groupsHandler;

    public BeanWithGroupsModel(BeanWithGroups bean) {
        super(bean);
    }

    public void setGroups(Collection< ? extends SettingValueHandler> groups) {
        m_groupsHandler = new MulticastSettingValueHandler(groups);
    }

    @Override
    protected SettingValue getDefault(Setting setting) {
        SettingValue value = null;

        if (m_groupsHandler != null) {
            value = m_groupsHandler.getSettingValue(setting);
        }

        if (value == null) {
            value = getDefaultsHandler().getSettingValue(setting);
        }

        return value;
    }
}
