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

import java.util.Date;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

public class AcdHistoricalStatsTest extends TestCase {
    
    private AcdHistoricalStats m_history;

    protected void setUp() {
        m_history = new AcdHistoricalStatsImpl();
    }
    
    public void testGetAgentSignInActivity() {
        List<Map<String,Object>> stats = m_history.getAgentSignInActivity(new Date(), new Date());
        assertEquals(1, stats.size());
    }
}
