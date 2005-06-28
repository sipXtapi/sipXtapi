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

import java.util.Iterator;
import java.util.Set;
import java.util.TreeSet;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingVisitor;
import org.sipfoundry.sipxconfig.setting.ValueStorage;

/**
 * Common code for line and phone information.
 */
public class AbstractData extends BeanWithId {

    private Set m_explicitGroups = new TreeSet();
    
    private ValueStorage m_valueStorage = new ValueStorage();

    public Set getGroups() {
        return m_explicitGroups;
    }

    public void setGroups(Set settingSets) {
        m_explicitGroups = settingSets;        
    }
    
    public Setting decorate(Group implicitRootTag, Setting settings) {
        return decorate(implicitRootTag, getGroups(), settings);
    }
    
    protected Setting decorate(Group implicitRootTag, Set groups, Setting settings) {
        Setting decorated = implicitRootTag.decorate(settings);
        
        if (groups != null) {
            Iterator i = groups.iterator();
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
    
    public void addGroup(Group tag) {
        m_explicitGroups.add(tag);
    }

    public ValueStorage getValueStorage() {
        return m_valueStorage;
    }    

    public void setValueStorage(ValueStorage valueStorage) {
        m_valueStorage = valueStorage;
    }   
}
