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
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.polycom.Polycom;
import org.sipfoundry.sipxconfig.setting.FilterRunner;
import org.sipfoundry.sipxconfig.setting.Folder;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.setting.SettingFilter;


public abstract class PhoneDefaults extends BasePage implements PageRenderListener  {
    
    public static final String PAGE = "PhoneDefaults";
    
    private Endpoint m_blankEndpoint;
    
    private Line m_blankLine;
    
    public abstract void setPhone(Phone phone);
    
    public abstract Phone getPhone();
    
    public abstract String getPhoneId();
    
    public abstract void setPhoneId(String phoneId);
    
    public abstract Folder getEndpointFolder();
    
    public abstract void setEndpointFolder(Folder folder);

    public abstract Folder getLineFolder();

    public abstract void setLineFolder(Folder folder);

    public abstract SettingDao getSettingDao();
    
    public abstract PhoneContext getPhoneContext();

    public abstract Setting getCurrentNavigationSetting();
    
    public abstract Setting getCurrentEditFormSetting();
    
    public abstract Collection getEditFormSettings();
    
    public abstract void setEditFormSettings(Collection settings);
    
    public abstract String getEditFormSettingName();
    
    public abstract void setEditFormSettingName(String name);

    public abstract void setEditFormFolderResource(String resource);
    
    public abstract String getEditFormFolderResource();
    
    public Collection getEndpointNavigationSettings() {
        return getPhone().getSettingModel(m_blankEndpoint).getValues();
    }
    
    public Collection getLineNavigationSettings() {
        return getPhone().getSettingModel(m_blankLine).getValues();        
    }
    
    public void editEndpointSettings(IRequestCycle cycle_) {
        setEditFormFolderResource(Endpoint.FOLDER_RESOURCE_NAME);
        setEditFormSettingName(getCurrentNavigationSetting().getName());
        editSettings();        
    }
        
    public void editLineSettings(IRequestCycle cycle_) {
        setEditFormFolderResource(Line.FOLDER_RESOURCE_NAME);
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
        getSettingDao().storeFolder(getEndpointFolder());
        getSettingDao().storeFolder(getLineFolder());
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }

    public void pageBeginRender(PageEvent event_) {
        if (getPhoneId() == null) {
            // all models the same
            setPhoneId(Polycom.MODEL_600.getModelId());
        }
        // future, edit other meta storages, not just root's
        setEndpointFolder(getPhoneContext().loadRootEndpointFolder());
        setLineFolder(getPhoneContext().loadRootLineFolder());
        
        m_blankEndpoint = new Endpoint();
        m_blankEndpoint.setPhoneId(getPhoneId());
        m_blankEndpoint.setFolder(getEndpointFolder());
        
        m_blankLine = new Line();
        m_blankEndpoint.addLine(m_blankLine);
        m_blankLine.setFolder(getLineFolder());

        setPhone(getPhoneContext().getPhone(m_blankEndpoint));
        
        String editSettingsName = getEditFormSettingName(); 
        if (editSettingsName == null) {
            setEditFormFolderResource(Endpoint.FOLDER_RESOURCE_NAME);
            Iterator nav = getEndpointNavigationSettings().iterator();
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
        if (getEditFormFolderResource().equals(Endpoint.FOLDER_RESOURCE_NAME)) {
            folder = getEndpointFolder();
            rootSettings = getPhone().getSettingModel(m_blankEndpoint);
        } else {
            folder = getLineFolder();
            rootSettings = getPhone().getSettingModel(m_blankLine);
        }

        Setting subset = rootSettings.getSetting(getEditFormSettingName());
        Setting decorated = folder.decorate(subset);
        setEditFormSettings(FilterRunner.filter(SettingFilter.ALL, decorated));
    }    
}
