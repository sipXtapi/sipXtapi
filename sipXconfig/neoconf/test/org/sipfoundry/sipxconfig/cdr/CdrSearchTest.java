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

import junit.framework.TestCase;

public class CdrSearchTest extends TestCase {

    public void testGetSqlEmpty() {
        CdrSearch search = new CdrSearch();
        assertEquals("", search.getSql());
    }

    public void testGetSqlCaller() {
        CdrSearch search = new CdrSearch();
        search.setTerm("abc");
        search.setMode(CdrSearch.Mode.CALLER);
        assertEquals(" AND (caller_aor LIKE '%abc%')", search.getSql());
    }

    public void testGetSqlCallee() {
        CdrSearch search = new CdrSearch();
        search.setTerm("abc");
        search.setMode(CdrSearch.Mode.CALLEE);
        assertEquals(" AND (callee_aor LIKE '%abc%')", search.getSql());
    }

    public void testGetSqlAny() {
        CdrSearch search = new CdrSearch();
        search.setTerm("abc");
        search.setMode(CdrSearch.Mode.ANY);
        assertEquals(" AND (callee_aor || caller_aor LIKE '%abc%')", search.getSql());
    }

}
