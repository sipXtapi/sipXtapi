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
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.Iterator;

import junit.framework.TestCase;

import org.dom4j.Element;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.springframework.context.ApplicationContext;

public class DataSetGeneratorTest extends TestCase {
    public void testDataSetGeneratorBeans() {
        ApplicationContext ac = TestHelper.getApplicationContext();
        for (Iterator i = DataSet.iterator(); i.hasNext();) {
            DataSet set = (DataSet) i.next();
            DataSetGenerator gen = (DataSetGenerator) ac.getBean(set.getBeanName(),
                    DataSetGenerator.class);
            assertNotNull(set.getName(), gen);
            assertNotNull(set.getName(), gen.getCoreContext());
        }
    }

    public void testGetDomain() {
        DataSetGenerator dsg = new DataSetGenerator() {
            protected DataSet getType() {
                return null;
            }

            protected void addItems(Element items) {
                // do nothing
            }
        };

        IMocksControl coreContextCtrl = EasyMock.createControl();
        CoreContext coreContext = coreContextCtrl.createMock(CoreContext.class);
        coreContext.getDomainName();
        coreContextCtrl.andReturn("core.domain.com");
        coreContextCtrl.replay();

        dsg.setCoreContext(coreContext);
        assertEquals("core.domain.com", dsg.getSipDomain());

        coreContextCtrl.verify();
        
        dsg.setSipDomain("set.domain.com");
        assertEquals("set.domain.com", dsg.getSipDomain());
    }
}
