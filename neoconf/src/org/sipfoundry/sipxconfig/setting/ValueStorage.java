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
 * This represents a link table between several business objects and SettingValues
 * It serves no business purpose but gets around a hibernate and RDBMS mismatch
 */
public class ValueStorage extends AbstractStorage implements SettingVisitor {
    
    private static final long serialVersionUID = 1L;
    
    private Folder m_metaStorage;
    
    public Map getValues() {
        return getDelegate();
    }
    
    public void setValues(Map delegate) {
        setDelegate(delegate);
    }
    
    public Folder getMetaStorage() {
        return m_metaStorage;
    }
    
    public void setMetaStorage(Folder metaStorage) {
        m_metaStorage = metaStorage;
    }
    
    public Setting decorate(Setting setting) {
        setting.acceptVisitor(this);        
        return setting;
    }

    public void visitSetting(Setting setting) {
        Setting decorated = new SettingValue(this, setting);
        setting.getSettingGroup().addSetting(decorated);
    }

    public void visitSettingGroup(SettingGroup group) {        
        Iterator i = group.getValues().iterator();
        while (i.hasNext()) {
            ((Setting) i.next()).acceptVisitor(this);
        }        
    }
}

