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


package com.pingtel.pds.sds.cache;

import java.sql.SQLException;
import java.sql.Connection;


public interface DatabaseConnectionFactory {


    public void initialize ( String driverClassname, String jdbcURL, String userName, String password )
            throws SQLException;

    public Connection getConnection () throws SQLException;

    public String getDriverClassname ();

}
