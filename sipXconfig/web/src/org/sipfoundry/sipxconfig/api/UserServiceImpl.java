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
 */
package org.sipfoundry.sipxconfig.api;

import java.beans.PropertyDescriptor;
import java.lang.reflect.InvocationTargetException;

import org.apache.commons.beanutils.PropertyUtils;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class UserServiceImpl implements UserService {

    private CoreContext m_coreContext;
    
    public void createUser(String userName, String pin) {
        User u = new User();
        u.setUserName(userName);
        u.setPin(pin, m_coreContext.getAuthorizationRealm());
        m_coreContext.saveUser(u);
    }
    
    public void deleteUser(String userName) {
        User u = requireUser(userName);
        m_coreContext.deleteUser(u);
    }
    
    public String[] getUserProperties() {
        return getObjectProperties(User.class);
    }
    
    private String[] getObjectProperties(Class c) {
        PropertyDescriptor[] properties = PropertyUtils.getPropertyDescriptors(c);
        String[] names = new String[properties.length];
        for (int i = 0; i < properties.length; i++) {
            names[i] = properties[i].getName();
        }
        return names;
    }
    
    public void setUserProperty(String userName, String property, Object value) {
        User u = requireUser(userName);
        setProperty(u, property, value);
    }
    
    private void setProperty(Object o, String property, Object value) {
        try {
            PropertyUtils.setSimpleProperty(o, property, value);
        } catch (IllegalAccessException e) {
            throw new IllegalArgumentException(e.getMessage());
        } catch (InvocationTargetException e) {
            throw new IllegalArgumentException(e.getMessage());
        } catch (NoSuchMethodException e) {
            throw new IllegalArgumentException(e.getMessage());
        }        
    }
    
    private User requireUser(String username) {
        User u = m_coreContext.loadUserByUserName(username);
        if (u == null) {
            throw new IllegalArgumentException("Could not find user with username '" + username + "'");
        }
        
        return u;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
