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
import org.springframework.beans.factory.BeanFactory;
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
        return global;
    }

    /**
     * Retrieves Spring application context setupForRequest must have been called first, but this
     * should be accessable by any pages.
     * 
     * @deprecated use ognl:global.beanName instead or global.getApplicationContext() directly
     */
    public BeanFactory getBeanFactory() {
        BeanFactoryGlobals global = (BeanFactoryGlobals) getGlobal();
        return global.getApplicationContext();
    }
}
