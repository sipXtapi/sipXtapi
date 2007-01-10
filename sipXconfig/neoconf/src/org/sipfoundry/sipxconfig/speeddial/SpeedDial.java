/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.User;

/**
 * Collection of speeddial buttons associated with the user.
 */
public class SpeedDial extends BeanWithId {
    private User m_user;

    private List<Button> m_buttons = new ArrayList<Button>();

    public List<Button> getButtons() {
        return m_buttons;
    }

    public void setButtons(List<Button> buttons) {
        m_buttons = buttons;
    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public void replaceButtons(List<Button> buttons) {
        m_buttons.clear();
        m_buttons.addAll(buttons);
    }

    public void moveButtons(int index, int moveOffset) {
        List<Button> buttons = getButtons();
        DataCollectionUtil.move(buttons, index, moveOffset);
    }

    public String getResourceListId() {
        return "~~rl~" + getUser().getId();
    }

    public String getResourceListName() {
        return getUser().getUserName();
    }

    /**
     * If at least one button supports BLF we need to register this list as BLF
     */
    public boolean isBlf() {
        for (Button button : getButtons()) {
            if (button.isBlf()) {
                return true;
            }
        }
        return false;
    }
}
