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

import java.util.Iterator;
import java.util.Map;


/**
 * Settings meta information user overrides is stored in this collection
 */
public class MetaStorage extends AbstractStorage implements SettingVisitor {
    
    private static final long serialVersionUID = 1L;      

    public Map getMeta() {
        return getDelegate();
    }
    
    public void setMeta(Map delegate) {
        setDelegate(delegate);
    }

    public Setting decorate(Setting setting) {
        setting.acceptVisitor(this);        
        return setting;
    }

    public void visitSetting(Setting setting) {
        SettingMeta meta = (SettingMeta) get(setting.getPath());
        if (meta == null) {
            meta = new SettingMeta(this, setting);
        } else {
            meta.setDelegate(setting);
        }
        
        setting.getSettingGroup().addSetting(meta);
    }

    public void visitSettingGroup(SettingGroup group) {        
        Iterator i = group.getValues().iterator();
        while (i.hasNext()) {
            ((Setting) i.next()).acceptVisitor(this);
        }        
    }
}

