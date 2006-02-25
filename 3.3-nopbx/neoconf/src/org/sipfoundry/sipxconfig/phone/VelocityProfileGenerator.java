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

import java.io.Writer;
import java.util.Collection;

import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.SettingUtil;

/**
 * Baseclass for velocity template generators
 */
public class VelocityProfileGenerator {
    
    /**
     * Shows all settings and groups in a flat collection
     */
    private static final SettingFilter RECURSIVE_SETTINGS = new SettingFilter() {
        public boolean acceptSetting(Setting root_, Setting setting) {
            boolean group = SettingSet.class.isAssignableFrom(setting.getClass());
            return !group;
        }
    };        
    
    private static final SettingFilter SETTINGS = new SettingFilter() {
        public boolean acceptSetting(Setting root, Setting setting) {
            boolean firstGeneration = setting.getParentPath().equals(root.getPath());
            boolean isLeaf = setting.getValues().isEmpty();            
            return firstGeneration && isLeaf;
        }
    };        

    private Phone m_phone;

    private VelocityEngine m_velocityEngine;

    public VelocityProfileGenerator(Phone phone) {
        m_phone = phone;
    }

    public VelocityEngine getVelocityEngine() {
        return m_velocityEngine;
    }

    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public Phone getPhone() {
        return m_phone;
    }
    
    /**
     * Velocity macro convienence method. Recursive list of all settings, ignoring groups
     */
    public Collection getRecursiveSettings(Setting group) {
        return SettingUtil.filter(RECURSIVE_SETTINGS, group);        
    }

    public Collection getSettings(Setting group) {
        return SettingUtil.filter(SETTINGS, group);        
    }
    
    /**
     * Velocity macro convienence method for accessing endpoint settings
     */
    public Setting getEndpointSettings() {
        return getPhone().getSettings();
    }
    
    protected void addContext(VelocityContext context) {
        context.put("phone", getPhone());
        context.put("cfg", this);                        
    }

    public void generateProfile(String template, Writer out) {
        VelocityContext velocityContext = new VelocityContext();
        addContext(velocityContext);

        try {
            getVelocityEngine().mergeTemplate(template, velocityContext, out);
        } catch (Exception e) {
            throw new RuntimeException("Error using velocity template " + template, e);
        }
    }
}
