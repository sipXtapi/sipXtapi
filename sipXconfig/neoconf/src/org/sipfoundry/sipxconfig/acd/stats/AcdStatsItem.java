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

/**
 * Allow polymorphic access to data across independant statistical subclasses. 
 */
public class AcdStatsItem {
    
    public static interface AgentName {
        public String getAgentName();        
    }
    
    public static interface QueueName {
        public String getQueueName();        
    }
}
