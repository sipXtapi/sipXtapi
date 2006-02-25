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
 * 
 * Incarnation from http://sourceforge.net/projects/diaphragma/ project
 */
package org.sipfoundry.sipxconfig.site;

import org.apache.hivemind.events.RegistryShutdownListener;
import org.apache.tapestry.web.WebContext;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.web.context.WebApplicationContext;

/**
 * Glue between Tapestry and Spring using Hiveminds builtin Spring integration
 */
public class SpringBeanFactoryHolderImpl extends
        org.apache.hivemind.lib.impl.SpringBeanFactoryHolderImpl implements
        RegistryShutdownListener {

    private WebContext m_context;

    public void setContext(WebContext context) {
        m_context = context;
    }

    public WebContext getContext() {
        return m_context;
    }

    public BeanFactory getBeanFactory() {
        if (super.getBeanFactory() == null) {
            super.setBeanFactory(getWebApplicationContext(getContext()));
        }
        return super.getBeanFactory();
    }

    public static WebApplicationContext getWebApplicationContext(WebContext wc) {
        Object attr = wc
                .getAttribute(WebApplicationContext.ROOT_WEB_APPLICATION_CONTEXT_ATTRIBUTE);
        if (attr == null) {
            return null;
        }
        if (attr instanceof RuntimeException) {
            throw (RuntimeException) attr;
        }
        if (attr instanceof Error) {
            throw (Error) attr;
        }
        if (!(attr instanceof WebApplicationContext)) {
            throw new IllegalStateException(
                    "Root context attribute is not of type WebApplicationContext: " + attr);
        }
        return (WebApplicationContext) attr;
    }

    public void registryDidShutdown() {
        ((ConfigurableApplicationContext) super.getBeanFactory()).close();
    }
}
