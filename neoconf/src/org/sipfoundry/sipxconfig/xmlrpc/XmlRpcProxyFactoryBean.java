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
package org.sipfoundry.sipxconfig.xmlrpc;

import java.net.MalformedURLException;

import org.springframework.aop.framework.ProxyFactory;
import org.springframework.beans.factory.FactoryBean;

public class XmlRpcProxyFactoryBean extends XmlRpcClientInterceptor implements FactoryBean {

    private Object m_serviceProxy;

    public void afterPropertiesSet() throws MalformedURLException {
        super.afterPropertiesSet();
        m_serviceProxy = ProxyFactory.getProxy(getServiceInterface(), this);
    }

    public Object getObject() throws Exception {
        return m_serviceProxy;
    }

    public Class getObjectType() {
        return m_serviceProxy != null ? m_serviceProxy.getClass() : getServiceInterface();
    }

    public boolean isSingleton() {
        return true;
    }
}
