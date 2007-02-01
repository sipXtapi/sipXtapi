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
import java.net.MalformedURLException;
import java.net.URL;
import java.rmi.RemoteException;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import javax.xml.rpc.ServiceException;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.cdr.Cdr.Termination;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.jdbc.core.PreparedStatementCreator;
import org.springframework.jdbc.core.ResultReader;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.jdbc.core.SingleColumnRowMapper;
import org.springframework.jdbc.core.support.JdbcDaoSupport;

public class CdrManagerImpl extends JdbcDaoSupport implements CdrManager {
    static final String CALLEE_AOR = "callee_aor";
    static final String TERMINATION = "termination";
    static final String FAILURE_STATUS = "failure_status";
    static final String END_TIME = "end_time";
    static final String CONNECT_TIME = "connect_time";
    static final String START_TIME = "start_time";
    static final String CALLER_AOR = "caller_aor";

    private String m_cdrAgentHost;
    private int m_cdrAgentPort;

    public List<Cdr> getCdrs(Date from, Date to) {
        return getCdrs(from, to, new CdrSearch());
    }

    public List<Cdr> getCdrs(Date from, Date to, CdrSearch search) {
        return getCdrs(from, to, search, 0, 0);
    }

    public List<Cdr> getCdrs(Date from, Date to, CdrSearch search, int limit, int offset) {
        CdrsStatementCreator psc = new SelectAll(from, to, search, limit, offset);
        CdrsResultReader resultReader = new CdrsResultReader();
        return getJdbcTemplate().query(psc, resultReader);
    }

    public void dumpCdrs(Writer writer, Date from, Date to, CdrSearch search) throws IOException {
        CdrsStatementCreator psc = new SelectAll(from, to, search);
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

    public int getCdrCount(Date from, Date to, CdrSearch search) {
        CdrsStatementCreator psc = new SelectCount(from, to, search);
        RowMapper rowMapper = new SingleColumnRowMapper(Integer.class);
        List results = getJdbcTemplate().query(psc, rowMapper);
        return (Integer) DataAccessUtils.requiredUniqueResult(results);
    }

    public List<Cdr> getActiveCalls() {
        try {
            CdrService cdrService = getCdrService();
            ActiveCall[] activeCalls = cdrService.getActiveCalls();
            List<Cdr> cdrs = new ArrayList<Cdr>(activeCalls.length);
            for (ActiveCall call : activeCalls) {
                Cdr cdr = new Cdr();
                cdr.setCallerAor(call.getFrom());
                cdr.setCalleeAor(call.getTo());
                cdr.setStartTime(call.getStart_time().getTime());
                cdrs.add(cdr);
            }
            return cdrs;
        } catch (RemoteException e) {
            throw new RuntimeException(e);
        }
    }

    public CdrService getCdrService() {
        try {
            URL url = new URL("http", m_cdrAgentHost, m_cdrAgentPort, StringUtils.EMPTY);
            return new CdrImplServiceLocator().getCdrService(url);
        } catch (ServiceException e) {
            throw new RuntimeException(e);
        } catch (MalformedURLException e) {
            throw new RuntimeException(e);
        }
    }

    public void setCdrAgentHost(String cdrAgentHost) {
        m_cdrAgentHost = cdrAgentHost;
    }

    public void setCdrAgentPort(int cdrAgentPort) {
        m_cdrAgentPort = cdrAgentPort;
    }

    abstract static class CdrsStatementCreator implements PreparedStatementCreator {
        private static final String FROM = " FROM cdrs WHERE (? <= start_time) AND (start_time <= ?)";
        private static final String LIMIT = " LIMIT ? OFFSET ?";

        private Timestamp m_from;
        private Timestamp m_to;
        private CdrSearch m_search;
        private int m_limit;
        private int m_offset;

        public CdrsStatementCreator(Date from, Date to, CdrSearch search) {
            this(from, to, search, 0, 0);
        }

        public CdrsStatementCreator(Date from, Date to, CdrSearch search, int limit, int offset) {
            long fromMillis = from != null ? from.getTime() : 0;
            m_from = new Timestamp(fromMillis);
            long toMillis = to != null ? to.getTime() : System.currentTimeMillis();
            m_to = new Timestamp(toMillis);
            m_search = search;
            m_limit = limit;
            m_offset = offset;
        }

        public PreparedStatement createPreparedStatement(Connection con) throws SQLException {
            StringBuilder sql = new StringBuilder(getSelectSql());
            sql.append(FROM);
            m_search.appendGetSql(sql);
            appendOrderBySql(sql);
            if (m_limit > 0) {
                sql.append(LIMIT);
            }
            PreparedStatement ps = con.prepareStatement(sql.toString());
            ps.setTimestamp(1, m_from);
            ps.setTimestamp(2, m_to);
            if (m_limit > 0) {
                ps.setInt(3, m_limit);
                ps.setInt(4, m_offset);
            }
            return ps;
        }

        public abstract String getSelectSql();

        protected void appendOrderBySql(StringBuilder sql) {
            m_search.appendOrderBySql(sql);
        }
    }

    static class SelectAll extends CdrsStatementCreator {
        public SelectAll(Date from, Date to, CdrSearch search, int limit, int offset) {
            super(from, to, search, limit, offset);
        }

        public SelectAll(Date from, Date to, CdrSearch search) {
            super(from, to, search);
        }

        @Override
        public String getSelectSql() {
            return "SELECT *";
        }
    }

    static class SelectCount extends CdrsStatementCreator {

        public SelectCount(Date from, Date to, CdrSearch search) {
            super(from, to, search);
        }

        @Override
        public String getSelectSql() {
            return "SELECT COUNT(id)";
        }

        @Override
        protected void appendOrderBySql(StringBuilder sql) {
            // no ordering when selecting COUNT
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
