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

import java.io.IOException;
import java.io.Writer;

import org.apache.commons.lang.StringUtils;

public class CsvWriter {
    private Writer m_writer;

    public CsvWriter(Writer writer) {
        super();
        m_writer = writer;
    }

    public void write(String[] fields, boolean quote) throws IOException {
        StringBuilder line = new StringBuilder();
        for (int i = 0; i < fields.length; i++) {
            String field = fields[i];

            if (quote) {
                line.append(CsvParserImpl.FIELD_QUOTE);
            }
            line.append(StringUtils.defaultString(field));
            if (quote) {
                line.append(CsvParserImpl.FIELD_QUOTE);
            }
            if (i < fields.length - 1) {
                line.append(CsvParserImpl.FIELD_SEPARATOR);
            } else {
                line.append('\n');
            }
        }
        m_writer.write(line.toString());
    }
}
