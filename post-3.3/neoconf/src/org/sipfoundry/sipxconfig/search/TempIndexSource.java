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
import java.io.IOException;

import org.apache.lucene.store.Directory;
import org.apache.lucene.store.RAMDirectory;

/**
 * Memory only indexing - suitable for testing TempIndexSource
 */
public class TempIndexSource extends SimpleIndexSource {
    private Directory m_ramDirectory;

    protected Directory createDirectory(File file_, boolean createDirectory) throws IOException {
        if (m_ramDirectory == null || createDirectory) {
            m_ramDirectory = new RAMDirectory();
        }
        return m_ramDirectory;
    }
}
