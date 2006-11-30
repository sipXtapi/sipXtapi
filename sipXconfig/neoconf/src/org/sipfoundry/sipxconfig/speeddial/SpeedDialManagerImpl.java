/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import java.util.List;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.UserDeleteListener;

public class SpeedDialManagerImpl extends SipxHibernateDaoSupport<SpeedDial> implements
        SpeedDialManager {

    private CoreContext m_coreContext;

    public SpeedDial getSpeedDialForUserId(Integer userId) {
        List<SpeedDial> speeddials = findSpeedDialForUserId(userId);
        if (!speeddials.isEmpty()) {
            return speeddials.get(0);
        }
        SpeedDial speedDial = new SpeedDial();
        speedDial.setUser(m_coreContext.loadUser(userId));
        saveSpeedDial(speedDial);
        return speedDial;
    }

    private List<SpeedDial> findSpeedDialForUserId(Integer userId) {
        List<SpeedDial> speeddials = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "speedDialForUserId", "userId", userId);
        return speeddials;
    }

    public void saveSpeedDial(SpeedDial speedDial) {
        getHibernateTemplate().saveOrUpdate(speedDial);
    }

    public UserDeleteListener createUserDeleteListener() {
        return new OnUserDelete();
    }

    private class OnUserDelete extends UserDeleteListener {
        protected void onUserDelete(User user) {
            List<SpeedDial> spedDials = findSpeedDialForUserId(user.getId());
            getHibernateTemplate().deleteAll(spedDials);
        }
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
