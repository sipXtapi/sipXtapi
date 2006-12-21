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

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;

import org.springframework.jdbc.core.PreparedStatementCreator;

public class AcdHistoricalReport implements PreparedStatementCreator {   
    private String m_query;
    private Integer m_limit;

    public String getQuery() {
        return m_query;
    }

    public void setQuery(String query) {
        m_query = query;
    }
    
    public void setLimit(Integer limit) {
        m_limit = limit;
    }

    public PreparedStatement createPreparedStatement(Connection con) throws SQLException {
        String query = getQuery();
        if (m_limit != null) {
            query += " limit " + m_limit;
        }
        PreparedStatement ps = con.prepareStatement(getQuery());
        return ps;
    }
}
