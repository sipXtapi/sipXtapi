/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import org.aopalliance.intercept.MethodInterceptor;
import org.aopalliance.intercept.MethodInvocation;

/**
 * Cache returns same copy of settings, this will return a copy of the cached
 * copy which inturn avoids reparsing xml file.
 */
public class CopyFromCacheInterceptor implements MethodInterceptor {
    private MethodInterceptor m_cacheInterceptor;

    public void setCacheInterceptor(MethodInterceptor cacheInterceptor) {
        m_cacheInterceptor = cacheInterceptor;
    }

    public Object invoke(MethodInvocation arg0) throws Throwable {
        Setting cached = (Setting) m_cacheInterceptor.invoke(arg0);
        Setting copy = cached.copy();
        return copy;
    }    
}
