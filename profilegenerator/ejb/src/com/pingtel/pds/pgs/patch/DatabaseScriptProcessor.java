/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/patch/DatabaseScriptProcessor.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.patch;

import javax.naming.*;
import javax.ejb.*;
import javax.sql.DataSource;

import java.sql.*;

import java.io.*;

import java.util.*;

import org.apache.log4j.Category;

import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.common.PDSException;

public class DatabaseScriptProcessor implements Patch {

    private String m_scriptName;
    private Category m_logger = Category.getInstance( "pgs" );

    public void initialize(HashMap props) {
        m_scriptName = (String) props.get( "script" );
    }

    public String update() throws PDSException {

        Connection con = null;
        Statement stmt = null;

        try {

            InitialContext ctx = new InitialContext();
            // @JC Need to remove the hard coding from here
            DataSource ds = (DataSource)ctx.lookup ( "java:/PDSDataSource" );
            con = ds.getConnection();

            InputStream is = getClass().getResourceAsStream( m_scriptName );

            byte [] buffer = new byte [1024];
            StringBuffer scriptContent = new StringBuffer();

            while ( true ) {
                int len = is.read( buffer );
                if ( len == -1 )
                    break;

                scriptContent.append( new String ( buffer, 0, len ) );
            }

            StringTokenizer st = new StringTokenizer ( scriptContent.toString().trim(), ";" );
            while ( st.hasMoreTokens() ) {
                String statementText = st.nextToken();

                m_logger.debug ( "statment text <" + statementText + ">" );
                stmt = con.createStatement();
                stmt.execute( statementText );
            }
        }
        catch ( Exception ex ) {
            m_logger.error ( "error in DatabaseScriptProcessor.update(): " + ex.toString() );
            throw new PDSException ( "error in DatabaseScriptProcessor.update(): " ,ex );
        }
        finally {
            try {
                if ( stmt != null )
                    stmt.close();
            }
            catch ( SQLException ex ) {}

            try {
                if ( con != null )
                    con.close();
            }
            catch ( SQLException ex ) {}

        }

        return "S";
    }
}