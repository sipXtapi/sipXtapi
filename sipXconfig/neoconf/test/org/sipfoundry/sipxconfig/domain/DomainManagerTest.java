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
package org.sipfoundry.sipxconfig.domain;

import java.util.Collections;

import junit.framework.TestCase;

import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.admin.commserver.SipxServer;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class DomainManagerTest extends TestCase {
    
    public void testSaveDomain() {
        Domain domain = new Domain("goose");

        IMocksControl serverControl = org.easymock.classextension.EasyMock.createControl();
        SipxServer server = serverControl.createMock(SipxServer.class);
        server.setDomainName("goose");
        server.setRegistrarDomainAliases(null);
        server.applySettings();
        serverControl.replay();
        
        IMocksControl dbControl = org.easymock.classextension.EasyMock.createControl();
        HibernateTemplate db = dbControl.createMock(HibernateTemplate.class);
        db.findByNamedQuery("domain");
        dbControl.andReturn(Collections.EMPTY_LIST);
        db.saveOrUpdate(domain);        
        dbControl.replay();
        
        // TEST DELETE EXISTING
        DomainManagerImpl mgr = new DomainManagerImpl();
        mgr.setHibernateTemplate(db);
        mgr.setServer(server);
        mgr.saveDomain(domain);

        dbControl.verify();
        serverControl.verify();

        // TEST IGNORE EXISTING (assumes there is no other, doesn't care actually)
        domain.setUniqueId(); // isNew!
        serverControl.reset();
        server.setDomainName("goose");
        server.setRegistrarDomainAliases(null);
        server.applySettings();        
        serverControl.replay();
        
        dbControl.reset();
        db.saveOrUpdate(domain);        
        dbControl.replay();
        
        mgr.saveDomain(domain);

        dbControl.verify();
        serverControl.verify();
    }
}