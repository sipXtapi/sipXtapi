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

import javax.servlet.ServletContext;

import org.apache.tapestry.engine.BaseEngine;
import org.apache.tapestry.request.RequestContext;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.web.context.support.WebApplicationContextUtils;

/**
 * Hook Spring into Tapestry's global application context
 */
public class SipxconfigEngine extends BaseEngine {

    private static final String BEANFACTORY_CONTEXT_KEY = "sipXconfigContext";

    private static final long serialVersionUID;

    static {
        serialVersionUID = 1L;
    }

    /**
     * TODO: Unittest . No unittests for this at the moment. Waiting to see if
     * we go w/tapestry and how to create a valid RequestContect object. Quick
     * attempts failed.
     */
    protected void setupForRequest(RequestContext context) {
        super.setupForRequest(context);

        // insert PhoneContext in global, if not there
        Map global = (Map) getGlobal();
        // in ogml use: global.appContext.getBean("sipXconfigContext") to get
        // this object
        BeanFactory bf = (BeanFactory) global.get(BEANFACTORY_CONTEXT_KEY);
        if (bf == null) {
            ServletContext servletContext = context.getServlet().getServletContext();
            bf = WebApplicationContextUtils.getWebApplicationContext(servletContext);
            global.put(BEANFACTORY_CONTEXT_KEY, bf);
        }
    }
    
    /**
     * setupForRequest must have been called first, but this should be accessable
     * by any pages.
     */
    public  BeanFactory getBeanFactory() {
        // insert PhoneContext in global, if not there
        Map global = (Map) getGlobal();
        // in ogml use: global.appContext.getBean("sipXconfigContext") to get
        // this object
        return (BeanFactory) global.get(BEANFACTORY_CONTEXT_KEY);
    }
}
