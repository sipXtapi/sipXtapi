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
package org.sipfoundry.sipxconfig.site;

import junit.framework.TestCase;

import org.apache.tapestry.engine.state.ApplicationStateManager;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class ApplicationLifecycleTest extends TestCase {
    
    public void testLogout() {
        IMocksControl stateManagerCtrl = EasyMock.createControl();
        ApplicationStateManager stateManager = stateManagerCtrl.createMock(ApplicationStateManager.class);
        stateManager.exists(UserSession.SESSION_NAME);
        stateManagerCtrl.andReturn(true);
        stateManager.get(UserSession.SESSION_NAME);
        stateManagerCtrl.andReturn(new UserSession());
        stateManagerCtrl.replay();
        
        ApplicationLifecycleImpl life = new ApplicationLifecycleImpl();
        life.setStateManager(stateManager);
        life.logout();
        assertTrue(life.getDiscardSession());
        
        stateManagerCtrl.verify();
    }
}
