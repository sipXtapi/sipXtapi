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
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ITableRendererSource;
import org.apache.tapestry.contrib.table.model.ognl.ExpressionTableColumn;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.components.LocalizedTableRendererSource;

public abstract class Services extends BasePage {
    public static final String PAGE = "Services";
    private static final String STATUS_COLUMN = "status";

    public abstract SipxProcessContext getSipxProcessContext();

    public abstract Collection getServicesToStart();

    public abstract Collection getServicesToStop();

    public abstract Collection getServicesToRestart();

    public ITableColumn getStatusColumn() {
        ExpressionTableColumn column = new ExpressionTableColumn(STATUS_COLUMN,
                getMessage(STATUS_COLUMN), "status.name", true);
        ITableRendererSource rendererSource = new LocalizedTableRendererSource(getMessages(),
                STATUS_COLUMN);
        column.setValueRendererSource(rendererSource);
        return column;
    }

    public void formSubmit(IRequestCycle cycle_) {
        // Ideally the start/stop/restart operations would be implemented in button listeners.
        // However, Tapestry 3.0 has a bug in it such that when a component listener is
        // triggered, data is available only for those components that precede it in the
        // rendering order. So wait until formSubmit, at which time all data will be there.

        manageServices(getServicesToStart(), SipxProcessContext.Command.START);
        manageServices(getServicesToStop(), SipxProcessContext.Command.STOP);
        manageServices(getServicesToRestart(), SipxProcessContext.Command.RESTART);
    }

    private void manageServices(Collection services, SipxProcessContext.Command operation) {
        if (services == null) {
            // nothing to do
            return;
        }
        String[] names = (String[]) services.toArray(new String[services.size()]);
        getSipxProcessContext().manageServices(names, operation);
    }
}
