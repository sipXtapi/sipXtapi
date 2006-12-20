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

import junit.framework.TestCase;

/**
 * Requires stats server to be running
 * 
 * cd sipXstats/lib ruby server.rb
 * 
 * Unclear how to integrate this
 */
public class AcdStatisticsImplTestStats extends TestCase {

    public void testCallStats() throws Exception {
        AcdStatsService service = new AcdStatisticsImpl().getAcdStatsService(null);
        CallStats[] callStats = service.getCallStats();
        assertNotNull(callStats);
    }

    public void testAgentStats() throws Exception {
        AcdStatsService service = new AcdStatisticsImpl().getAcdStatsService(null);
        AgentStats[] agentStats = service.getAgentStats();
        assertNotNull(agentStats);
    }

    public void testQueueStats() throws Exception {
        AcdStatsService service = new AcdStatisticsImpl().getAcdStatsService(null);
        QueueStats[] queueStats = service.getQueueStats();
        assertNotNull(queueStats);
    }

    public void testBirdArray() throws Exception {
        AcdStatsService service = new AcdStatisticsImpl().getAcdStatsService(null);
        Bird[] birds = service.getBirdArray();
        assertEquals("robin", birds[0].getSpecies());
        assertEquals("bluejay", birds[1].getSpecies());
    }
}
