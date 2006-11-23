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

import java.io.IOException;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.services.ResponseRenderer;

public class EmptyResponseRenderer implements ResponseRenderer {

    public void renderResponse(IRequestCycle cycle) throws IOException {
        // do nothing - it is used by services that pre-render response in listeners
    }

}
