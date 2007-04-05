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

public class MulticastSettingValueHandler implements SettingValueHandler {
    private Collection< ? extends SettingValueHandler> m_resolvers;

    public MulticastSettingValueHandler(Collection< ? extends SettingValueHandler> resolvers) {
        m_resolvers = resolvers;
    }

    public SettingValue getSettingValue(Setting setting) {
        SettingValue value = null;
        for (SettingValueHandler resolver : m_resolvers) {
            SettingValue sv = resolver.getSettingValue(setting);
            if (sv != null) {
                value = sv;
            }
        }

        return value;
    }
}
