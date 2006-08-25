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

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.Ring;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.login.LoginContext;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.site.UserSession;
import org.sipfoundry.sipxconfig.site.user.ManageUsers;

/**
 * UserCallForwarding
 */
public abstract class UserCallForwarding extends PageWithCallback implements
        PageBeginRenderListener {
    public static final String PAGE = "UserCallForwarding";
    private static final String ACTION_ADD = "add";

    public abstract ForwardingContext getForwardingContext();

    public abstract LoginContext getLoginContext();

    public abstract CoreContext getCoreContext();

    public abstract Integer getUserId();

    public abstract void setUserId(Integer userId);

    public abstract String getAction();

    public abstract UserSession getUserSession();

    public abstract List getRings();

    public abstract void setRings(List rings);

    public void pageBeginRender(PageEvent event_) {
        if (getRings() != null) {
            return;
        }

        Integer userId = getActiveUserId();
        setUserId(userId);

        List rings = createDetachedRingList(getCallSequence());
        setRings(rings);

        if (getCallback() == null && getUserSession().isAdmin()) {
            setCallback(new PageCallback(ManageUsers.PAGE));
        }
    }

    /**
     * Determine the id of the user for which page will be changing call forwarding setting.
     * 
     * If current login user has admin privilidges he can change call forwarding for any user.
     * However user without admin privilidges can only edit settings for logged in user.
     * 
     * @return id of the user for which page will be changing call forwarding setting
     */
    private Integer getActiveUserId() {
        Integer userId = getUserId();
        Integer loginUserId = getUserSession().getUserId();

        if (userId == null) {
            // No userId has been set yet, so make it the logged-in user
            return loginUserId;
        }
        // if they are the same it does not matter which one we return
        if (userId.equals(loginUserId)) {
            return userId;
        }

        // If the userId is not that of the logged-in user, then make sure
        // that the logged-in user has admin privileges. If not, then
        // force the userId to be the one for the logged-in user, so non-admin
        // users can only see/modify their own settings.
        if (getLoginContext().isAdmin(loginUserId)) {
            return userId;
        }
        return loginUserId;
    }

    /**
     * Create list of rings that is going to be stored in session.
     * 
     * The list is a clone of the list kept by current call sequence, ring objects do not have
     * valid ids and their call sequence field is set to null.
     */
    private List createDetachedRingList(CallSequence callSequence) {
        List rings = callSequence.getRings();
        List list = new ArrayList();
        for (Iterator i = rings.iterator(); i.hasNext();) {
            BeanWithId ring = (BeanWithId) i.next();
            Ring dup = (Ring) ring.duplicate();
            dup.setCallSequence(null);
            list.add(dup);
        }
        return list;
    }

    private CallSequence getCallSequence() {
        ForwardingContext forwardingContext = getForwardingContext();
        Integer userId = getUserId();
        return forwardingContext.getCallSequenceForUserId(userId);
    }

    public void submit() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        if (ACTION_ADD.equals(getAction())) {
            getRings().add(new Ring());
        }
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        CallSequence callSequence = getCallSequence();
        callSequence.clear();
        callSequence.insertRings(getRings());
        getForwardingContext().saveCallSequence(callSequence);
    }

    public void deleteRing(int position) {
        getRings().remove(position);
    }

    public String getFirstCallMsg() {
        Object[] params = {
            getUser().getUserName()
        };
        return getMessages().format("msg.first", params);
    }

    public boolean getHasVoiceMail() {
        return getUser().hasPermission(Permission.VOICEMAIL);
    }

    public User getUser() {
        Integer userId = getUserId();
        User user = getCoreContext().loadUser(userId);
        return user;
    }
}
