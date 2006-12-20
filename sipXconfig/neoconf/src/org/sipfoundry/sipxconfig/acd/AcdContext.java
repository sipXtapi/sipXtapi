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
package org.sipfoundry.sipxconfig.acd;

import java.io.Serializable;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.common.User;

public interface AcdContext extends AliasProvider {
    public static final String CONTEXT_BEAN_NAME = "acdContext";

    List getServers();
    
    List getUsersWithAgents();

    void store(AcdComponent acdComponent);

    AcdServer newServer();

    AcdLine newLine();

    AcdQueue newQueue();

    AcdAudio newAudio();

    void removeServers(Collection serversIds);

    void removeLines(Collection linesIds);

    void removeQueues(Collection queuesIds);

    void removeAgents(Serializable queueId, Collection agentsIds);

    void addUsersToQueue(Serializable queueId, Collection usersIds);

    AcdServer loadServer(Serializable serverId);

    AcdLine loadLine(Serializable id);

    AcdQueue loadQueue(Serializable id);

    AcdAgent loadAgent(Serializable id);

    void associate(Serializable lineId, Serializable queueId);

    void moveAgentsInQueue(Serializable queueId, Collection agentsIds, int step);

    void moveQueuesInAgent(Serializable agnetId, Collection queueIds, int step);

    /**
     * Removes all the servers, queues, lines and agents.
     */
    void clear();

    String getAudioServerUrl();

    /**
     * Migration task
     */
    void migrateOverflowQueues();

    void migrateLineExtensions();
    
    /**
     * @return true if ACD configuration disabled
     */
    boolean isEnabled();
    
    Collection<AcdQueue> getQueuesForUsers(AcdServer server, Collection<User> agents);
}
