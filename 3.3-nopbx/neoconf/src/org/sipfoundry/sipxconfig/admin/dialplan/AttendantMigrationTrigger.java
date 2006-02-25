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

import org.sipfoundry.sipxconfig.common.InitializationTask;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class AttendantMigrationTrigger implements ApplicationListener {
    private AttendantMigrationContext m_attendantMigrationContext;

    public void setAttendantMigrationContext(AttendantMigrationContext attendantMigrationContext) {
        m_attendantMigrationContext = attendantMigrationContext;
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof InitializationTask) {
            InitializationTask task = (InitializationTask) event;
            String taskName = task.getTask();
            if ("dial_plan_migrate_attendant_rules".equals(taskName)) {
                m_attendantMigrationContext.migrateAttendantRules();
            }
        }
    }
}
