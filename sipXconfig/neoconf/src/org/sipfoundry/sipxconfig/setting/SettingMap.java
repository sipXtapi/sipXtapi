/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Collection;
import java.util.LinkedHashMap;

public class SettingMap extends LinkedHashMap<String, Setting> {

    public void acceptVisitor(SettingVisitor visitor) {
        for (Setting setting : values()) {
            setting.acceptVisitor(visitor);
        }
    }
    
    public Setting addSetting(Setting setting) {
        Setting existingChild = put(setting.getName(), setting);
        if (existingChild != null) {
            Collection<Setting> grandChildren = existingChild.getValues();
            for (Setting grandChild : grandChildren) {
                setting.addSetting(grandChild);
            }
        }

        return setting;
    }
    

}
