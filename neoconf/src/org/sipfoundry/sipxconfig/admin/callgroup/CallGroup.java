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


public class CallGroup extends AbstractCallSequence {
    private boolean m_enabled;
    private String m_name;
    private String m_extension;
    private String m_lineName;
    private String m_description;

    public CallGroup() {
        // bean usage only
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    public String getLineName() {
        return m_lineName;
    }

    public void setLineName(String lineName) {
        m_lineName = lineName;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }
    
    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }
    
    /**
     * inserts a new ring for a specific user
     * @param user 
     * @return newly created user ring
     */
    public UserRing insertRing(User user) {
        UserRing ring = new UserRing();
        ring.setCallGroup(this);
        ring.setUser(user);
        insertRing(ring);
        return ring;
    }
}

