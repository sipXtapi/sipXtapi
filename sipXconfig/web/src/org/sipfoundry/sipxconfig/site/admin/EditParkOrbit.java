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
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.IValidationDelegate;
import org.sipfoundry.sipxconfig.admin.callgroup.CallGroupContext;
import org.sipfoundry.sipxconfig.admin.callgroup.ParkOrbit;
import org.sipfoundry.sipxconfig.components.AssetSelector;
import org.sipfoundry.sipxconfig.components.StringSizeValidator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class EditParkOrbit extends BasePage implements PageRenderListener {
    public static final String PAGE = "EditParkOrbit";

    public abstract CallGroupContext getCallGroupContext();

    public abstract Integer getParkOrbitId();

    public abstract void setParkOrbitId(Integer id);

    public abstract ParkOrbit getParkOrbit();

    public abstract void setParkOrbit(ParkOrbit parkOrbit);

    public abstract boolean getCommitChanges();

    public void pageBeginRender(PageEvent event_) {
        ParkOrbit orbit = getParkOrbit();
        if (null != orbit) {
            return;
        }
        Integer id = getParkOrbitId();
        if (null != id) {
            CallGroupContext context = getCallGroupContext();
            orbit = context.loadParkOrbit(id);
        } else {
            orbit = new ParkOrbit();
        }
        setParkOrbit(orbit);
    }

    /**
     * Called when any of the submit componens on the form is activated.
     * 
     * Usually submit components are setting propertied. formSubmit will first check if the form
     * is valid, then it will call all the "action" listeners. Only one of the listeners (the one
     * that recongnizes the property that is set) will actually do something. This is a bit
     * strange consequnce of the fact that Tapestry listeners are pretty much usuless because they
     * are called while the form is still rewinding and not all changes are commited to beans.
     * 
     * @param cycle current request cycle
     */
    public void formSubmit(IRequestCycle cycle) {
        AssetSelector assetSelector = getAssetSelector();
        assetSelector.checkFileUpload();

        if (!isValid()) {
            return;
        }

        if (getCommitChanges()) {
            saveValid();
            cycle.activate(ListParkOrbits.PAGE);
        }
    }

    private boolean isValid() {
        IValidationDelegate delegate = TapestryUtils.getValidator(this);
        AbstractComponent component = (AbstractComponent) getComponent("common");
        StringSizeValidator descriptionValidator = (StringSizeValidator) component.getBeans()
                .getBean("descriptionValidator");
        descriptionValidator.validate(delegate);

        AssetSelector assetSelector = getAssetSelector();
        assetSelector.validateNotEmpty(delegate,
                "You must select an existing music on hold or upload a new one.");

        return !delegate.getHasErrors();
    }

    private AssetSelector getAssetSelector() {
        return (AssetSelector) getComponent("musicOnHoldSelector");
    }

    private void saveValid() {
        CallGroupContext context = getCallGroupContext();
        ParkOrbit orbit = getParkOrbit();
        context.storeParkOrbit(orbit);
        context.activateParkOrbits();
        Integer id = getParkOrbit().getId();
        setParkOrbitId(id);
    }
}
