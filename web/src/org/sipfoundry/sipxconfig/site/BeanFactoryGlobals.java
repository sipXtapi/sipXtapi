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
package org.sipfoundry.sipxconfig.site;

import java.util.Collection;
import java.util.Map;
import java.util.Set;

import org.springframework.web.context.WebApplicationContext;

/**
 * BeanFactoryGlobals
 * 
 * special bean factory to be used as tapestry globals
 * 
 * in ognl use global.beanName to retrieve Spring beans
 * global.sipXconfigContext.getBean("beanName") also works for compatibility 
 */
public class BeanFactoryGlobals implements Map {
    private WebApplicationContext m_applicationContext;

    public WebApplicationContext getApplicationContext() {
        return m_applicationContext;
    }

    public void setApplicationContext(WebApplicationContext applicationContext) {
        m_applicationContext = applicationContext;
    }

    // TODO: this function is used temporarily to support global.sipXconfigContext OGNL
    // expressions
    public Object get(Object key) {
        if (SipxconfigEngine.BEANFACTORY_CONTEXT_KEY.equals(key)) {
            return m_applicationContext;
        }
        return m_applicationContext.getBean((String) key);
    }

    // map implementation - mostly unsupported operations
    public int size() {
        return m_applicationContext.getBeanDefinitionNames().length;
    }

    public boolean isEmpty() {
        return size() > 0;
    }

    public boolean containsKey(Object key) {
        return m_applicationContext.containsBeanDefinition((String) key);
    }

    public boolean containsValue(Object value_) {
        throw new UnsupportedOperationException();
    }

    public Collection values() {
        throw new UnsupportedOperationException();
    }

    public void putAll(Map t_) {
        throw new UnsupportedOperationException();
    }

    public Set entrySet() {
        throw new UnsupportedOperationException();
    }

    public Set keySet() {
        throw new UnsupportedOperationException();
    }

    public Object remove(Object key_) {
        throw new UnsupportedOperationException();
    }

    public Object put(Object key_, Object value_) {
        throw new UnsupportedOperationException();
    }

    public void clear() {
        throw new UnsupportedOperationException();
    }
}
