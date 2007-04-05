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

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.InitializationTask;
import org.springframework.context.ApplicationContext;

public class DataInitializerTest extends TestCase {
    
    DataInitializer initializer;
    
    protected void setUp() {
        initializer = new DataInitializer();
    }
    
    public void testApplyPatch() {        
        InitializationTask event0 = new InitializationTask("unittest");
        
        IMocksControl adminControl = EasyMock.createStrictControl();        
        AdminContext admin = adminControl.createMock(AdminContext.class);
        admin.deleteInitializationTask(event0.getTask());
        adminControl.replay();
                
        IMocksControl appControl = EasyMock.createStrictControl();        
        ApplicationContext context = appControl.createMock(ApplicationContext.class);        
        context.publishEvent(event0);
        appControl.replay();
        
        initializer.setAdminContext(admin);
        initializer.setApplicationContext(context);
        initializer.initializeData("unittest");
        
        adminControl.verify();
        appControl.verify();
    }
}
