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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.Writer;
import java.util.Collection;

import org.apache.velocity.Template;
import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.setting.FilterRunner;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingGroup;

/**
 * Baseclass for velocity template generators
 */
public class ConfigurationFile {
    
    public static final String CALL_SETTINGS = "call";

    public static final String REGISTRATION_SETTINGS = "reg";

    /**
     * Shows all settings and groups in a flat collection
     */
    private static final SettingFilter RECURSIVE_SETTINGS = new SettingFilter() {
        public boolean acceptSetting(Setting root_, Setting setting) {
            boolean group = SettingGroup.class.isAssignableFrom(setting.getClass());
            return !group;
        }
    };        
    
    private static final SettingFilter SETTINGS = new SettingFilter() {
        public boolean acceptSetting(Setting root, Setting setting) {
            boolean firstLevel = (setting.getSettingGroup() == root); 
            boolean group = SettingGroup.class.isAssignableFrom(setting.getClass());
            
            return !group && firstLevel;
        }
    };        

    private PolycomPhone m_phone;

    private String m_template;

    public ConfigurationFile(PolycomPhone phone) {
        m_phone = phone;
    }

    public String getTemplate() {
        return m_template;
    }

    public void setTemplate(String template) {
        m_template = template;
    }

    public PolycomPhone getPhone() {
        return m_phone;
    }
    
    /**
     * Velocity macro convienence method. Recursive list of all settings, ignoring groups
     */
    public Collection getRecursiveSettings(Setting group) {
        return FilterRunner.filter(RECURSIVE_SETTINGS, group);        
    }

    public Collection getSettings(Setting group) {
        return FilterRunner.filter(SETTINGS, group);        
    }
    
    /**
     * Velocity macro convienence method for accessing endpoint settings
     */
    public Setting getEndpointSettings() {
        return getPhone().getSettings();
    }

    protected void addContext(VelocityContext context) {
        context.put("phone", getPhone());
        context.put("endpoint", getPhone().getPhoneMetaData());
        context.put("cfg", this);        
    }

    public void generateProfile(Writer out) {
        Template template;
        // has to be relative to system directory
        try {
            template = getPhone().getVelocityEngine().getTemplate(getTemplate());
        } catch (Exception e) {
            throw new RuntimeException("Error creating velocity template " + getTemplate(), e);
        }

        // PERFORMANCE: depending on how resource intensive the above code is
        // try to reuse the template objects for subsequent profile
        // generations

        VelocityContext velocityContext = new VelocityContext();
        addContext(velocityContext);

        try {
            template.merge(velocityContext, out);
        } catch (Exception e) {
            throw new RuntimeException("Error using velocity template " + getTemplate(), e);
        }
    }
}
