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
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.SettingGroup;
import org.sipfoundry.sipxconfig.site.phone.ManagePhones;
import org.sipfoundry.sipxconfig.site.phone.PhonePageUtils;


/**
 * Comments
 */
public abstract class LineSettings extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "LineSettings";

    public abstract int getLineId();
    
    /** REQUIRED PAGE PARAMETER */
    public abstract void setLineId(int id);
    
    public abstract Line getLine();
    
    public abstract void setLine(Line line);
    
    public abstract String getParentSettingGroupName();
    
    /** REQUIRED PAGE PARAMETER */
    public abstract void setParentSettingGroupName(String name); 

    public abstract SettingGroup getParentSettingGroup();
    
    public abstract void setParentSettingGroup(SettingGroup parent);
    
    public void pageBeginRender(PageEvent event) {
        PhoneContext context = PhonePageUtils.getPhoneContext(event.getRequestCycle());
        Line line = context.loadLine(getLineId());
        setLine(line);
        Phone phone = context.getPhone(line.getEndpoint()); 
        SettingGroup root = line.getSettings(phone);
        SettingGroup parent = (SettingGroup) root.getSetting(getParentSettingGroupName());
        setParentSettingGroup(parent);
    }

    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);
    }

    public void apply(IRequestCycle cycle) {
        PhoneContext dao = PhonePageUtils.getPhoneContext(cycle);
        dao.storeLine(getLine());
        dao.flush();
    }
    
    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }    
}
