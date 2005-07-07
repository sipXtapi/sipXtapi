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
import org.sipfoundry.sipxconfig.phone.polycom.PolycomModel;
import org.sipfoundry.sipxconfig.setting.FilterRunner;
import org.sipfoundry.sipxconfig.setting.Folder;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.setting.SettingFilter;

/**
 * Future Redesign Notes:
 * This page should be redesigned as a generic EditFolder page.  Right now this manages
 * root folders for phones and line and probably shouldn't from a user and architecture
 * position.  Also, it assumes root folder and should edit any level folder.
 */
public abstract class PhoneDefaults extends BasePage implements PageRenderListener  {
    
    public static final String PAGE = "PhoneDefaults";
    
    public abstract void setPhone(Phone phone);
    
    public abstract Phone getPhone();
    
    public abstract String getPhoneFactoryId();
    
    public abstract void setPhoneFactoryId(String factoryId);
    
    public abstract Folder getPhoneFolder();
    
    public abstract void setPhoneFolder(Folder folder);

    public abstract Folder getLineFolder();

    public abstract void setLineFolder(Folder folder);

    public abstract SettingDao getSettingDao();
    
    public abstract PhoneContext getPhoneContext();

    public abstract Setting getCurrentNavigationSetting();
    
    public abstract void setEditFormSetting(Setting setting);
    
    public abstract void setEditFormSettings(Collection settings);
    
    public abstract String getEditFormSettingName();
    
    public abstract void setEditFormSettingName(String name);

    public abstract void setEditFormFolderResource(String resource);
    
    public abstract String getEditFormFolderResource();
    
    public Collection getPhoneNavigationSettings() {
        return getPhone().getSettings().getValues();
    }
    
    public Collection getLineNavigationSettings() {
        return getPhone().getLine(0).getSettings().getValues();        
    }
    
    public void editPhoneSettings(IRequestCycle cycle_) {
        setEditFormFolderResource(PhoneData.FOLDER_RESOURCE_NAME);
        setEditFormSettingName(getCurrentNavigationSetting().getName());
        editSettings();        
    }
        
    public void editLineSettings(IRequestCycle cycle_) {
        setEditFormFolderResource(LineData.FOLDER_RESOURCE_NAME);
        setEditFormSettingName(getCurrentNavigationSetting().getName());
        editSettings();        
    }
    
    public void hideGroup(IRequestCycle cycle_) {
        //getEditSetting().setHidden(true);
    }

    public void showGroup(IRequestCycle cycle_) {
        //getEditSetting().setHidden(false);
    }

    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);        
    }
    
    public void apply(IRequestCycle cycle_) {
        getSettingDao().storeFolder(getPhoneFolder());
        getSettingDao().storeFolder(getLineFolder());
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }

    public void pageBeginRender(PageEvent event_) {
        if (getPhoneFactoryId() == null) {
            // all models the same
            setPhoneFactoryId(PolycomModel.MODEL_600.getModelId());
        }
        // future, edit other meta storages, not just root's
        setPhoneFolder(getPhoneContext().loadRootPhoneFolder());
        setLineFolder(getPhoneContext().loadRootLineFolder());
        
        Phone phone = getPhoneContext().newPhone(getPhoneFactoryId());
        phone.getPhoneData().setFolder(getPhoneFolder());
        
        Line line = phone.createLine(new LineData());
        phone.addLine(line);
        line.getLineData().setFolder(getLineFolder());

        setPhone(phone);
        
        String editSettingsName = getEditFormSettingName(); 
        if (editSettingsName == null) {
            setEditFormFolderResource(PhoneData.FOLDER_RESOURCE_NAME);
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
        Folder folder;
        Setting rootSettings;
        if (getEditFormFolderResource().equals(PhoneData.FOLDER_RESOURCE_NAME)) {
            folder = getPhoneFolder();
            rootSettings = getPhone().getSettingModel().copy();
        } else {
            folder = getLineFolder();
            rootSettings = getPhone().getLine(0).getSettingModel().copy();
        }

        Setting subset = rootSettings.getSetting(getEditFormSettingName());
        setEditFormSetting(subset);
        Setting decorated = folder.decorate(subset);
        setEditFormSettings(FilterRunner.filter(SettingFilter.ALL, decorated));
    }    
}
