/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.common;

import org.apache.log4j.Priority;

/**
 * TXPriority is a log4j priority class.   It is basically used to differentiate
 * log messages which should be used for auditing purposes as apposed to ERROR,
 * DEBUG, INFO, etc.
 *
 * @see
 * @since
 */
public class TXPriority extends Priority {

    public final static int TX_INT  = 35000;

    public final static int TX_SYS = 3;

    public final static TXPriority TX_MESSAGE = new TXPriority(TX_INT, "TRANSACTION", TX_SYS);

    public TXPriority (int level, String levelStr, int syslogEquivalent) {
        super( level, levelStr, syslogEquivalent );
    }
}
