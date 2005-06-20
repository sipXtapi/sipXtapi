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
package org.sipfoundry.sipxconfig.phone;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingVisitor;
import org.sipfoundry.sipxconfig.setting.Tag;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

/**
 * Common code for line and phone information.
 */
public class AbstractData extends BeanWithId {

    private List m_explicitTags;
    
    private ValueStorage m_valueStorage = new ValueStorage();

    public List getTags() {
        return m_explicitTags;
    }

    public void setTags(List settingSets) {        
        m_explicitTags = settingSets;
    }
    
    protected Setting decorate(Tag implicitRootTag, Setting settings) {
        Setting decorated = implicitRootTag.decorate(settings);
        
        if (m_explicitTags != null) {
            Iterator i = m_explicitTags.iterator();
            while (i.hasNext()) {
                SettingVisitor visitor = (SettingVisitor) i.next(); 
                decorated.acceptVisitor(visitor);
            }
        }
        
        if (m_valueStorage == null) {
            m_valueStorage = new ValueStorage();
        }
        
        decorated = m_valueStorage.decorate(decorated);
        
        return decorated;
    }
    
    public void addTag(Tag tag) {
        if (m_explicitTags == null) {
            m_explicitTags = new ArrayList();
        }
        m_explicitTags.add(tag);
    }

    public ValueStorage getValueStorage() {
        return m_valueStorage;
    }    

    public void setValueStorage(ValueStorage valueStorage) {
        m_valueStorage = valueStorage;
    }   
}
