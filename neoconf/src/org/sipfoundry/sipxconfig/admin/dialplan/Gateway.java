/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import org.sipfoundry.sipxconfig.phone.PhoneContext;

/**
 * Gateway
 */
public class Gateway extends BeanWithId {
    private String m_name;
    private String m_address;
    private String m_description;

    public Gateway() {
        super(new Integer(PhoneContext.UNSAVED_ID));
    }

    /**
     * Only to be used by DialPlanManager
     * @param id of the existing gateway
     */
    public Gateway(Integer id) {
        super(id);
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getAddress() {
        return m_address;
    }

    public void setAddress(String address) {
        m_address = address;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }
}
