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

import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.springframework.jdbc.core.support.JdbcDaoSupport;

public class AcdHistoricalStatsImpl extends JdbcDaoSupport implements AcdHistoricalStats {
    
    public List<Map<String, Object>> getAgentSignInActivity(Date start, Date end) {
        List<Map<String, Object>> records = new ArrayList<Map<String, Object>>();
        Map<String, Object> record = new HashMap<String, Object>();
        record.put("agent_uri", "john@pingtel.com");
        record.put("signed_in", new Date());
        record.put("signed_out", new Date());
        records.add(record);
        return records;
    }
}
