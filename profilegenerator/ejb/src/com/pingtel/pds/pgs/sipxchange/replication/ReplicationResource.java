/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/sipxchange/replication/ReplicationResource.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.sipxchange.replication;


/**
 * ReplicationResource
 *
 * @author Pradeep Paudyal
 */
public interface ReplicationResource
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public static final String TYPE_FILE              = "file" ;
    public static final String TYPE_DATABASE          = "database" ;

    public static final String DATABASE_CREDENTIAL      = "credential";
    public static final String DATABASE_ALIAS           = "alias";
    public static final String DATABASE_REGISTRATION    = "registration";
    public static final String DATABASE_PERMISSION      = "permission";
    public static final String DATABASE_EXTENSION       = "extension";
    public static final String DATABASE_AUTH_EXCEPTIONS = "authexception";


    public static final String FILE_RESOURCEMAP_XML   = "resourcemap_xml";
    public static final String FILE_RESOURCEMAP_DTD   = "resourcemap_dtd";



//////////////////////////////////////////////////////////////////////////////
// Attributes
////

//////////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////


//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////
}

