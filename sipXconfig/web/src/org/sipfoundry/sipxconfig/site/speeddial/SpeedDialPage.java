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

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;

public abstract class SpeedDialPage extends UserBasePage {
    public static final String PAGE = "speeddial/SpeedDialPage";

    @InjectObject(value = "spring:speedDialManager")
    public abstract SpeedDialManager getSpeedDialManager();

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
        if (!getValidator().getHasErrors()) {
            SpeedDialManager speedDialManager = getSpeedDialManager();
            speedDialManager.saveSpeedDial(getSpeedDial());
        }
    }
}
