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

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;

public abstract class PhoneModels extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "PhoneModels";

    public abstract void setGroupId(Integer id);

    public abstract Integer getGroupId();

    public abstract void setGroup(Group group);

    public abstract SettingDao getSettingContext();

    public IPage editGroup(IRequestCycle cycle) {
        EditGroup page = (EditGroup) cycle.getPage(EditGroup.PAGE);
        page.editGroup(getGroupId(), PAGE);
        return page;
    }

    public IPage editPhoneDefaults(IRequestCycle cycle, String name) {
        PhoneModel model = PhoneModel.getModel(name);
        EditPhoneDefaults page = (EditPhoneDefaults) cycle.getPage(EditPhoneDefaults.PAGE);
        page.editPhoneSettings(model, getGroupId());
        return page;
    }

    public void pageBeginRender(PageEvent event_) {
        Group g = getSettingContext().loadGroup(getGroupId());
        setGroup(g);
    }
}
