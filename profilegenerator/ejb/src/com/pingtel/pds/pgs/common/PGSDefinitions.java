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



package com.pingtel.pds.pgs.common;

/**
 * PGSDefinitions should be used to store static finals which are used only
 * by PGS code.   If you need to share a value with another PDS component put
 * the defintions in PDSDefinitions.
 *
 * @author IB
 * @see com.pingtel.pds.common.PDSDefinitions
 */
public interface PGSDefinitions {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////

    /** These are the values for Properties finality */
    public static final Integer PR_FINAL = new Integer ( 1 );
    public static final Integer PR_NOT_FINAL = new Integer ( 0 );

    /** These are the values for Properties read-only attribute */
    public static final Integer PR_READ_ONLY = new Integer ( 1 );
    public static final Integer PR_NOT_READ_ONLY = new Integer ( 0 );


    /** These are the delete user group and phone group option values */
    public static final int DEL_DEEP_DELETE = 0;
    public static final int DEL_NO_GROUP = 1;
    public static final int DEL_REASSIGN = 2;

    ///////////////////////////////////////////////////////////////////////
    //
    // Definitions for the various cardinality values used by Ref Properties
    //
    ///////////////////////////////////////////////////////////////////////
    public static final String CARD_ONE_ONE = "1";
    public static final String CARD_ONE_MANY = "1..N";
    public static final String CARD_ZERO_ONE = "0..1";
    public static final String CARD_ZERO_MANY = "0..N";


    ///////////////////////////////////////////////////////////////////////
    //
    // These values are indexes used in boolean arrays to help the
    // EntityDetailsModifierBean know which XML databases to generate when
    // users, user groups or devices are generated.
    //
    ///////////////////////////////////////////////////////////////////////
    public static final int USER_CHANGE_ARRAY_SIZE = 4;
    public static final int EXTENSION_CHANGED = 0;
    public static final int ALIASES_CHANGED = 1;
    public static final int LINE_INFO_CHANGED = 2;
    public static final int HIERARCHY_CHANGED = 3;

    ///////////////////////////////////////////////////////////////////////
    //
    // Definitions for the various URL's used by the Config Server
    //
    ///////////////////////////////////////////////////////////////////////
    public static final String APPS_EXTERNAL_LOCATION =
        "http://www.pingtel.com/downloads/sipxchange/applicationslist.xml";
}