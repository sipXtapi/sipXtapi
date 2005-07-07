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
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.resource.MethodNotAllowedException;
import org.sipfoundry.sipxconfig.resource.ResourceContext;
import org.sipfoundry.sipxconfig.resource.ResourceHandler;

/**
 * Manage lists of users
 */
public class ManageUsersResource implements ResourceHandler {

    private CoreContext m_coreContext;

    public CoreContext getCoreContext() {
        return m_coreContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    /**
     * Adds all posted users, then returns same list of users but with updated fields
     */
    public InputStream postResource(ResourceContext context, InputStream in) {
        List users = (List) context.getTranslator().read(in);
        Iterator i = users.iterator();
        while (i.hasNext()) {
            User newUser = (User) i.next();
            m_coreContext.saveUser(newUser);
        }

        return null;
    }

    /**
     * Returns all users
     */
    public InputStream getResource(ResourceContext context, InputStream in_) {
        List users = m_coreContext.loadUserByTemplateUser(new User());
        return context.getTranslator().write(users);
    }
    
    public InputStream deleteResource(ResourceContext context_, InputStream in_) {
        throw new MethodNotAllowedException("DELETE");
    }

    public InputStream putResource(ResourceContext context_, InputStream in_) {
        throw new MethodNotAllowedException("PUT");
    }

    public InputStream onResource(ResourceContext context_, String method, InputStream in_) {
        throw new MethodNotAllowedException(method);
    }
}
