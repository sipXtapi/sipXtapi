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

import java.util.HashMap;
import java.util.Map;

public class CdrSearch {
    public static final Map<String, String> ORDER_TO_COLUMN;

    static {
        Map<String, String> map = new HashMap<String, String>();
        map.put("caller", CdrManagerImpl.CALLER_AOR);
        map.put("callee", CdrManagerImpl.CALLEE_AOR);
        map.put("startTime", CdrManagerImpl.START_TIME);
        map.put("duration", CdrManagerImpl.END_TIME + " - " + CdrManagerImpl.CONNECT_TIME);
        map.put("termination", CdrManagerImpl.TERMINATION);
        ORDER_TO_COLUMN = map;
    }

    public enum Mode {
        NONE, CALLER, CALLEE, ANY
    }

    private Mode m_mode = Mode.NONE;
    private String m_term;
    private String m_order;
    private boolean m_ascending = true;

    public void setMode(Mode mode) {
        m_mode = mode;
    }

    public Mode getMode() {
        return m_mode;
    }

    public void setTerm(String term) {
        m_term = term;
    }

    public String getTerm() {
        return m_term;
    }

    public void setOrder(String order, boolean ascending) {
        m_order = order;
        m_ascending = ascending;
    }

    public String getColumnsStr() {
        switch (m_mode) {
        case CALLER:
            return CdrManagerImpl.CALLER_AOR;
        case CALLEE:
            return CdrManagerImpl.CALLEE_AOR;
        case ANY:
            return CdrManagerImpl.CALLER_AOR + " || " + CdrManagerImpl.CALLEE_AOR;
        default:
            return null;
        }
    }

    public boolean appendGetSql(StringBuilder sql) {
        String columnsStr = getColumnsStr();
        if (columnsStr == null) {
            return false;
        }

        sql.append(" AND (");
        sql.append(columnsStr);
        sql.append(" LIKE '%");
        sql.append(m_term);
        sql.append("%')");
        return true;
    }

    public boolean appendOrderBySql(StringBuilder sql) {
        String column = orderToColumn();
        if (column == null) {
            return false;
        }
        sql.append(" ORDER BY ");
        sql.append(column);
        sql.append(' ');
        sql.append(m_ascending ? "ASC" : "DESC");
        return true;
    }

    public boolean isSearch() {
        return m_mode != Mode.NONE;
    }

    private String orderToColumn() {
        if (m_order == null) {
            return CdrManagerImpl.START_TIME;
        }
        return ORDER_TO_COLUMN.get(m_order);
    }
}
