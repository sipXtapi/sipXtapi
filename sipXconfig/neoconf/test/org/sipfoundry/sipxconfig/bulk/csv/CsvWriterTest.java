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
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.StringWriter;

import junit.framework.TestCase;

public class CsvWriterTest extends TestCase {

    protected void setUp() throws Exception {
        super.setUp();
    }

    public void testWriteNoQuote() throws Exception {
        StringWriter writer = new StringWriter();
        CsvWriter csvWriter = new CsvWriter(writer);
        String[] row = {
            "aa", null, "cc"
        };
        csvWriter.write(row, false);
        assertEquals("aa,,cc\n", writer.getBuffer().toString());
    }

    public void testWriteQuote() throws Exception {
        StringWriter writer = new StringWriter();
        CsvWriter csvWriter = new CsvWriter(writer);
        String[] row = {
            "aa", null, "cc"
        };
        csvWriter.write(row, true);
        assertEquals("\"aa\",\"\",\"cc\"\n", writer.getBuffer().toString());
    }
}
