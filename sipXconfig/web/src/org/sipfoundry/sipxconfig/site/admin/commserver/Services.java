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
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.Collection;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.commserver.ServiceStatus;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessManager;
import org.sipfoundry.sipxconfig.components.SelectMap;

public abstract class Services extends BasePage {
    public static final String PAGE = "Services";

    public abstract SipxProcessManager getSipxProcessContext();

    public abstract ServiceStatus getCurrentRow();

    public abstract SelectMap getSelections();

    public abstract Collection getServicesToStart();

    public abstract Collection getServicesToStop();

    public abstract Collection getServicesToRestart();

    public void formSubmit(IRequestCycle cycle_) {
        // Ideally the start/stop/restart operations would be implemented in button listeners.
        // However, Tapestry 3.0 has a bug in it such that when a component listener is
        // triggered, data is available only for those components that precede it in the
        // rendering order. So wait until formSubmit, at which time all data will be there.

        SipxProcessManager processContext = getSipxProcessContext();
        if (getServicesToStart() != null) {
            // Start selected services
            String[] serviceNames = namesCollectionToArray(getServicesToStart());
            processContext.manageServices(serviceNames, SipxProcessManager.Command.START);
        } else if (getServicesToStop() != null) {
            // Stop selected services
            String[] serviceNames = namesCollectionToArray(getServicesToStop());
            processContext.manageServices(serviceNames, SipxProcessManager.Command.STOP);
        } else if (getServicesToRestart() != null) {
            // Restart selected services
            String[] serviceNames = namesCollectionToArray(getServicesToRestart());
            processContext.manageServices(serviceNames, SipxProcessManager.Command.RESTART);
        }
    }

    /** No-op function does nothing. It is used as the listener function for the Refresh button. */
    public void noop(IRequestCycle cycle_) {
    }

    private String[] namesCollectionToArray(Collection names) {
        return (String[]) names.toArray(new String[names.size()]);
    }
}
