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
package org.sipfoundry.sipxconfig.presence;

import java.util.Hashtable;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;

public class PresenceServerTest extends TestCase {
    
    private static final Hashtable SUCCESS_RESPONSE = new Hashtable();
    static {
        SUCCESS_RESPONSE.put("result-code", new Integer(1));
    }
    
    public void testSignInAction() {
        MockControl coreContextCtrl = MockControl.createStrictControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) coreContextCtrl.getMock();
        coreContextCtrl.expectAndReturn(coreContext.getDomainName(), "example.com");
        coreContextCtrl.replay();
        
        Hashtable signin = new Hashtable();
        signin.put("object-class", "login");
        signin.put("user-action", "sip:joe@example.com");
        
        MockControl apiCtrl = MockControl.createControl(PresenceServerImpl.SignIn.class);
        PresenceServerImpl.SignIn api = (PresenceServerImpl.SignIn) apiCtrl.getMock();
        apiCtrl.expectAndReturn(api.action(signin), SUCCESS_RESPONSE);        
        apiCtrl.replay();

        User joe = new User();
        joe.setUserName("joe");
        
        PresenceServerImpl impl = new PresenceServerImpl();
        impl.setCoreContext(coreContext);
        impl.userAction(api, "user-action", joe);

        apiCtrl.verify();
        coreContextCtrl.verify();
    }    
    
    public void testCheckError() {
        Hashtable response = new Hashtable();
        response.put("result-code", new Integer(0));
        response.put("result-text", "testing error handling");
        try {
            PresenceServerImpl.checkErrorCode(response);
            fail("Expected exception");
        } catch (UserException e) {
            assertTrue(true);
        }
        
        PresenceServerImpl.checkErrorCode(SUCCESS_RESPONSE);        
    }
}
