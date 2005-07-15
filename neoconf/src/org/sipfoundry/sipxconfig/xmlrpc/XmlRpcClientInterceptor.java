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

import java.lang.reflect.Method;
import java.net.MalformedURLException;

import org.aopalliance.intercept.MethodInterceptor;
import org.aopalliance.intercept.MethodInvocation;
import org.apache.xmlrpc.XmlRpcClient;
import org.apache.xmlrpc.XmlRpcClientRequest;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.remoting.support.UrlBasedRemoteAccessor;

public class XmlRpcClientInterceptor extends UrlBasedRemoteAccessor implements MethodInterceptor,
        InitializingBean {
    private XmlRpcClient m_xmlRpcClient;

    public Object invoke(MethodInvocation invocation) throws Throwable {
        XmlRpcClientRequest request = new Request(invocation);
        return m_xmlRpcClient.execute(request);
    }

    public void afterPropertiesSet() throws MalformedURLException {
        if (getServiceInterface() == null) {
            throw new IllegalArgumentException("serviceInterface is required");
        }
        if (getServiceUrl() == null) {
            throw new IllegalArgumentException("serviceUrl is required");
        }
        m_xmlRpcClient = new XmlRpcClient(getServiceUrl());
    }

    private class Request implements XmlRpcClientRequest {
        private Method m_method;

        private Object[] m_args;

        public Request(MethodInvocation invocation) {
            m_method = invocation.getMethod();
            m_args = invocation.getArguments();
        }

        public String getMethodName() {
            return m_method.getName();
        }

        public int getParameterCount() {
            return m_args.length;
        }

        public Object getParameter(int index) {
            return m_args[index];
        }
    }
}
