/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.components.service;

import org.apache.tapestry.engine.DirectService;

public class ExportService extends DirectService {

    public static final String SERVICE_NAME = "export";

    public String getName() {
        return SERVICE_NAME;
    }
}
