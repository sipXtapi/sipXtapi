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
package org.sipfoundry.sipxconfig.site.phone;

import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineData;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneData;
import org.sipfoundry.sipxconfig.setting.FilterRunner;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.Tag;

public abstract class EditPhoneDefaults extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "EditPhoneDefaults";
    
    public abstract void setPhone(Phone phone);
    
    public abstract Phone getPhone();
    
    public abstract String getPhoneFactoryId();
    
    public abstract void setPhoneFactoryId(String factoryId);
    
    public abstract Tag getPhoneTag();
    
    public abstract void setPhoneTag(Tag tag);

    public abstract Tag getLineTag();

    public abstract void setLineTag(Tag tag);

    public abstract SettingDao getSettingDao();
    
    public abstract PhoneContext getPhoneContext();

    public abstract Setting getCurrentNavigationSetting();
    
    public abstract void setEditFormSetting(Setting setting);
    
    public abstract void setEditFormSettings(Collection settings);
    
    public abstract String getEditFormSettingName();
    
    public abstract void setEditFormSettingName(String name);

    public abstract void setEditFormTagResource(String resource);
    
    public abstract String getEditFormTagResource();
    
    /**
     * Entry point for other pages to edit a phone model's default settings 
     */
    public void editPhoneSettings(String factoryId) {
        setPhoneFactoryId(factoryId);
        setEditFormSettingName(null);
    }
    
    public Collection getPhoneNavigationSettings() {
        return getPhone().getSettings().getValues();
    }
    
    public Collection getLineNavigationSettings() {
        return getPhone().getLine(0).getSettings().getValues();        
    }
    
    public void editPhoneSettings(IRequestCycle cycle_) {
        setEditFormTagResource(PhoneData.TAG_RESOURCE_NAME);
        setEditFormSettingName(getCurrentNavigationSetting().getName());
        editSettings();        
    }
        
    public void editLineSettings(IRequestCycle cycle_) {
        setEditFormTagResource(LineData.TAG_RESOURCE_NAME);
        setEditFormSettingName(getCurrentNavigationSetting().getName());
        editSettings();        
    }
    
    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);        
    }
    
    public void apply(IRequestCycle cycle_) {
        getSettingDao().storeTag(getPhoneTag());
        getSettingDao().storeTag(getLineTag());
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }

    public void pageBeginRender(PageEvent event_) {
        if (getPhoneFactoryId() == null) {
            throw new IllegalArgumentException("phone factory id required");
        }
        // future, edit other meta storages, not just root's
        setPhoneTag(getPhoneContext().loadRootPhoneTag());
        setLineTag(getPhoneContext().loadRootLineTag());
        
        Phone phone = getPhoneContext().newPhone(getPhoneFactoryId());
        phone.getPhoneData().addTag(getPhoneTag());
        
        Line line = phone.createLine(new LineData());
        phone.addLine(line);
        line.getLineData().addTag(getLineTag());

        setPhone(phone);
        
        String editSettingsName = getEditFormSettingName(); 
        if (editSettingsName == null) {
            setEditFormTagResource(PhoneData.TAG_RESOURCE_NAME);
            Iterator nav = getPhoneNavigationSettings().iterator();
            setEditFormSettingName(((Setting) nav.next()).getName());
        }        
        
        editSettings();        
    }
    
    /**
     * Based on current (persistent) page state, setup the settings data 
     * for the setting edit form
     */
    private void editSettings() {
        Tag folder;
        Setting rootSettings;
        if (getEditFormTagResource().equals(PhoneData.TAG_RESOURCE_NAME)) {
            folder = getPhoneTag();
            rootSettings = getPhone().getSettingModel().copy();
        } else {
            folder = getLineTag();
            rootSettings = getPhone().getLine(0).getSettingModel().copy();
        }

        Setting subset = rootSettings.getSetting(getEditFormSettingName());
        setEditFormSetting(subset);
        Setting decorated = folder.decorate(subset);
        setEditFormSettings(FilterRunner.filter(SettingFilter.ALL, decorated));
    }    

}
