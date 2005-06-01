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
package org.sipfoundry.sipxconfig.admin.callgroup;

import org.sipfoundry.sipxconfig.common.User;

public class UserRing extends AbstractRing {
    private CallGroup m_callGroup;
    private User m_user;

    public UserRing() {
        // bean only
    }

    protected Object getUserPart() {
        return m_user.getDisplayId();
    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public CallGroup getCallGroup() {
        return m_callGroup;
    }

    public void setCallGroup(CallGroup callGroup) {
        m_callGroup = callGroup;
    }

    /**
     * Checks if this ring is first in the sequence
     * 
     * @return true if this is the first ring
     */
    public boolean isFirst() {
        AbstractRing ring = (AbstractRing) getCallGroup().getCalls().get(0);
        return ring.equals(this);
    }
}
