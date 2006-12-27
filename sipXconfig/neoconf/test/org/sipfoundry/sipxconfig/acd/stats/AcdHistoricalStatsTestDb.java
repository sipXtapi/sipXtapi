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
package org.sipfoundry.sipxconfig.acd.stats;

import java.util.Calendar;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class AcdHistoricalStatsTestDb extends TestCase {
    
    private AcdHistoricalStats m_history;

    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_history = (AcdHistoricalStats) app.getBean(AcdHistoricalStats.BEAN_NAME);
    }
    
    public void testSignoutActivityReport() {
        List<Map<String, Object>> stats = m_history.getReport("agentAvailablityReport", new Date(0), new Date());        
        assertEquals(10, stats.size());
        Map<String, Object> record;
        Iterator<Map<String, Object>> i = stats.iterator();
        record = i.next();
        assertEquals("sip:374@pingtel.com", record.get("agent_uri"));
    }

    public void testSignoutActivityReportColumns() {
        List<String> columns = m_history.getReportFields("agentAvailablityReport");        
        assertEquals(3, columns.size());
        assertEquals("agent_uri", columns.get(0));
        assertEquals("sign_in_time", columns.get(1));
        assertEquals("sign_out_time", columns.get(2));
    }
    
    public void testForReportSQLErrors() {
        List<String> reports = m_history.getReports();
        for (String report : reports) {
            m_history.getReportFields(report);
            m_history.getReport(report, new Date(0), new Date());
        }
    }
}
