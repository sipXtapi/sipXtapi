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
package org.sipfoundry.sipxconfig.site.user;

import java.io.InputStream;

import javax.servlet.http.HttpServletResponse;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.resource.MethodNotAllowedException;
import org.sipfoundry.sipxconfig.resource.ResourceContext;
import org.sipfoundry.sipxconfig.resource.ResourceException;
import org.sipfoundry.sipxconfig.resource.ResourceHandler;

public class UserResource implements ResourceHandler {
    
    private CoreContext m_coreContext;
    
    public InputStream deleteResource(ResourceContext context, InputStream in_) {
        User user = getUser(context.getUri());
        m_coreContext.deleteUser(user);

        return null;
    }

    public InputStream postResource(ResourceContext context_, InputStream in_) {
        throw new MethodNotAllowedException("POST");
    }

    public InputStream putResource(ResourceContext context, InputStream in) {
        User user = getUser(context.getUri());
        context.getTranslator().merge(in, user); 
        m_coreContext.saveUser(user);
        
        return null;
    }
    
    String fromEnd(String uri, int fromEnd) {
        String [] tokens = uri.split("/");
        return tokens[(tokens.length - 1) - fromEnd];
    }
    
    User getUser(String uri) {
        String displayId = fromEnd(uri, 0);
        User user = m_coreContext.loadUserByDisplayId(displayId);
        if (user == null) {
            throw new ResourceException(HttpServletResponse.SC_NOT_FOUND, "User with displayId not found : " 
                    + displayId);
        }
        
        return user;        
    }

    public InputStream getResource(ResourceContext context, InputStream in_) {
        User user = getUser(context.getUri());        
        return context.getTranslator().write(user);
    }

    public InputStream onResource(ResourceContext context_, String method, InputStream in_) {
        throw new MethodNotAllowedException(method);
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

}
