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

import java.util.Map;

import org.apache.tapestry.engine.BaseEngine;
import org.apache.tapestry.request.RequestContext;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

/**
 * Hook Spring into Tapestry's global application context
 */
public class SipxconfigEngine extends BaseEngine {

    private static final String APPLICATION_CONTEXT_KEY = "phoneContext";

    private static final long serialVersionUID;

    static {
        serialVersionUID = 1L;
    }

    /**
     * TODO: Unittest . No unittests for this at the moment. Waiting to see if we go w/tapestry
     * and how to create a valid RequestContect object. Quick attempts failed.
     */
    protected void setupForRequest(RequestContext context) {
        super.setupForRequest(context);

        // insert PhoneContext in global, if not there
        Map global = (Map) getGlobal();
        PhoneContext pc = (PhoneContext) global.get(APPLICATION_CONTEXT_KEY);
        if (pc == null) {
            BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
            BeanFactoryReference bf = bfl.useBeanFactory("sipxconfig-web");
            pc = (PhoneContext) bf.getFactory().getBean(APPLICATION_CONTEXT_KEY);
            if (pc == null) {
                throw new IllegalStateException("Could not create phone context");
            }
            global.put(APPLICATION_CONTEXT_KEY, pc);
        }
    }
}
