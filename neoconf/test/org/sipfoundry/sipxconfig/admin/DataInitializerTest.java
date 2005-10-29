/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.InitializationTask;
import org.springframework.context.ApplicationContext;

public class DataInitializerTest extends TestCase {
    
    DataInitializer initializer;
    
    protected void setUp() {
        initializer = new DataInitializer();
    }
    
    public void testApplyPatch() {        
        InitializationTask event0 = new InitializationTask("unittest");
        
        MockControl adminControl = MockControl.createStrictControl(AdminManager.class);        
        AdminManager admin = (AdminManager) adminControl.getMock();
        admin.deleteInitializationTask(event0.getTask());
        adminControl.replay();
                
        MockControl appControl = MockControl.createStrictControl(ApplicationContext.class);        
        ApplicationContext context = (ApplicationContext) appControl.getMock();        
        context.publishEvent(event0);
        appControl.setDefaultVoidCallable();
        appControl.replay();
        
        initializer.initializeData("unittest", context, admin);
        
        adminControl.verify();
        appControl.verify();
    }
}
