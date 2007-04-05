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

import org.sipfoundry.sipxconfig.common.InitTaskListener;

public class AcdMigrationTrigger extends InitTaskListener {
    private AcdContext m_acdContext;

    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
    }

    public void onInitTask(String task) {
        if ("acd_migrate_line_extensions".equals(task)) {
            m_acdContext.migrateLineExtensions();
        } else if ("acd_migrate_overflow_queues".equals(task)) {
            m_acdContext.migrateOverflowQueues();
        }
    }
}
