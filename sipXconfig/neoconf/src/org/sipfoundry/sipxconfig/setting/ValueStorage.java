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

import java.util.Map;

/**
 * Basic layer of settings decoration that captures just setting values.
 */
public class ValueStorage extends AbstractStorage implements Storage {

    public Map getValues() {
        return getDelegate();
    }

    public void setValues(Map delegate) {
        setDelegate(delegate);
    }

    public void decorate(Setting setting) {        
        setting.acceptVisitor(new Decorator(setting));
    }

    
    class Decorator implements SettingVisitor {
        
        private Setting m_root;
        
        Decorator(Setting root) {
            m_root = root;
        }
        
        public void visitSetting(Setting setting) {
            Setting decorated = new SettingValue(ValueStorage.this, setting);
            String parentPath = setting.getParentPath();
            if (parentPath == null) {
                m_root.addSetting(decorated);
            } else {
                SettingUtil.getSettingFromRoot(m_root, parentPath).addSetting(decorated);
            }
        }

        public void visitSettingGroup(Setting group_) {
            // nothing to do for group
        }
    }
}
