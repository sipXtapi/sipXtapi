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

import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbit;
import org.sipfoundry.sipxconfig.admin.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditParkOrbit extends BasePage implements PageBeginRenderListener {
    public static final String PAGE = "EditParkOrbit";

    public abstract ParkOrbitContext getParkOrbitContext();

    public abstract Integer getParkOrbitId();

    public abstract void setParkOrbitId(Integer id);

    public abstract ParkOrbit getParkOrbit();

    public abstract void setParkOrbit(ParkOrbit parkOrbit);

    public abstract ICallback getCallback();

    public abstract void setCallback(ICallback callback);

    public void pageBeginRender(PageEvent event_) {
        ParkOrbit orbit = getParkOrbit();
        if (null != orbit) {
            return;
        }
        Integer id = getParkOrbitId();
        if (null != id) {
            ParkOrbitContext context = getParkOrbitContext();
            orbit = context.loadParkOrbit(id);
        } else {
            orbit = getParkOrbitContext().newParkOrbit();
        }
        setParkOrbit(orbit);

        // If no callback was set before navigating to this page, then by
        // default, go back to the ListParkOrbits page
        if (getCallback() == null) {
            setCallback(new PageCallback(ListParkOrbits.PAGE));
        }
    }

    public void commit() {
        if (!isValid()) {
            return;
        }

        saveValid();
    }

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        return !delegate.getHasErrors();
    }

    private void saveValid() {
        ParkOrbitContext context = getParkOrbitContext();
        ParkOrbit orbit = getParkOrbit();
        context.storeParkOrbit(orbit);
        Integer id = getParkOrbit().getId();
        setParkOrbitId(id);
    }
}
