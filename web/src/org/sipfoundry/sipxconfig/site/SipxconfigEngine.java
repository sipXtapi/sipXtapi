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

import javax.servlet.ServletContext;

import org.apache.tapestry.engine.BaseEngine;
import org.apache.tapestry.request.RequestContext;
import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.springframework.context.ApplicationContext;
import org.springframework.web.context.WebApplicationContext;
import org.springframework.web.context.support.WebApplicationContextUtils;

/**
 * Hook Spring into Tapestry's global application context
 */
public class SipxconfigEngine extends BaseEngine {
    private static final long serialVersionUID;

    static {
        serialVersionUID = 1L;
    }

    /**
     * Inserts application context in global.
     * 
     * In ognl use: <code>global.beanName</code> to get Spring beans. You can also use
     * <code>global.sipXconfigContext.getBean("beanName")</code>
     * 
     */
    protected Object createGlobal(RequestContext context) {
        BeanFactoryGlobals global = (BeanFactoryGlobals) super.createGlobal(context);
        ServletContext servletContext = context.getServlet().getServletContext();
        WebApplicationContext bf = WebApplicationContextUtils
                .getWebApplicationContext(servletContext);
        global.setApplicationContext(bf);
        
        initializeApplication(bf);
        
        return global;
    }

    /**
     * tell entire application, we're ready to run
     * IMPLEMENTATION NOTE: Alternative to putting this here would be to subclassing Springs
     * ContextLoaderListener but this next best choice w/o adding another class. Lazy or stingy,
     * you decide.
     */    
    private void initializeApplication(ApplicationContext app) {
        app.publishEvent(new ApplicationInitializedEvent(this));        
    }
}
