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
package org.sipfoundry.sipxconfig.site.line;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.phone.ManagePhones;


/**
 * Comments
 */
public abstract class LineSettings extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "LineSettings";

    public abstract Integer getLineId();
    
    /** REQUIRED PAGE PARAMETER */
    public abstract void setLineId(Integer id);
    
    public abstract Line getLine();
    
    public abstract void setLine(Line line);
    
    public abstract String getParentSettingName();
    
    /** REQUIRED PAGE PARAMETER */
    public abstract void setParentSettingName(String name); 

    public abstract Setting getParentSetting();
    
    public abstract void setParentSetting(Setting parent);
    
    public abstract PhoneContext getPhoneContext();

    public void pageBeginRender(PageEvent event_) {
        Line line = getLine();
        if (line != null) {
            return;
        }

        PhoneContext context = getPhoneContext();
        line = context.loadLine(getLineId());
        setLine(line);
        Setting root = line.getSettings();
        Setting parent = root.getSetting(getParentSettingName());
        setParentSetting(parent);
    }

    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);
    }

    public void apply(IRequestCycle cycle_) {
        PhoneContext dao = getPhoneContext();
        dao.storeLine(getLine());
        dao.flush();
    }
    
    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }    
}
