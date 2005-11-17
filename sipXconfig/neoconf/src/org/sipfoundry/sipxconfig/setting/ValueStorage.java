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
        setting.acceptVisitor(new Decorator(setting, this));
    }

    public static class Decorator extends DecoratingVisitor {

        private Storage m_storage;

        public Decorator(Setting root, Storage storage) {
            super(root);
            m_storage = storage;
        }

        public Setting decorate(Setting setting) {
            return new SettingValue(m_storage, setting);
        }
    }
}
