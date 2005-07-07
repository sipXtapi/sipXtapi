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
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.form.ListEditMap;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRouting;
import org.sipfoundry.sipxconfig.admin.dialplan.RoutingException;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

/**
 * EditEmergencyRouting
 */
public abstract class EditEmergencyRouting extends BasePage implements PageRenderListener {

    public abstract DialPlanContext getDialPlanManager();

    public abstract EmergencyRouting getEmergencyRouting();

    public abstract void setEmergencyRouting(EmergencyRouting emergencyRouting);

    public abstract ListEditMap getExceptionsMap();

    public abstract void setExceptionsMap(ListEditMap map);

    public abstract void setExceptionItem(RoutingException exception);

    public void pageBeginRender(PageEvent event_) {
        EmergencyRouting emergencyRouting = getEmergencyRouting();
        if (emergencyRouting == null) {
            emergencyRouting = getDialPlanManager().getEmergencyRouting();
            setEmergencyRouting(emergencyRouting);
        }
        Collection exceptions = emergencyRouting.getExceptions();
        ListEditMap map = new ListEditMap();
        for (Iterator i = exceptions.iterator(); i.hasNext();) {
            RoutingException exception = (RoutingException) i.next();
            map.add(exception.getId(), exception);
        }
        setExceptionsMap(map);
    }

    public void formSubmit(IRequestCycle cycle_) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
    }

    public void apply(IRequestCycle cycle_) {
        if (TapestryUtils.isValid(this)) {
            getDialPlanManager().applyEmergencyRouting();
        }
    }

    /**
     * Called by ListEdit component to retrieve exception object associated with a specific id
     */
    public void synchronizeExceptionItem(IRequestCycle cycle_) {
        ListEditMap exceptionsMap = getExceptionsMap();
        RoutingException exception = (RoutingException) exceptionsMap.getValue();

        if (null == exception) {
            TapestryUtils.staleLinkDetected(this);
        } else {
            setExceptionItem(exception);
        }
    }

    public void addException(IRequestCycle cycle_) {
        EmergencyRouting emergencyRouting = getEmergencyRouting();
        emergencyRouting.addException(new RoutingException());
    }

    public void deleteException(IRequestCycle cycle) {
        Integer id = (Integer) TapestryUtils.assertParameter(Integer.class, cycle
                .getServiceParameters(), 0);
        getEmergencyRouting().removeException(id);
    }
}
