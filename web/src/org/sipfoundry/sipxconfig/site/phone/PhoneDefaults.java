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
import org.sipfoundry.sipxconfig.setting.Folder;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.setting.SettingGroup;


public abstract class PhoneDefaults extends BasePage implements PageRenderListener  {
    
    public static final String PAGE = "PhoneDefaults";
    
    private Endpoint m_blankEndpoint;
    
    private Line m_blankLine;
    
    public abstract void setPhone(Phone phone);
    
    public abstract Phone getPhone();
    
    public abstract int getPhoneId();

    public abstract void setPhoneId();
    
    /** REQUIRED PROPERTY */
    public abstract void setPhoneId(int id);
    
    public abstract Folder getPhoneFolder();
    
    public abstract void setPhoneFolder(Folder metaStorage);

    public abstract Folder getLineFolder();

    public abstract void setLineFolder(Folder metaStorage);

    public abstract SettingDao getSettingDao();
    
    public abstract PhoneContext getPhoneContext();

    public abstract SettingGroup getCurrentSetting();
    
    public abstract void setCurrentSetting(Setting setting);
    
    public abstract void setEditSetting(Setting setting);
    
    public abstract Setting getEditSetting();
    
    public Collection getPhoneSettings() {
        return getPhone().getSettingModel(m_blankEndpoint).getValues();
    }
    
    public Collection getLineSettings() {
        return getPhone().getSettingModel(m_blankLine).getValues();        
    }
    
    public void edit(IRequestCycle cycle_) {        
        setEditSetting(getCurrentSetting());
    }
    
    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);        
    }
    
    public void apply(IRequestCycle cycle_) {
        getSettingDao().storeMetaStorage(getPhoneFolder());
        getSettingDao().storeMetaStorage(getLineFolder());
    }

    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }

    public void pageBeginRender(PageEvent event_) {
        m_blankEndpoint = new Endpoint();
        m_blankEndpoint.addLine(m_blankLine);
        m_blankLine = new Line();

        setPhone(getPhoneContext().getPhone(getPhoneId()));
        
        // future, edit other meta storages, not just root's
        setPhoneFolder(getPhoneContext().getRootPhoneFolder());
        setLineFolder(getPhoneContext().getRootLineFolder());        
        
        if (getEditSetting() == null) {
            setEditSetting((Setting) getPhoneSettings().iterator().next());
        }
    }
}
