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

import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;

/**
 * UserCallForwarding
 */
public abstract class SpeedDialPage extends UserBasePage {
    public static final String PAGE = "speeddial/SpeedDialPage";

    @InjectObject(value = "spring:speedDialManager")
    public abstract SpeedDialManager getSpeedDialManager();

    @Persist
    public abstract Integer getSavedUserId();

    public abstract void setSavedUserId(Integer savedUserId);

    @Persist
    public abstract List<Button> getButtons();
    
    public abstract void setButtons(List<Button> buttons);

    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);

        Integer userId = getUserId();
        if (userId.equals(getSavedUserId()) && getButtons() != null) {
            // same user and we have cached buttons - nothing to do
            return;
        }

        SpeedDial speedDial = getSpeedDialManager().getSpeedDialForUserId(userId);
        ArrayList<Button> buttons = new ArrayList<Button>();
        buttons.addAll(speedDial.getButtons());
        if (buttons.isEmpty()) {
            buttons.add(new Button());
        }
        setButtons(buttons);
        setSavedUserId(userId);
    }
    
    public void onApply() {
        Integer userId = getUserId();
        SpeedDialManager speedDialManager = getSpeedDialManager();
        SpeedDial speedDial = speedDialManager.getSpeedDialForUserId(userId);
        speedDial.getButtons().clear();
        speedDial.getButtons().addAll(getButtons());
        
        speedDialManager.saveSpeedDial(speedDial);
        setButtons(null);
    }

}
