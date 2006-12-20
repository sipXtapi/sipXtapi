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

import java.io.Serializable;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.acd.AcdQueue;
import org.sipfoundry.sipxconfig.common.User;

public interface AcdStatistics {
    /**
     * @param queueUri null is all queues
     */
    List getAgentsStats(Serializable acdServerId, String queueUri);

    /**
     * @param queueUri null is all queues
     */
    List getCallsStats(Serializable acdServerId, String queueUri);

    List getQueuesStats(Serializable acdServerId);

    void setUsers(Collection<User> users);
    
    void setQueues(Collection<AcdQueue> queues);
}
