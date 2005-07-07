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

import java.io.InputStream;
import java.io.IOException;

import java.text.MessageFormat;
import java.util.Properties;

public class VersionInfo {
    private Properties m_props;
    static String SHORT_VERSION_INFO = "Version: {0}";  
    static String LONG_VERSION_INFO = "Version: {0} ({1}) Build #: {2}";  

    public VersionInfo( InputStream versionStream ) throws IOException {
        m_props = new Properties();
        m_props.load( versionStream );
    }

    public String getVersion () {
        return getVersion(SHORT_VERSION_INFO);
    }

    String getVersion ( String format ) {
        String version = m_props.getProperty( "version" );
        String comment = m_props.getProperty( "comment" );
        String buildNumber = m_props.getProperty( "build_number" );
        Object[] args = { version, comment, buildNumber };
        return MessageFormat.format( format, args );
    }
    
    
    public String getProperty(String prop) {
        return m_props.getProperty(prop);
    }
}
