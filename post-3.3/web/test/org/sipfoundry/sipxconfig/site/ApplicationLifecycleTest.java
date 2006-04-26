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
import org.easymock.MockControl;

public class ApplicationLifecycleTest extends TestCase {
    
    public void testLogout() {
        MockControl stateManagerCtrl = MockControl.createControl(ApplicationStateManager.class);
        ApplicationStateManager stateManager = (ApplicationStateManager) stateManagerCtrl.getMock();
        stateManagerCtrl.expectAndReturn(stateManager.exists(UserSession.SESSION_NAME), true);
        stateManagerCtrl.expectAndReturn(stateManager.get(UserSession.SESSION_NAME), new UserSession());
        stateManagerCtrl.replay();
        
        ApplicationLifecycleImpl life = new ApplicationLifecycleImpl();
        life.setStateManager(stateManager);
        life.logout();
        assertTrue(life.getDiscardSession());
        
        stateManagerCtrl.verify();
    }
}
