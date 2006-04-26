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

import junit.framework.TestCase;

import org.apache.xmlrpc.XmlRpcClient;
import org.apache.xmlrpc.XmlRpcException;
import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;
import org.springframework.aop.framework.ProxyFactory;

public class XmlRpcClientInterceptorMockTest extends TestCase {

    public void testIntercept() throws Exception {
        MockControl mcClient = MockClassControl.createControl(XmlRpcClient.class, new Class[] {
            String.class
        }, new Object[] {
            "http://localhost:9997"
        });
        XmlRpcClient client = (XmlRpcClient) mcClient.getMock();        
        client.execute(null);
        mcClient.setMatcher(MockControl.ALWAYS_MATCHER);
        mcClient.setReturnValue("xxxxx");
        mcClient.replay();

        XmlRpcClientInterceptor interceptor = new XmlRpcClientInterceptor();
        interceptor.setServiceInterface(TestFunctions.class);
        interceptor.setXmlRpcClient(client);

        TestFunctions proxy = (TestFunctions) ProxyFactory.getProxy(TestFunctions.class,
                interceptor);

        String result = proxy.multiplyTest("x", 5);
        assertEquals("xxxxx", result);

        mcClient.verify();
    }
    
    public void testInterceptException() throws Exception {
        MockControl mcClient = MockClassControl.createControl(XmlRpcClient.class, new Class[] {
            String.class
        }, new Object[] {
            "http://localhost:9997"
        });
        XmlRpcClient client = (XmlRpcClient) mcClient.getMock();        
        client.execute(null);
        mcClient.setMatcher(MockControl.ALWAYS_MATCHER);
        mcClient.setThrowable(new XmlRpcException(2, "message"));
        mcClient.replay();

        XmlRpcClientInterceptor interceptor = new XmlRpcClientInterceptor();
        interceptor.setServiceInterface(TestFunctions.class);
        interceptor.setXmlRpcClient(client);

        TestFunctions proxy = (TestFunctions) ProxyFactory.getProxy(TestFunctions.class,
                interceptor);

        try {
            proxy.multiplyTest("x", 5);
            fail("Should throw exception");
        }
        catch (XmlRpcRemoteException e) {
            assertEquals(2, e.getFaultCode());
            assertEquals("message", e.getMessage());
        }

        mcClient.verify();
    }

    public void testInterceptFault() throws Exception {
        MockControl mcClient = MockClassControl.createControl(XmlRpcClient.class, new Class[] {
            String.class
        }, new Object[] {
            "http://localhost:9997"
        });
        XmlRpcClient client = (XmlRpcClient) mcClient.getMock();        
        client.execute(null);
        mcClient.setMatcher(MockControl.ALWAYS_MATCHER);
        // sometimes client will return exception instead of throwing it
        mcClient.setReturnValue(new XmlRpcException(2, "message"));
        mcClient.replay();

        XmlRpcClientInterceptor interceptor = new XmlRpcClientInterceptor();
        interceptor.setServiceInterface(TestFunctions.class);
        interceptor.setXmlRpcClient(client);

        TestFunctions proxy = (TestFunctions) ProxyFactory.getProxy(TestFunctions.class,
                interceptor);

        try {
            proxy.multiplyTest("x", 5);
            fail("Should throw exception");
        }
        catch (XmlRpcRemoteException e) {
            assertEquals(2, e.getFaultCode());
            assertEquals("message", e.getMessage());
        }

        mcClient.verify();
    }    
}
