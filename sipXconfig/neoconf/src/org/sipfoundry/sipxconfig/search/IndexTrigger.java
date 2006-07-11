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
package org.sipfoundry.sipxconfig.search;

import java.io.File;

import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.springframework.context.ApplicationEvent;
import org.springframework.context.ApplicationListener;

public class IndexTrigger implements ApplicationListener {
    private IndexManager m_indexManager;

    private boolean m_enabled = true;

    private File m_indexDirectory;

    public void onApplicationEvent(ApplicationEvent event) {
        if (!m_enabled) {
            return;
        }
        if (event instanceof ApplicationInitializedEvent) {
            if (!m_indexDirectory.exists()) {
                m_indexManager.indexAll();
            }
        }
    }

    public void setIndexManager(IndexManager indexManager) {
        m_indexManager = indexManager;
    }

    public void setIndexDirectoryName(String indexDirectoryName) {
        m_indexDirectory = new File(indexDirectoryName);
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }
}
