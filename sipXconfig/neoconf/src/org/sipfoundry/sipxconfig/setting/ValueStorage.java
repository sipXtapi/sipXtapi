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
public class ValueStorage extends AbstractStorage implements Storage, SettingVisitor {

    public Map getValues() {
        return getDelegate();
    }

    public void setValues(Map delegate) {
        setDelegate(delegate);
    }

    public Setting decorate(Setting setting) {
        setting.acceptVisitor(this);
        return setting;
    }

    public void visitSetting(Setting setting) {
        Setting decorated = new SettingValue(this, setting);
        setting.getParent().addSetting(decorated);
    }

    public void visitSettingGroup(Setting group_) {
        // nothing to do for group
    }
}
