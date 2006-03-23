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
package org.sipfoundry.sipxconfig.presence;

import org.apache.commons.lang.enums.Enum;


public final class PresenceStatus extends Enum {     
    public static final PresenceStatus OPEN = new PresenceStatus("open");
    public static final PresenceStatus CLOSED = new PresenceStatus("closed");
    private PresenceStatus(String name) {
        super(name);
    }
    public static PresenceStatus resolve(String name) { 
        return (PresenceStatus) getEnum(PresenceStatus.class, name);
    }
}
