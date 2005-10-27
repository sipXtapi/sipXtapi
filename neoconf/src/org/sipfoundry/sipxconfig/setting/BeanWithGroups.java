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

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import org.apache.commons.collections.Transformer;

/**
 * Common code for line, phone, and user information.
 */
public class BeanWithGroups extends BeanWithSettings {
    public static final String GROUPS_PROPERTY = "groups";

    private Set m_groups = new TreeSet();

    public Set getGroups() {
        return m_groups;
    }

    public void setGroups(Set settingSets) {
        m_groups = settingSets;
    }

    public List getGroupsAsList() {
        return new ArrayList(getGroups());
    }

    public void setGroupsAsList(List groups) {
        getGroups().clear();
        getGroups().addAll(groups);
    }

    public void addGroup(Group tag) {
        m_groups.add(tag);
    }

    public void removeGroup(Group tag) {
        m_groups.remove(tag);
    }

    protected void decorateSettings() {
        Setting settings = getSettings();
        Set groups = getGroups();
        if (groups != null) {
            Iterator i = groups.iterator();
            while (i.hasNext()) {
                Group group = (Group) i.next();
                group.decorate(settings);
            }
        }

        super.decorateSettings();
    }

    public static class AddTag implements Transformer {
        private Group m_tag;

        public AddTag(Group tag) {
            m_tag = tag;
        }

        public Object transform(Object input) {
            BeanWithGroups bean = (BeanWithGroups) input;
            bean.addGroup(m_tag);
            return bean;
        }
    }

    public static class RemoveTag implements Transformer {
        private Group m_tag;

        public RemoveTag(Group tag) {
            m_tag = tag;
        }

        public Object transform(Object input) {
            BeanWithGroups bean = (BeanWithGroups) input;
            bean.removeGroup(m_tag);
            return bean;
        }
    }
}
