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

import java.util.Iterator;
import java.util.List;

import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.admin.forwarding.Ring;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Permission;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.login.LoginContext;
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

    public abstract CallSequence getCallSequence();

    public abstract void setCallSequence(CallSequence callSequence);

    public abstract User getUser();

    public abstract void setUser(User user);

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

        if (getCallback() == null && getUserSession().isAdmin()) {
            setCallback(new PageCallback(ManageUsers.PAGE));
        }
    }

    public void submit() {
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

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            // do nothing on errors
            return;
        }
        getForwardingContext().saveCallSequence(getCallSequence());
    }

    public void deleteRing(Integer id) {
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
        return getMessages().format("msg.first", params);
    }

    public boolean getHasVoiceMail() {
        return getUser().hasPermission(Permission.VOICEMAIL);
    }

    public IPrimaryKeyConverter getConverter() {
        return new Converter(getCallSequence());
    }

    public static final class Converter implements IPrimaryKeyConverter {

        private final CallSequence m_callSequence;

        public Converter(CallSequence callSequence) {
            m_callSequence = callSequence;
        }

        public Object getPrimaryKey(Object objValue) {
            BeanWithId bean = (BeanWithId) objValue;
            return bean.getPrimaryKey();
        }

        public Object getValue(Object objPrimaryKey) {
            List rings = m_callSequence.getRings();
            for (Iterator i = rings.iterator(); i.hasNext();) {
                PrimaryKeySource bean = (PrimaryKeySource) i.next();
                if (bean.getPrimaryKey().equals(objPrimaryKey)) {
                    return bean;
                }
            }
            return null;
        }
    }
}
