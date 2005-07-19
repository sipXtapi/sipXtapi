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


import java.util.Hashtable;
import java.util.Map;

import junit.framework.TestCase;

import org.springframework.aop.framework.ProxyFactory;

public class XmlRpcClientInterceptorTest extends TestCase {
    private Server m_server;

    protected void setUp() throws Exception {
        m_server = new Server();
    }

    protected void tearDown() throws Exception {
        m_server.stop();
    }

    public void testTest() {
        String result = m_server.multiplyTest("x", 5);
        assertEquals("xxxxx", result);
    }

    public void testIntercept() throws Exception {
        XmlRpcClientInterceptor interceptor = new XmlRpcClientInterceptor();
        interceptor.setServiceInterface(TestFunctions.class);
        interceptor.setServiceUrl("http://localhost:9997");
        interceptor.afterPropertiesSet();

        TestFunctions proxy = (TestFunctions) ProxyFactory.getProxy(TestFunctions.class,
                interceptor);
        
        String result = proxy.multiplyTest("x", 5);
        assertEquals("xxxxx", result);
        int len = proxy.calculateTest(new String[] {"a", "cc", "ddd"});
        assertEquals(6,len);
        
        Map xx = new Hashtable();
        xx.put("name", new Integer(5));
        proxy.create(xx);
    }
}
