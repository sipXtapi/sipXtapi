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
import org.sipfoundry.sipxconfig.setting.MetaStorage;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.setting.SettingGroup;


public abstract class PhoneDefaults extends BasePage implements PageRenderListener  {
    
    private Endpoint m_blankEndpoint;
    
    private Line m_blankLine;
    
    public abstract void setPhone(Phone phone);
    
    public abstract Phone getPhone();
    
    public abstract int getPhoneId();
    
    /** REQUIRED PAGE PROPERTY */
    public abstract void setPhoneId(int id);
    
    /** REQUIRED PAGE PROPERTY */
    public abstract void setMetaStorageId(int id);
    
    public abstract int getMetaStorageId();
    
    public abstract MetaStorage getMetaStorage();
    
    public abstract void setMetaStorage(MetaStorage metaStorage);
    
    public abstract SettingDao getSettingDao();
    
    public abstract PhoneContext getPhoneContext();

    public abstract SettingGroup getCurrentSetting();
    
    public abstract void setCurrentSetting(SettingGroup group);
    
    public abstract void setEditSetting(SettingGroup group);
    
    public abstract SettingGroup getEditSetting();
    
    public Collection getPhoneSettings() {
        return getPhone().getSettingModel(m_blankEndpoint).getValues();
    }
    
    public Collection getLineSettings() {
        return getPhone().getSettingModel(m_blankLine).getValues();        
    }
    
    public void editSettings(IRequestCycle cycle_) {        
        setEditSetting(getCurrentSetting());
    }
    
    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);        
    }
    
    public void apply(IRequestCycle cycle_) {
        getSettingDao().storeMetaStorage(getMetaStorage());
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }

    public void pageBeginRender(PageEvent event_) {
        m_blankEndpoint = new Endpoint();
        m_blankEndpoint.addLine(m_blankLine);
        m_blankLine = new Line();
        
        MetaStorage metaStorage;
        if (getMetaStorageId() == 0) {
            metaStorage = new MetaStorage();
        } else {
            metaStorage = getSettingDao().loadMetaStorage(getMetaStorageId());
        }
        setMetaStorage(metaStorage);
        
        setPhone(getPhoneContext().getPhone(getPhoneId()));
        
        if (getEditSetting() == null) {
            setEditSetting((SettingGroup) getPhoneSettings().iterator().next());
        }
    }
}
