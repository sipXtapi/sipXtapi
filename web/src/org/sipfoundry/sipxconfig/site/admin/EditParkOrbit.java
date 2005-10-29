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

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.callback.ICallback;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupManager;
import org.sipfoundry.sipxconfig.admin.callgroup.ParkOrbit;
import org.sipfoundry.sipxconfig.components.StringSizeValidator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditParkOrbit extends BasePage implements PageRenderListener {
    public static final String PAGE = "EditParkOrbit";

    public abstract CallGroupManager getCallGroupContext();

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
            CallGroupManager context = getCallGroupContext();
            orbit = context.loadParkOrbit(id);
        } else {
            orbit = new ParkOrbit();
        }
        setParkOrbit(orbit);
        
        // If no callback was set before navigating to this page, then by
        // default, go back to the ListParkOrbits page
        if (getCallback() == null) {
            setCallback(new PageCallback(ListParkOrbits.PAGE));
        }
    }

    public void commit(IRequestCycle cycle_) {
        if (!isValid()) {
            return;
        }

        saveValid();
    }

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        AbstractComponent component = (AbstractComponent) getComponent("common");
        StringSizeValidator descriptionValidator = (StringSizeValidator) component.getBeans()
                .getBean("descriptionValidator");
        descriptionValidator.validate(delegate);

        return !delegate.getHasErrors();
    }

    private void saveValid() {
        CallGroupManager context = getCallGroupContext();
        ParkOrbit orbit = getParkOrbit();
        context.storeParkOrbit(orbit);
        context.activateParkOrbits();
        Integer id = getParkOrbit().getId();
        setParkOrbitId(id);
    }
}
