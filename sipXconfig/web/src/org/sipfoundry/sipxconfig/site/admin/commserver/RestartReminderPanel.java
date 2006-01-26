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

    public abstract void setSipxProcessContext(SipxProcessContext sipxProcessContext);

    public abstract void setProcesses(Object[] processes);

    public abstract Object[] getProcesses();

    public void restart() {
        List procsToRestart = getProcessesToRestart();
        if (procsToRestart != null) {
            SipxProcessContext processContext = getSipxProcessContext();
            processContext.manageServices(procsToRestart, SipxProcessContext.Command.RESTART);
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
        return Process.getAll();
    }
}
