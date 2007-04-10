/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.speeddial;

import java.util.Collection;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;

public abstract class SpeedDialPage extends UserBasePage {
    public static final String PAGE = "speeddial/SpeedDialPage";

    @InjectObject(value = "spring:speedDialManager")
    public abstract SpeedDialManager getSpeedDialManager();

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    @InjectObject(value = "spring:profileManager")
    public abstract ProfileManager getProfileManager();

    @Persist
    public abstract Integer getSavedUserId();

    public abstract void setSavedUserId(Integer savedUserId);

    public abstract SpeedDial getSpeedDial();

    public abstract void setSpeedDial(SpeedDial speedDial);

    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);

        Integer userId = getUserId();
        if (userId.equals(getSavedUserId()) && getSpeedDial() != null) {
            // same user and we have cached buttons - nothing to do
            return;
        }

        SpeedDial speedDial = getSpeedDialManager().getSpeedDialForUserId(userId, true);
        setSpeedDial(speedDial);
        setSavedUserId(userId);
    }

    public void onApply() {
        if (TapestryUtils.isValid(this)) {
            SpeedDialManager speedDialManager = getSpeedDialManager();
            speedDialManager.saveSpeedDial(getSpeedDial());
        }
    }
    
    public void onUpdatePhones() {
        if (TapestryUtils.isValid(this)) {
            onApply();
            Collection<Phone> phones = getPhoneContext().getPhonesByUserId(getActiveUserId());
            Collection<Integer> ids = DataCollectionUtil.extractPrimaryKeys(phones);
            getProfileManager().generateProfilesAndRestart(ids);
        }
    }
}
