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
package org.sipfoundry.sipxconfig.admin.commserver;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;

public class LazySipxReplicationContextImplTest extends TestCase {

    public void testGenerateAll() throws Exception {
        MockControl replicationCtrl = MockControl.createControl(SipxReplicationContext.class);
        SipxReplicationContext replication = (SipxReplicationContext) replicationCtrl.getMock();
        replication.generate(DataSet.ALIAS);
        replication.generate(DataSet.AUTH_EXCEPTION);
        replication.generate(DataSet.CREDENTIAL);
        replication.generate(DataSet.EXTENSION);
        replication.generate(DataSet.PERMISSION);
        replication.generate(DataSet.ALIAS);
        replication.generate(DataSet.AUTH_EXCEPTION);
        replication.generate(DataSet.CREDENTIAL);
        replication.generate(DataSet.EXTENSION);
        replication.generate(DataSet.PERMISSION);
        replicationCtrl.replay();
        
        LazySipxReplicationContextImpl lazy = new LazySipxReplicationContextImpl();
        
        int interval = 50;
        lazy.setSleepInterval(interval);
        lazy.setTarget(replication);
        
        lazy.init();
        
        for(int i = 0; i < 20; i++) {
            lazy.generate(DataSet.AUTH_EXCEPTION);
            lazy.generate(DataSet.PERMISSION);
            lazy.generateAll();            
        }
        
        Thread.sleep(200);
        
        for(int i = 0; i < 20; i++) {
            lazy.generate(DataSet.AUTH_EXCEPTION);
            lazy.generate(DataSet.PERMISSION);
            lazy.generateAll();            
        }        

        Thread.sleep(200);
        
        replicationCtrl.verify();
    }

}
