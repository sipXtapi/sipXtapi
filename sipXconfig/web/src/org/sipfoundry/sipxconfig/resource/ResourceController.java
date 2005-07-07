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
package org.sipfoundry.sipxconfig.resource;


import java.io.InputStream;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.springframework.util.PathMatcher;
import org.springframework.web.servlet.ModelAndView;
import org.springframework.web.servlet.mvc.AbstractController;

/**
 * Separate the action taken based on the method of the HTTP message. Key to RESTful
 * applications 
 */
public class ResourceController extends AbstractController {
    
    private Properties m_handlers;
    
    private String m_resourceContext;
        
    public Properties getResourceHandlers() {
        return m_handlers;
    }

    public void setResourceHandlers(Properties handlers) {
        m_handlers = handlers;
    }

    public void setResourceContext(String resourceContext) {
        m_resourceContext = resourceContext;
    }

    protected ModelAndView handleRequestInternal(HttpServletRequest request,
            HttpServletResponse response) throws Exception {
        
        ModelAndView mv = null;
        try {
            InputStream in = null;
            String method = request.getMethod();
            String path = request.getPathInfo();
            ResourceHandler handler = getHandler(path);
            if (handler == null) {
                response.sendError(HttpServletResponse.SC_NO_CONTENT, "No handler registered for " + path);
            } else {
                ResourceContext context = (ResourceContext) getApplicationContext().getBean(m_resourceContext);
                context.setUri(path);
                if ("GET".equals(method)) {
                    in = handler.getResource(context, request.getInputStream());
                } else if ("POST".equals(method)) {
                    in = handler.postResource(context, request.getInputStream());
                } else if ("PUT".equals(method)) {
                    in = handler.putResource(context, request.getInputStream());
                } else if ("DELETE".equals(method)) {
                    in = handler.deleteResource(context, request.getInputStream());
                } else {
                    in = handler.onResource(context, method, request.getInputStream());
                }
                Map model = new HashMap();
                model.put(ResourceView.RESOURCE_STREAM, in);
                mv = new ModelAndView("xml", model);
            }
        } catch (ResourceException re) {
            response.sendError(re.getHttpErrorCode(), re.getMessage());
        }
        
        return mv;
    }
    
    private ResourceHandler getHandler(String urlPath) {
        String handlerBeanName = m_handlers.getProperty(urlPath);
        if (handlerBeanName == null) {
            for (Iterator i = m_handlers.keySet().iterator(); i.hasNext();) {
                String registeredPath = (String) i.next();
                if (PathMatcher.match(registeredPath, urlPath)) {
                    handlerBeanName = m_handlers.getProperty(registeredPath);
                }
            }
        }

        ResourceHandler handler = null;
        if (handlerBeanName != null) {
            handler = (ResourceHandler) getApplicationContext().getBean(handlerBeanName);
        }
        
        return handler;
    }
}
