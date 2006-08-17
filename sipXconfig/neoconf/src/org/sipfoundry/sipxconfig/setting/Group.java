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

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.DataCollectionItem;
import org.sipfoundry.sipxconfig.common.NamedObject;

/**
 * User labeled storage of settings.
 * 
 * @author dhubler
 * 
 */
public class Group extends ValueStorage implements Comparable, DataCollectionItem, NamedObject {
    private String m_name;
    private String m_description;
    private String m_resource;
    private Integer m_weight;
    private GroupSettingModel m_model;

    public String getName() {
        return m_name;
    }

    public void setName(String label) {
        m_name = label;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getResource() {
        return m_resource;
    }

    public void setResource(String resource) {
        m_resource = resource;
    }

    /**
     * When setting values conflict, the setting with the highest weight wins.
     * 
     * @return setting weight
     */
    public Integer getWeight() {
        return m_weight;
    }

    public void setWeight(Integer weight) {
        m_weight = weight;
    }

    public int compareTo(Object arg0) {
        Group b = (Group) arg0;
        Integer w1 = defaultWeight(m_weight);
        Integer w2 = defaultWeight(b.getWeight());
        int cmp = w1.compareTo(w2);
        if (cmp == 0) {
            String s1 = StringUtils.defaultString(getName());
            String s2 = StringUtils.defaultString(b.getName());
            cmp = s1.compareTo(s2);
        }
        return cmp;
    }

    private Integer defaultWeight(Integer weight) {
        return weight != null ? weight : new Integer(-1);
    }

    /**
     * byproduct of DataCollectionItem interface, returns weight - 1
     */
    public int getPosition() {
        int w = (m_weight != null ? m_weight.intValue() : -1);
        return w - 1;
    }

    /**
     * byproduct of DataCollectionItem interface, sets weight to position + 1
     */
    public void setPosition(int position) {
        m_weight = new Integer(position + 1);
    }

    /**
     * When editing group settings, a group needs to inherhit the settings for a bean so it can
     * save setting values for that bean.
     * 
     * @param bean to inherit settings for
     * @return copy of settings to be edited
     */
    public Setting inherhitSettingsForEditing(BeanWithSettings bean) {
        Setting settings = bean.getSettings().copy();
        m_model = new GroupSettingModel(this, bean);
        settings.acceptVisitor(new InheritSettings(m_model));
        return settings;
    }

    static class GroupSettingModel implements SettingModel2 {
        private BeanWithSettings m_bean;
        private Group m_group;

        public GroupSettingModel(Group group, BeanWithSettings bean) {
            m_bean = bean;
            m_group = group;
        }

        public void setSettingValue(Setting setting, String value) {
            SettingValue2 defaultValue = new SettingValueImpl(m_bean.getSettingValue(setting
                    .getPath()));
            m_group.setSettingValue(setting, new SettingValueImpl(value), defaultValue);
        }

        public SettingValue2 getSettingValue(Setting setting) {
            SettingValue2 value = m_group.getSettingValue(setting);
            if (value == null) {
                value = m_bean.getSettingModel2().getSettingValue(setting);
            }

            return value;
        }

        public SettingValue2 getDefaultSettingValue(Setting setting) {
            // can delegate to bean, because settings still contains different setting model
            SettingValue2 value = m_bean.getSettingModel2().getDefaultSettingValue(setting);
            return value;
        }

        public SettingValue2 getProfileName(Setting setting) {
            // can delegate to bean, because settings still contains different setting model
            SettingValue2 value = m_bean.getSettingModel2().getProfileName(setting);
            return value;
        }
    }

    private static class InheritSettings extends AbstractSettingVisitor {
        private SettingModel2 m_model;

        public InheritSettings(SettingModel2 model) {
            m_model = model;
        }

        @Override
        public void visitSetting(Setting setting) {
            SettingImpl impl = SettingUtil.getSettingImpl(setting);
            impl.setModel(m_model);
        }
    }
}
