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
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.File;
import java.io.Reader;

public interface BulkManager {
    public static final String CONTEXT_BEAN_NAME = "bulkManager";

    void insertFromCsv(Reader reader);
    
    void insertFromCsv(File file, boolean deleteOnImport);
}
