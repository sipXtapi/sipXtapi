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

import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;

public interface SipxReplicationManager {
    void generate(DataSet dataSet);

    void generateAll();
}
