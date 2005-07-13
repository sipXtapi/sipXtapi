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

import java.io.InputStream;
import java.util.Iterator;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSetGenerator;
import org.springframework.context.ApplicationContext;

public class SipxProcessContextImplTest extends TestCase {
    SipxProcessContextImpl m_processContextImpl;

    protected void setUp() throws Exception {
        m_processContextImpl = new SipxProcessContextImpl() {
            protected InputStream getTopologyAsStream() {
                return SipxProcessContextImplTest.class.getResourceAsStream("topology.test.xml");
            }
        };
    }

    public void testConstructRestartUrl() {
        String[] url = m_processContextImpl
                .constructRestartUrl(SipxProcessContext.Process.REGISTRAR);

        assertEquals(
                "https://localhost:8091/cgi-bin/processmonitor/process.cgi?command=restart&process=SIPRegistrar",
                url[0]);
    }

    public void testRetrieveProcesUrls() throws Exception {
        String[] urls = m_processContextImpl.getProcessUrls();
        assertEquals(2, urls.length);
        assertEquals("https://localhost:8091/cgi-bin/processmonitor/process.cgi", urls[0]);
        assertEquals("https://192.168.0.27:8091/cgi-bin/processmonitor/process.cgi", urls[1]);
    }

    public void testRetrieveREplicationUrls() throws Exception {
        String[] urls = m_processContextImpl.getReplicationUrls();
        assertEquals(2, urls.length);
        assertEquals("https://localhost:8091/cgi-bin/replication/replication.cgi", urls[0]);
        assertEquals("https://192.168.0.27:8091/cgi-bin/replication/replication.cgi", urls[1]);
    }

    public void testDataSetGeneratorBeans() {
        ApplicationContext applicationContext = TestHelper.getApplicationContext();
        for (Iterator i = DataSet.iterator(); i.hasNext();) {
            DataSet set = (DataSet) i.next();
            assertNotNull(applicationContext.getBean(set.getBeanName(), DataSetGenerator.class));
        }
    }
}
