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

import java.util.Properties;

public class VersionInfo {

    private String m_version;
    
    private Properties m_props;

    public VersionInfo( InputStream versionStream ) throws IOException {

        m_props = new Properties();
        m_props.load( versionStream );

        String version = m_props.getProperty( "version" );
        String buildNumber = m_props.getProperty( "build_number" );
        String comment = m_props.getProperty( "comment" );

        StringBuffer versionString = new StringBuffer();

        versionString.append( "Version: " + version );
        if ( comment != null && comment.trim().length() > 0 )
            versionString.append( " (" + comment + ")" );

        versionString.append( " Build #: " + buildNumber );

        m_version = versionString.toString();
    }

    public String getVersion () {
        return m_version;
    }

    public String getProperty(String prop) {
        return m_props.getProperty(prop);
    }
}
