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

import java.io.Serializable;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedHashMap;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.context.MessageSource;

/**
 * Meta information about a group of settings, can contain nested SettingModels. Order is
 * preserved
 */
public class SettingSet extends AbstractSetting implements Cloneable, Serializable {

    public static final Log LOG = LogFactory.getLog(SettingSet.class);

    private LinkedHashMap<String, Setting> m_children = new LinkedHashMap<String, Setting>();

    private MessageSource m_messageSource;

    /**
     * Root setting group and bean access only
     */
    public SettingSet() {
    }

    public SettingSet(String name) {
        super(name);
    }

    /**
     * includes deep copy of all childen
     */
    public Setting copy() {
        SettingSet copy = (SettingSet) shallowCopy();
        for (Setting child : m_children.values()) {
            copy.addSetting(child.copy());
        }
        return copy;
    }

    protected Setting shallowCopy() {
        SettingSet copy = (SettingSet) super.copy();
        copy.m_children = new LinkedHashMap<String, Setting>();
        return copy;
    }

    public void acceptVisitor(SettingVisitor visitor) {
        if (visitor.visitSettingGroup(this)) {
            for (Setting setting : m_children.values()) {
                setting.acceptVisitor(visitor);
            }
        }
    }

    /**
     * adds the setting to this group collection along with setting the group on the setting
     */
    public Setting addSetting(Setting setting) {
        setting.setParent(this);

        Setting existingChild = m_children.put(setting.getName(), setting);
        if (existingChild != null) {
            Collection<Setting> grandChildren = existingChild.getValues();
            for (Setting grandChild : grandChildren) {
                setting.addSetting(grandChild);
            }
        }

        return setting;
    }

    public Setting getSetting(String name) {
        if (StringUtils.isEmpty(name)) {
            // empty string returns the setting itself
            // why?
            // to support path round-tripping
            // String s = root.getSetting("x").getParent().getPath();
            // Setting root = root.getSetting(s);
            return this;
        }

        String prefix = name;
        String remainder = null;
        int slash = name.indexOf(Setting.PATH_DELIM);
        if (slash > 0) {
            prefix = name.substring(0, slash);
            remainder = name.substring(slash + 1);
        }
        Setting child = m_children.get(prefix);
        if (child == null) {
            // TODO: should we throw an exception here?
            LOG.warn("Cannot find setting: " + name + " in " + this.getPath());
            return null;
        }

        if (remainder == null) {
            // nothing more to do
            return child;
        }

        return child.getSetting(remainder);
    }

    public Collection<Setting> getValues() {
        return m_children.values();
    }

    /**
     * @param requiredType returned Setting must be
     * @return default setting in the set - usually first child
     */
    public Setting getDefaultSetting(Class requiredType) {
        for (Iterator i = getValues().iterator(); i.hasNext();) {
            Setting setting = (Setting) i.next();
            if (requiredType.isAssignableFrom(setting.getClass())) {
                return setting;
            }
        }
        return null;
    }

    public String getDefaultValue() {
        throw new UnsupportedOperationException(getPath());
    }

    public String getValue() {
        throw new UnsupportedOperationException(getPath());
    }

    public void setValue(String value) {
        throw new UnsupportedOperationException(getPath());
    }

    public void setMessageSource(MessageSource messageSource) {
        m_messageSource = messageSource;
    }

    public MessageSource getMessageSource() {
        if (m_messageSource != null) {
            return m_messageSource;
        }
        return super.getMessageSource();
    }
}
