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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import org.apache.commons.lang.enum.Enum;

/**
 * Permission
 */
public final class Permission extends Enum {
    public static final Permission NO_ACCESS = new Permission("NoAccess");
    public static final Permission VOICEMAIL = new Permission("Voicemail");
    public static final Permission RESTRICTED_DIALING = new Permission("900Dialing");
    public static final Permission LONG_DISTANCE_DIALING = new Permission("LongDistanceDialing");
    public static final Permission INTERNATIONAL_DIALING = new Permission("InternationalDialing");
    public static final Permission LOCAL_DIALING = new Permission("LocalDialing");

    private Permission(String permision) {
        super(permision);
    }
}
