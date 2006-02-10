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
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.ListEditMap;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.Ring;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Permission;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.login.LoginContext;
import org.sipfoundry.sipxconfig.site.UserSession;
import org.sipfoundry.sipxconfig.site.user.ManageUsers;

/**
 * UserCallForwarding
 */
public abstract class UserCallForwarding extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "UserCallForwarding";
    private static final String ACTION_ADD = "add";

    public abstract ForwardingContext getForwardingContext();

    public abstract LoginContext getLoginContext();

    public abstract CoreContext getCoreContext();

    public abstract CallSequence getCallSequence();

    public abstract void setCallSequence(CallSequence callSequence);

    public abstract User getUser();

    public abstract void setUser(User user);

    public abstract ListEditMap getRingsMap();

    public abstract void setRingsMap(ListEditMap map);

    public abstract Ring getRing();

    public abstract void setRing(Ring ring);

    public abstract Integer getUserId();

    public abstract void setUserId(Integer userId);

    public abstract String getAction();
    
    public abstract UserSession getUserSession();

    public void pageBeginRender(PageEvent event_) {
        CallSequence callSequence = getCallSequence();
        if (callSequence != null) {
            return;
        }

        Integer userId = getUserId();
        Integer loggedInUserId = getUserSession().getUserId();

        if (userId == null) {
            // No userId has been set yet, so make it the logged-in user
            userId = loggedInUserId;
        } else {
            // If the userId is not that of the logged-in user, then make sure
            // that the logged-in user has admin privileges. If not, then
            // force the userId to be the one for the logged-in user, so non-admin
            // users can only see/modify their own settings.
            if (!userId.equals(loggedInUserId)) {
                if (!getLoginContext().isAdmin(loggedInUserId)) {
                    userId = loggedInUserId;
                }
            }
        }

        setUserId(userId);

        User user = getCoreContext().loadUser(userId);
        setUser(user);

        ForwardingContext forwardingContext = getForwardingContext();
        callSequence = forwardingContext.getCallSequenceForUserId(userId);
        setCallSequence(callSequence);

        ListEditMap map = createListEditMap(callSequence);
        setRingsMap(map);

        if (getCallback() == null && getUserSession().isAdmin()) {
            setCallback(new PageCallback(ManageUsers.PAGE));
        }        
    }

    /**
     * Creates edit map for a collection of rings
     * 
     * @param callSequence
     * @return newly created map
     */
    private ListEditMap createListEditMap(CallSequence callSequence) {
        ListEditMap map = new ListEditMap();
        Collection calls = callSequence.getRings();
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
            // read saved rings from database
            setCallSequence(null);
        }
    }

    public void commit(IRequestCycle cycle_) {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        getForwardingContext().saveCallSequence(getCallSequence());
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
                .getListenerParameters(), 0);
        ForwardingContext forwardingContext = getForwardingContext();
        Ring ring = forwardingContext.getRing(id);
        CallSequence callSequence = ring.getCallSequence();
        callSequence.removeRing(ring);
        forwardingContext.saveCallSequence(callSequence);
    }

    public String getFirstCallMsg() {
        Object[] params = {
            getUser().getUserName()
        };
        return format("msg.first", params);
    }

    public boolean getHasVoiceMail() {
        return getUser().hasPermission(Permission.VOICEMAIL);
    }
}
