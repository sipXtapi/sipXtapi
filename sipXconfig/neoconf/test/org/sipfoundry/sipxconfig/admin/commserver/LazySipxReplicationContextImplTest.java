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
import org.sipfoundry.sipxconfig.admin.dialplan.config.MappingRules;
import org.sipfoundry.sipxconfig.admin.dialplan.config.Orbits;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;

public class LazySipxReplicationContextImplTest extends TestCase {

    public void testGenerateAll() throws Exception {
        XmlFile mr = new MappingRules();
        XmlFile orbits = new Orbits();
        
        MockControl replicationCtrl = MockControl.createControl(SipxReplicationContext.class);
        SipxReplicationContext replication = (SipxReplicationContext) replicationCtrl.getMock();
        replication.replicate(mr);
        replication.generate(DataSet.ALIAS);
        replication.generate(DataSet.AUTH_EXCEPTION);
        replication.generate(DataSet.CREDENTIAL);
        replication.generate(DataSet.EXTENSION);
        replication.generate(DataSet.PERMISSION);
        replication.replicate(orbits);
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
        
        lazy.replicate(mr);
        for(int i = 0; i < 20; i++) {
            lazy.generate(DataSet.AUTH_EXCEPTION);
            lazy.generate(DataSet.PERMISSION);
            lazy.generateAll();            
        }
        
        Thread.sleep(400);
        
        lazy.replicate(orbits);
        for(int i = 0; i < 20; i++) {
            lazy.generate(DataSet.AUTH_EXCEPTION);
            lazy.generate(DataSet.PERMISSION);
            lazy.generateAll();            
        }        

        Thread.sleep(400);
        
        replicationCtrl.verify();
    }
}
