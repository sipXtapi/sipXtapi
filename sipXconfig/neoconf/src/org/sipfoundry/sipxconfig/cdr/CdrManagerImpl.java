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
package org.sipfoundry.sipxconfig.cdr;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.sipfoundry.sipxconfig.cdr.Cdr.Termination;
import org.springframework.jdbc.core.PreparedStatementCreator;
import org.springframework.jdbc.core.ResultReader;
import org.springframework.jdbc.core.support.JdbcDaoSupport;

public class CdrManagerImpl extends JdbcDaoSupport implements CdrManager {

    public List<Cdr> getCdrs(Date from, Date to) {
        return getCdrs(from, to, new CdrSearch());
    }

    public List<Cdr> getCdrs(Date from, Date to, CdrSearch search) {
        CdrsStatementCreator psc = new CdrsStatementCreator(from, to, search);
        CdrsResultReader resultReader = new CdrsResultReader();
        return getJdbcTemplate().query(psc, resultReader);
    }

    static class CdrsStatementCreator implements PreparedStatementCreator {
        // FIXME: support LIMIT and offset " LIMIT 25 OFFSET 0";
        private static final String SELECT = "SELECT * FROM cdrs WHERE (? <= start_time) AND (start_time <= ?)";
        private static final String ORDER_BY = " ORDER BY start_time";

        private Timestamp m_from;
        private Timestamp m_to;
        private CdrSearch m_search;

        public CdrsStatementCreator(Date from, Date to, CdrSearch search) {
            long fromMillis = from != null ? from.getTime() : 0;
            m_from = new Timestamp(fromMillis);
            long toMillis = to != null ? to.getTime() : System.currentTimeMillis();
            m_to = new Timestamp(toMillis);
            m_search = search;
        }

        public PreparedStatement createPreparedStatement(Connection con) throws SQLException {
            String searchSql = m_search.getSql();
            PreparedStatement ps = con.prepareStatement(SELECT + searchSql + ORDER_BY);
            ps.setTimestamp(1, m_from);
            ps.setTimestamp(2, m_to);
            return ps;
        }
    }

    static class CdrsResultReader implements ResultReader {
        private List<Cdr> m_cdrs = new ArrayList<Cdr>();

        public List<Cdr> getResults() {
            return m_cdrs;
        }

        public void processRow(ResultSet rs) throws SQLException {
            Cdr cdr = new Cdr();
            cdr.setCalleeAor(rs.getString("callee_aor"));
            cdr.setCallerAor(rs.getString("caller_aor"));
            cdr.setStartTime(rs.getTimestamp("start_time"));
            cdr.setConnectTime(rs.getTimestamp("connect_time"));
            cdr.setEndTime(rs.getTimestamp("end_time"));
            cdr.setFailureStatus(rs.getInt("failure_status"));
            String termination = rs.getString("termination");
            cdr.setTermination(Termination.fromString(termination));
            m_cdrs.add(cdr);
        }
    }
}
