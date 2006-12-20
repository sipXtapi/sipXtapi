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
package org.sipfoundry.sipxconfig.acd;

import org.sipfoundry.sipxconfig.common.InitializationTask;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class AcdMigrationTrigger implements ApplicationListener {
    private AcdContext m_acdContext;

    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }

    public void onApplicationEvent(ApplicationEvent event) {
        if (event instanceof InitializationTask) {
            InitializationTask task = (InitializationTask) event;
            String taskName = task.getTask();
            if ("acd_migrate_line_extensions".equals(taskName)) {
                m_acdContext.migrateLineExtensions();
            } else if ("acd_migrate_overflow_queues".equals(taskName)) {
                m_acdContext.migrateOverflowQueues();
            }
        }
    }
}
