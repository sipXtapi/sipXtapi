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
package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.hibernate.event.SaveOrUpdateEvent;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.setting.Group;

public class PermissionReplicationTriggerTest extends TestCase {

    protected void setUp() throws Exception {
        super.setUp();
    }
    
    public void testOnSaveOrUpdate() {
        MockControl procControl = MockControl.createStrictControl(SipxProcessContext.class);
        SipxProcessContext proc = (SipxProcessContext) procControl.getMock();
        procControl.replay();
        
        CoreContextImpl.PermissionReplicationTrigger trigger = new CoreContextImpl.PermissionReplicationTrigger(proc);
        trigger.onSaveOrUpdate(new SaveOrUpdateEvent("bogus", new Object(), null));
        
        procControl.verify();
        
        procControl.reset();        
        proc.generate(DataSet.PERMISSION);
        procControl.setDefaultVoidCallable();
        procControl.replay();
        
        Group g = new Group();
        g.setResource("user");
        trigger.onSaveOrUpdate(new SaveOrUpdateEvent("real", g, null));

        procControl.verify();        
    }
}
