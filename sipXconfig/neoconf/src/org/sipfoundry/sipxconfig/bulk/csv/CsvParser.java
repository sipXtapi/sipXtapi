/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.Reader;
import java.util.List;

import org.apache.commons.collections.Closure;

public interface CsvParser {
    /**
     * Parses CSV file and returns a collection of rows.
     * 
     * Each row is and array of Strings
     * 
     * @param csv reader for CSV data
     * @return list of rows, each row is and array of strings, each String represents a single
     *         field
     */
    List parse(Reader csv);

    /**
     * A preferred version of parsing for potentially larger files Does not build in-memory array.
     * 
     * @param csv reader for CSV data
     * @param closure execute function is called and passed array of Strings for each row
     */
    void parse(Reader csv, Closure closure);
}
