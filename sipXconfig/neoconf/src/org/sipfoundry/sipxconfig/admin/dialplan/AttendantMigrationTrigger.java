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

import org.sipfoundry.sipxconfig.common.InitTaskListener;

public class AttendantMigrationTrigger extends InitTaskListener {
    private AttendantMigrationContext m_attendantMigrationContext;

    public void setAttendantMigrationContext(AttendantMigrationContext attendantMigrationContext) {
        m_attendantMigrationContext = attendantMigrationContext;
    }

    @Override
    public void onInitTask(String task) {
        if ("dial_plan_migrate_attendant_rules".equals(task)) {
            m_attendantMigrationContext.migrateAttendantRules();
        } else if ("attendant_defaults".equals(task)) {
            m_attendantMigrationContext.setAttendantDefaults();
        }
    }
}
