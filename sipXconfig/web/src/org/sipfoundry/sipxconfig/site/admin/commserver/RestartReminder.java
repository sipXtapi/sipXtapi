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
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.apache.tapestry.html.BasePage;

public abstract class RestartReminder extends BasePage {
    public static final String PAGE = "RestartReminder";

    public abstract void setNextPage(String nextPage);

    public abstract String getNextPage();

    public abstract void setProcesses(Object[] processes);

    public abstract Object[] getProcesses();

    public String proceed() {
        RestartReminderPanel reminder = (RestartReminderPanel) getComponent("reminder");
        reminder.restart();
        return getNextPage();
    }
}
