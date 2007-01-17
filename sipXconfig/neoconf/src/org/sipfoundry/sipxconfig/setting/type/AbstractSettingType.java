/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting.type;

public abstract class AbstractSettingType implements SettingType {
    private String m_id;

    public AbstractSettingType() {
        super();
    }

    public void setId(String id) {
        m_id = id;
    }

    protected String getId() {
        return m_id;
    }

}
