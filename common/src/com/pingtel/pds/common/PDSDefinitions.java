/*
 * $Id: //depot/OPENDEV/sipXconfig/common/src/com/pingtel/pds/common/PDSDefinitions.java#4 $
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

/**
 * PDSDefinitions is only to be used to store static finals which are shared
 * between 2 or more PDS components.   This allows for example the different
 * profile 'types' to be used by the ProfileWriter and the PGS in a common
 * way.
 *
 */
public interface PDSDefinitions {

    ///////////////////////////////////////////////////////////////////////
    //
    // Organizations come in three 'stereotypes', service provider, customer
    // (of a service provider) and enterprise (for PBX solutions).
    //
    // The following ORG_ definitions should be used to along with
    //
    ///////////////////////////////////////////////////////////////////////
    public static final int ORG_SERVICE_PROVIDER = 0;
    public static final int ORG_ENTERPRISE = 1;
    public static final int ORG_CUSTOMER = 2;

    // This sections definitions pertain to Profile definitions

    /** The following are values for Profile types */
    public static final int PROF_TYPE_PHONE = 1;
    public static final int PROF_TYPE_USER = 2;
    public static final int PROF_TYPE_APPLICATION_CONFIG = 3;
    public static final int PROF_TYPE_UPGRADESCRIPT = 4;
    public static final int PROF_TYPE_APPLICATION_REF = 5;

    /** The following are values for Phone models - matches the Model in DB */
/*
    public static final String DEVICE_TYPE_PINGTEL_INSTANT_XPRESSA = "InstantXpressa";
    public static final String DEVICE_TYPE_PINGTEL_XPRESSA = "Xpressa";
    public static final String DEVICE_TYPE_CISCO_7960 = "7960";
*/
    // To provide upgrade capability ensure that we have the old pre 2.1 subscribe
    // message Model=Value; values
    public static final String MODEL_LEGACY_XPRESSA         = "xpressa";
    public static final String MODEL_LEGACY_SOFTPHONE       = "instantXpressa";


    public static final String MODEL_HARDPHONE_CISCO_7960   = "7960";
    public static final String MODEL_HARDPHONE_CISCO_7940   = "7940";

    // These are the new values as per the new softphone
    public static final String MODEL_HARDPHONE_XPRESSA        = "xpressa_strongarm_vxworks";
    public static final String MODEL_SOFTPHONE_WIN            = "ixpressa_x86_win32";
    public static final String MODEL_SOFTPHONE_LINUX          = "ixpressa_x86_linux";
    public static final String MODEL_SOFTPHONE_SPARC_SOLARIS  = "ixpressa_sparc_solaris";

    // manufacturer types
    public static final String MANU_PINGTEL = "Pingtel";
    public static final String MANU_CISCO = "Cisco";

    // installation stereotypes
    public static final String ENTERPRISE_ST = "enterprise";

}