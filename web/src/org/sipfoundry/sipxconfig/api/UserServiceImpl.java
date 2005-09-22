
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

import java.lang.reflect.InvocationTargetException;
import java.util.Map;

import org.apache.commons.beanutils.BeanUtils;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;


public class UserServiceImpl implements UserService {
    
    private CoreContext m_coreContext;
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    public void echo() {
        System.out.println("nop");
    }

    public void createUser(Map properties) {
        System.out.println("createuser");
        User user = new User();
        try {
            BeanUtils.populate(user, properties);
            m_coreContext.saveUser(user);
        } catch (IllegalAccessException iae) {
        } catch (InvocationTargetException e) {            
        }
    }
}
