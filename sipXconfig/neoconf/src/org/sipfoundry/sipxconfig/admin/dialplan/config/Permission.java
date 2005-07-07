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
import org.sipfoundry.sipxconfig.common.EnumUserType;

/**
 * Permission
 */
public final class Permission extends Enum {
    public static final Permission NO_ACCESS = new Permission("NoAccess");
    public static final Permission VOICEMAIL = new Permission("Voicemail");
    public static final Permission RESTRICTED_DIALING = new Permission("_900Dialing");
    public static final Permission TOLL_FREE_DIALING = new Permission("TollFree");
    public static final Permission LONG_DISTANCE_DIALING = new Permission("LongDistanceDialing");
    public static final Permission INTERNATIONAL_DIALING = new Permission("InternationalDialing");
    public static final Permission LOCAL_DIALING = new Permission("LocalDialing");
    public static final Permission AUTO_ATTENDANT = new Permission("AutoAttendant");
    public static final Permission FORWARD_CALLS_EXTERNAL = new Permission("ForwardCallsExternal");
    public static final Permission RECORD_SYSTEM_PROMPTS = new Permission("RecordSystemPrompts");

    private Permission(String permision) {
        super(permision);
    }

    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(Permission.class);
        }
    }

    public static Permission getEnum(String permission) {
        return (Permission) getEnum(Permission.class, permission);
    }
}
