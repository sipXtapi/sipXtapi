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
    
    /** show all setting and groups in one flat list */
    private static final SettingFilter FLATTEN_SETTINGS = new SettingFilter() {
        public boolean acceptSetting(Setting root_, Setting setting_) {
            return true;
        }
    };

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

    public abstract Setting getCurrentSetting();
    
    public abstract void setCurrentSetting(Setting setting);
    
    public abstract void setEditSetting(Setting settings);
    
    public abstract Setting getEditSetting();
    
    public Collection getEndpointSettings() {
        return getPhone().getSettingModel(m_blankEndpoint).getValues();
    }
    
    public Collection getLineSettings() {
        return getPhone().getSettingModel(m_blankLine).getValues();        
    }
    
    public Collection getFlattenedSettings() {
        return FilterRunner.filter(FLATTEN_SETTINGS, getEditSetting());
    }    
    
    public void editEndpointSettings(IRequestCycle cycle_) {        
        setEditDecoratedSettings(getEndpointFolder(), getCurrentSetting());
    }
    
    public void editLineSettings(IRequestCycle cycle_) {        
        setEditDecoratedSettings(getLineFolder(), getCurrentSetting());
    }
    
    public void hideGroup(IRequestCycle cycle_) {
        getEditSetting().setHidden(true);
    }

    public void showGroup(IRequestCycle cycle_) {
        getEditSetting().setHidden(false);
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
        setEndpointFolder(getSettingDao().loadRootFolder(Endpoint.FOLDER_RESOURCE_NAME));
        setLineFolder(getSettingDao().loadRootFolder(Line.FOLDER_RESOURCE_NAME));
        
        m_blankEndpoint = new Endpoint();
        m_blankEndpoint.setPhoneId(getPhoneId());
        m_blankEndpoint.setFolder(getEndpointFolder());
        
        m_blankLine = new Line();
        m_blankEndpoint.addLine(m_blankLine);
        m_blankLine.setFolder(getLineFolder());

        setPhone(getPhoneContext().getPhone(m_blankEndpoint));
        
        if (getEditSetting() == null) {
            setEditDecoratedSettings(getEndpointFolder(), (Setting) getEndpointSettings().iterator().next());
        }
    }
    
    private void setEditDecoratedSettings(Folder folder, Setting setting) {
        setEditSetting(folder.decorate(setting));        
    }
}
