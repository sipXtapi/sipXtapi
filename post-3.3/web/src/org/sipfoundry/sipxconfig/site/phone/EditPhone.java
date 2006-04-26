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
import java.util.Collections;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.ProfileManager;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.site.setting.EditGroup;

/**
 * Tapestry Page support for editing and creating new phones
 */
public abstract class EditPhone extends PageWithCallback implements PageBeginRenderListener {

    public static final String PAGE = "EditPhone";

    public abstract Phone getPhone();

    public abstract void setPhone(Phone phone);

    /** REQUIRED PROPERTY */
    public abstract Integer getPhoneId();

    public abstract void setPhoneId(Integer id);

    public abstract PhoneContext getPhoneContext();
    public abstract ProfileManager getProfileManager();
    public abstract SettingDao getSettingDao();

    public IPage addLine(IRequestCycle cycle, Integer phoneId) {
        AddPhoneUser page = (AddPhoneUser) cycle.getPage(AddPhoneUser.PAGE);
        page.setReturnToEditPhone(true);
        page.setPhoneId(phoneId);
        return page;
    }

    public void commit() {
        save();
    }

    private boolean save() {
        boolean valid = TapestryUtils.isValid(this);
        if (valid) {
            PhoneContext dao = getPhoneContext();
            Phone phone = getPhone();
            EditGroup.saveGroups(getSettingDao(), phone.getGroups());            
            dao.storePhone(phone);
        }

        return valid;
    }
    
    public void generateProfile() {
        Collection phoneIds = Collections.singleton(getPhone().getId());
        getProfileManager().generateProfilesAndRestart(phoneIds);
        String msg = getMessages().getMessage("msg.success.profiles");
        TapestryUtils.recordSuccess(this, msg);
    }

    public void pageBeginRender(PageEvent event_) {
        if (getPhone() != null) {
            return;
        }

        // Load the phone with the ID that was passed in
        PhoneContext context = getPhoneContext();
        setPhone(context.loadPhone(getPhoneId()));
    }
}
