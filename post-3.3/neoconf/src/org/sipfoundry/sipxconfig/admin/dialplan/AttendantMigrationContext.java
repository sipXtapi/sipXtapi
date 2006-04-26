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
package org.sipfoundry.sipxconfig.admin.dialplan;

public interface AttendantMigrationContext {
    public static final String CONTEXT_BEAN_NAME = "attendantMigrationContext";

    void migrateAttendantRules();
    
    void setAttendantDefaults();
}
