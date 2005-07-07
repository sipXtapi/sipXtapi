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
    
    public static final String PROPERTY_RESOURCE_FILE = "sipxconfig.properties";
    
    static String SHORT_VERSION_INFO = "Version: {0}";
    
    static String LONG_VERSION_INFO = "Version: {0} ({1}) Build #: {2}";  

    private Properties m_props;
    
    public VersionInfo( InputStream versionStream ) throws IOException {
        m_props = new Properties();
        m_props.load( versionStream );

        // be backward compatible w/old names, going forward, do not add to
        // this list, use the new names
        m_props.setProperty("version", m_props.getProperty("sysdir.version"));
        m_props.setProperty("build_number", m_props.getProperty("sysdir.build_number"));
        m_props.setProperty("comment", m_props.getProperty("sysdir.comment"));
        m_props.setProperty("sipxconf", m_props.getProperty("sysdir.etc"));        
        m_props.setProperty("sipxtmp", m_props.getProperty("sysdir.tmp"));
        m_props.setProperty("sipxlog", m_props.getProperty("sysdir.log"));
        m_props.setProperty("sipxphone", m_props.getProperty("sysdir.phone"));
        m_props.setProperty("sipxdata", m_props.getProperty("sysdir.share"));
    }

    public String getVersion () {
        return getVersion(SHORT_VERSION_INFO);
    }

    String getVersion ( String format ) {
        String version = m_props.getProperty( "sysdir.version" );
        String comment = m_props.getProperty( "sysdir.comment" );
        String buildNumber = m_props.getProperty( "sysdir.build_number" );
        Object[] args = { version, comment, buildNumber };
        return MessageFormat.format( format, args );
    }
    
    
    public String getProperty(String prop) {
        return m_props.getProperty(prop);
    }
}
