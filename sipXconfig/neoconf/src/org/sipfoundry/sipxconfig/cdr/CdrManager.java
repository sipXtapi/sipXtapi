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

import java.util.Date;
import java.util.List;

public interface CdrManager {
    final String CONTEXT_BEAN_NAME = "cdrManager";

    /**
     * Retrieve CDRS between from and to dates
     * 
     * @param from date of first CDR retrieved, pass null for oldest
     * @param to date of the last CDR retrieved, pass null for latest
     * @return list of CDR objects
     */
    List<Cdr> getCdrs(Date from, Date to);
}
