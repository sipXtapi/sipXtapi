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

import org.apache.commons.lang.StringUtils;

public class CdrSearch {
    public enum Mode {
        NONE, CALLER, CALLEE, ANY
    }

    private Mode m_mode = Mode.NONE;
    private String m_term;

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

    public String getColumnsStr() {
        switch (m_mode) {
        case CALLER:
            return "caller_aor";
        case CALLEE:
            return "callee_aor";
        case ANY:
            return "callee_aor || caller_aor";
        default:
            return null;
        }
    }

    public String getSql() {
        String columnsStr = getColumnsStr();
        if (columnsStr == null) {
            return StringUtils.EMPTY;
        }

        return " AND (" + columnsStr + " LIKE '%" + m_term + "%')";
    }

    public boolean isSearch() {
        return m_mode != Mode.NONE;
    }
}
