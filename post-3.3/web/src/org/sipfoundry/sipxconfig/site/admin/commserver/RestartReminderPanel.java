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

import java.util.Arrays;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Process;

public abstract class RestartReminderPanel extends BaseComponent {
    public static final String PAGE = "RestartReminder";

    public abstract boolean getRestartLater();

    public abstract void setRestartLater(boolean restartLater);

    public abstract SipxProcessContext getSipxProcessContext();

    public abstract Object[] getProcesses();

    public abstract void setProcesses(Object[] processes);

    public abstract Class getEventClass();

    public abstract void setEventClass(Class eventClass);

    public void restart() {
        List procsToRestart = getProcessesToRestart();
        if (procsToRestart == null) {
            return;
        }
        SipxProcessContext processContext = getSipxProcessContext();
        Class eventClass = getEventClass();
        if (eventClass == null) {
            processContext.manageServices(procsToRestart, SipxProcessContext.Command.RESTART);
        } else {
            processContext.restartOnEvent(procsToRestart, eventClass);
        }
    }

    public List getProcessesToRestart() {
        if (getRestartLater()) {
            return null;
        }
        Object[] processes = getProcesses();
        if (processes != null) {
            return Arrays.asList(processes);
        }
        return Process.getRestartable();
    }
}
