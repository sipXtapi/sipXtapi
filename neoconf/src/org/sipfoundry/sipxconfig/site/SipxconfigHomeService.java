/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site;

import java.io.IOException;

import javax.servlet.ServletException;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.engine.HomeService;
import org.apache.tapestry.engine.IEngineServiceView;
import org.apache.tapestry.request.ResponseOutputStream;

/**
 * TODO: Unclear I need this, remove if not
 */
public class SipxconfigHomeService extends HomeService {

    public void service(IEngineServiceView engine, IRequestCycle cycle,
            ResponseOutputStream output) throws ServletException, IOException {
        Visit visit = (Visit) engine.getVisit();
        if (visit != null) {
            visit.setFoobar("foorbar");
        }

        super.service(engine, cycle, output);
    }
}