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
package org.sipfoundry.sipxconfig.site.user_portal;

import java.util.Collection;
import java.util.Iterator;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.form.ListEditMap;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.dialplan.BeanWithId;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.Ring;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

/**
 * UserCallForwarding
 */
public abstract class UserCallForwarding extends BasePage  implements PageRenderListener {
    
    public abstract ForwardingContext getForwardingContext();
    
    // FIXME: implement for a specific user
    public CallSequence getCallSequence() {
        ForwardingContext forwardingContext = getForwardingContext();
        return forwardingContext.getCallSequence(null);
    }
    
    public abstract ListEditMap getRingsMap();
    public abstract void setRingsMap(ListEditMap map);

    public abstract Ring getRing();
    public abstract void setRing(Ring ring);


    public void pageBeginRender(PageEvent event_) {
        CallSequence callSequence = getCallSequence();
        Collection calls = callSequence.getCalls();
        ListEditMap map = new ListEditMap();
        for (Iterator i = calls.iterator(); i.hasNext();) {
            BeanWithId bean = (BeanWithId) i.next();
            map.add(bean.getId(), bean);
        }
        setRingsMap(map);
    }
    
    public void apply(IRequestCycle cycle_) {
        if (TapestryUtils.isValid(this)) {
            //TODO: implement
        }
    }

    /**
     * Called by ListEdit component to retrieve exception object associated with a specific id
     */
    public void synchronizeRing(IRequestCycle cycle_) {
        ListEditMap ringsMap = getRingsMap();
        Ring ring = (Ring) ringsMap.getValue();

        if (null == ring) {
            TapestryUtils.staleLinkDetected(this);
        } else {
            setRing(ring);
        }
    }

    public void addRing(IRequestCycle cycle_) {
        CallSequence callSequence = getCallSequence();
        callSequence.addRing(new Ring());
    }

    public void deleteRing(IRequestCycle cycle) {
        Integer id = (Integer) TapestryUtils.assertParameter(Integer.class, cycle
                .getServiceParameters(), 0);
        CallSequence callSequence = getCallSequence();
        callSequence.removeRing(id);
    }
}
