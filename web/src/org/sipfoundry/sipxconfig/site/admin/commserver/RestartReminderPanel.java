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
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Process;

public abstract class RestartReminderPanel extends BaseComponent {
    public static final String PAGE = "RestartReminder";

    public abstract boolean getRestartLater();

    public abstract SipxProcessContext getSipxProcessContext();

    public abstract void setProcesses(Object[] processes);

    public abstract Object[] getProcesses();

    public void restart() {
        if (getRestartLater()) {
            return;
        }
        Object[] processes = getProcesses();
        List procsToRestart = (null != processes) ? Arrays.asList(processes) : Process.getAll();
        SipxProcessContext processContext = getSipxProcessContext();
        for (Iterator i = procsToRestart.iterator(); i.hasNext();) {
            Process p = (Process) i.next();
            processContext.manageService(p, SipxProcessContext.Command.RESTART);
        }
    }
}
