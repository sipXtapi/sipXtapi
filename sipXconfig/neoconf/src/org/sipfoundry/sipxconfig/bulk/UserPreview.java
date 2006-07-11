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
package org.sipfoundry.sipxconfig.bulk;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.User;

/**
 * Used to preview users that are imported but not added to the database
 */
public class UserPreview {
    private User m_user;
    private Collection<String> m_groupNames;

    public UserPreview(User user, Collection<String> groupNames) {
        m_user = user;
        m_groupNames = groupNames;
    }

    public User getUser() {
        return m_user;
    }

    public Collection<String> getGroupNames() {
        return m_groupNames;
    }
}
