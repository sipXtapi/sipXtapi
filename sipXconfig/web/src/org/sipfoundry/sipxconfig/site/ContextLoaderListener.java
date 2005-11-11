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
import javax.servlet.ServletContextEvent;

import org.sipfoundry.sipxconfig.common.ApplicationInitializedEvent;
import org.springframework.web.context.WebApplicationContext;
import org.springframework.web.context.support.WebApplicationContextUtils;

/**
 * Override Spring's listener to send application initialized event
 */
public class ContextLoaderListener extends org.springframework.web.context.ContextLoaderListener {

    public void contextInitialized(ServletContextEvent event) {
        super.contextInitialized(event);
        ServletContext servletContext = event.getServletContext();
        WebApplicationContext bf = WebApplicationContextUtils
                .getWebApplicationContext(servletContext);
        // tell entire application, we're ready to run
        bf.publishEvent(new ApplicationInitializedEvent(this));        
    }
}
