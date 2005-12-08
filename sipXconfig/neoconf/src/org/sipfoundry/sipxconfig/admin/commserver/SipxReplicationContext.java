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
import org.sipfoundry.sipxconfig.admin.dialplan.config.XmlFile;

public interface SipxReplicationContext {
    void generate(DataSet dataSet);

    void generateAll();
    
    void replicate(XmlFile xmlFile);
}
