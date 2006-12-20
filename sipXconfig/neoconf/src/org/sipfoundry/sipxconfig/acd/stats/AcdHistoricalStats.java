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

public interface AcdHistoricalStats {

    public List<Map<String, Object>> getAgentSignInActivity(Date start, Date end);
    
}
