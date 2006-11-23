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

import java.io.IOException;
import java.io.Writer;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.cdr.Cdr.Termination;
import org.springframework.jdbc.core.PreparedStatementCreator;
import org.springframework.jdbc.core.ResultReader;
import org.springframework.jdbc.core.support.JdbcDaoSupport;

public class CdrManagerImpl extends JdbcDaoSupport implements CdrManager {
    private static final String CALLEE_AOR = "callee_aor";
    private static final String TERMINATION = "termination";
    private static final String FAILURE_STATUS = "failure_status";
    private static final String END_TIME = "end_time";
    private static final String CONNECT_TIME = "connect_time";
    private static final String START_TIME = "start_time";
    private static final String CALLER_AOR = "caller_aor";

    public List<Cdr> getCdrs(Date from, Date to) {
        return getCdrs(from, to, new CdrSearch());
    }

    public List<Cdr> getCdrs(Date from, Date to, CdrSearch search) {
        CdrsStatementCreator psc = new CdrsStatementCreator(from, to, search);
        CdrsResultReader resultReader = new CdrsResultReader();
        return getJdbcTemplate().query(psc, resultReader);
    }

    public void dumpCdrs(Writer writer, Date from, Date to, CdrSearch search) throws IOException {
        CdrsStatementCreator psc = new CdrsStatementCreator(from, to, search);
        CdrsCsvWriter resultReader = new CdrsCsvWriter(writer);
        try {
            getJdbcTemplate().query(psc, resultReader);
        } catch (RuntimeException e) {
            // unwrap IOException that might happen during reading DB
            if (e.getCause() instanceof IOException) {
                throw (IOException) e.getCause();
            }
            throw e;
        }
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
            cdr.setCalleeAor(rs.getString(CALLEE_AOR));
            cdr.setCallerAor(rs.getString(CALLER_AOR));
            cdr.setStartTime(rs.getTimestamp(START_TIME));
            cdr.setConnectTime(rs.getTimestamp(CONNECT_TIME));
            cdr.setEndTime(rs.getTimestamp(END_TIME));
            cdr.setFailureStatus(rs.getInt(FAILURE_STATUS));
            String termination = rs.getString(TERMINATION);
            cdr.setTermination(Termination.fromString(termination));
            m_cdrs.add(cdr);
        }
    }

    static class CdrsCsvWriter implements ResultReader {
        /** List of fields that will be exported to CDR */
        private static final String[] FIELDS = {
            CALLEE_AOR, CALLER_AOR, START_TIME, CONNECT_TIME, END_TIME, FAILURE_STATUS,
            TERMINATION,
        };

        private CsvWriter m_csv;

        public CdrsCsvWriter(Writer writer) throws IOException {
            m_csv = new CsvWriter(writer);
            m_csv.write(FIELDS, false);
        }

        public List getResults() {
            return null;
        }

        public void processRow(ResultSet rs) throws SQLException {
            String[] row = new String[FIELDS.length];
            for (int i = 0; i < row.length; i++) {
                String value = rs.getString(FIELDS[i]);
                row[i] = value;
            }
            try {
                m_csv.write(row, true);
            } catch (IOException e) {
                new RuntimeException(e);
            }
        }
    }
}
