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

import org.apache.tapestry.IExternalPage;
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
public abstract class UserCallForwarding extends BasePage implements IExternalPage,
        PageRenderListener {
    private static final String ACTION_ADD = "add";
    private static final String ACTION_APPLY = "apply";

    public abstract ForwardingContext getForwardingContext();

    public abstract CallSequence getCallSequence();

    public abstract void setCallSequence(CallSequence callSequence);

    public abstract ListEditMap getRingsMap();

    public abstract void setRingsMap(ListEditMap map);

    public abstract Ring getRing();

    public abstract void setRing(Ring ring);

    public abstract int getUserId();

    public abstract void setUserId(int userId);

    public abstract String getAction();

    public void activateExternalPage(Object[] parameters, IRequestCycle cycle_) {
        String userIdAsString = (String) parameters[0];
        int userId = Integer.parseInt(userIdAsString);
        setUserId(userId);
    }

    public void pageBeginRender(PageEvent event_) {
        CallSequence callSequence = getCallSequence();
        if (null == callSequence) {
            ForwardingContext forwardingContext = getForwardingContext();
            int userId = getUserId();
            callSequence = forwardingContext.getCallSequenceForUserId(userId);
            setCallSequence(callSequence);
        }
        ListEditMap map = createListEditMap(callSequence);
        setRingsMap(map);
    }

    /**
     * Creates edit map for a collection of rings
     * 
     * @param callSequence
     * @return newly created map
     */
    private ListEditMap createListEditMap(CallSequence callSequence) {
        ListEditMap map = new ListEditMap();
        Collection calls = callSequence.getCalls();
        for (Iterator i = calls.iterator(); i.hasNext();) {
            BeanWithId bean = (BeanWithId) i.next();
            map.add(bean.getId(), bean);
        }
        return map;
    }

    public void submit(IRequestCycle cycle_) {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        if (ACTION_ADD.equals(getAction())) {
            CallSequence callSequence = getCallSequence();
            callSequence.insertRing();
            getForwardingContext().saveCallSequence(getCallSequence());
        }
        if (ACTION_APPLY.equals(getAction())) {
            getForwardingContext().saveCallSequence(getCallSequence());
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

    public void deleteRing(IRequestCycle cycle) {
        Integer id = (Integer) TapestryUtils.assertParameter(Integer.class, cycle
                .getServiceParameters(), 0);
        Ring ring = getForwardingContext().getRing(id);
        CallSequence callSequence = ring.getCallSequence();
        callSequence.removeRing(ring);
    }
}
