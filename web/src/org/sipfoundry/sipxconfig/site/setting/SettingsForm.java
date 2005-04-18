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
package org.sipfoundry.sipxconfig.site.setting;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;
import org.sipfoundry.sipxconfig.setting.FilterRunner;
import org.sipfoundry.sipxconfig.setting.RenderProperties;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingRenderer;


public abstract class SettingsForm extends BaseComponent {
    
    private static final RenderProperties DEFAULT_RENDER_PROPERTIES = new RenderProperties();
    
    public abstract SettingRenderer getSettingRenderer();
    
    public abstract Setting getCurrentSetting();
    
    public abstract Setting getSettings();
    
    public Collection getFlattenedSettings() {
        return FilterRunner.filter(SettingFilter.ALL, getSettings());
    }
    
    public RenderProperties getCurrentRenderProperties() {
        RenderProperties rprops = DEFAULT_RENDER_PROPERTIES;
        SettingRenderer render = getSettingRenderer();
        if (render != null) {
            RenderProperties customprops = render.getRenderProperties(getCurrentSetting());
            if (customprops != null) {
                rprops = customprops;
            }            
        }
        
        return rprops;
    }
}
