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
package org.sipfoundry.sipxconfig.device;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.EnumUserType;

/**
 * For devices that need to distinguish version can enumerate versions
 */
public final class DeviceVersion extends Enum {
    
    private String m_id;
    
    public DeviceVersion(String id) {
        super(id);
        m_id = id;
    }
    
    public String getId() {
        return m_id;
    }

    /** For hibernate mapping */
    public static class Type extends EnumUserType {
        public Type() {
            super(DeviceVersion.class);
        }
    }    
}
