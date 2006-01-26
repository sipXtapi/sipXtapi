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

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;

/**
 * Interface to replication.cgi
 */
public interface ReplicationManager {
    /**
     * Replicates IMDB data sets to remore locations
     * 
     * @param locations list of locations that will receive replicated data
     * @param generator data set to be replicated
     * @return true if the replication has been successful, false otherwise
     */
    boolean replicateData(Location[] locations, DataSetGenerator generator);

    /**
     * Replicates file content to remore locations
     * 
     * @param locations list of locations that will receive replicated file
     * @param file object representing file content
     * 
     * @return true if the replication has been successful, false otherwise
     */
    boolean replicateFile(Location[] locations, XmlFile file);
}
