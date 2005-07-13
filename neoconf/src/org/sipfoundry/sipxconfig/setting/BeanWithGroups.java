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
import java.util.Set;
import java.util.TreeSet;

/**
 * Common code for line and phone information.
 */
public class BeanWithGroups extends BeanWithSettings {

    private Set m_groups = new TreeSet();
    
    public Set getGroups() {
        return m_groups;
    }

    public void setGroups(Set settingSets) {
        m_groups = settingSets;        
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
        
        ValueStorage valueStorage = getValueStorage();
        if (valueStorage == null) {
            valueStorage = new ValueStorage();
            setValueStorage(valueStorage);
        }
        
        decorated = valueStorage.decorate(decorated);
        
        return decorated;    
    }
    
    public void addGroup(Group tag) {
        m_groups.add(tag);
    }
}
