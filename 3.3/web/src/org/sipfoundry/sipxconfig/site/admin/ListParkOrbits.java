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
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collection;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class ListParkOrbits extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "ListParkOrbits";

    public abstract ParkOrbitContext getParkOrbitContext();

    public abstract CoreContext getCoreContext();

    public void pageBeginRender(PageEvent event_) {
        // initialize properties
    }

    public abstract ParkOrbit getCurrentRow();

    public abstract void setCurrentRow(ParkOrbit parkOrbit);

    public abstract Collection getRowsToDelete();

    public void add(IRequestCycle cycle) {
        EditParkOrbit editPage = (EditParkOrbit) cycle.getPage(EditParkOrbit.PAGE);
        editPage.setParkOrbitId(null);
        editPage.setParkOrbit(null);
        cycle.activate(editPage);
    }

    public void edit(IRequestCycle cycle) {
        EditParkOrbit editPage = (EditParkOrbit) cycle.getPage(EditParkOrbit.PAGE);
        Integer callGroupId = TapestryUtils.getBeanId(cycle);
        editPage.setParkOrbitId(callGroupId);
        editPage.setParkOrbit(null);
        cycle.activate(editPage);
    }

    public void formSubmit(IRequestCycle cycle_) {
        delete();
    }

    public void activate(IRequestCycle cycle_) {
        getParkOrbitContext().activateParkOrbits();
    }

    /**
     * Deletes all selected rows (on this screen deletes call groups).
     */
    private void delete() {
        Collection selectedRows = getRowsToDelete();
        if (null != selectedRows) {
            getParkOrbitContext().removeParkOrbits(selectedRows);
        }
    }
}
