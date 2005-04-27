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

package com.pingtel.pds.pgs.patch;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Map;
import java.util.StringTokenizer;

import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.sql.DataSource;

import org.apache.log4j.Category;

public class DatabaseScriptProcessor implements Patch {

    private String m_scriptName;
    private Category m_logger = Category.getInstance("pgs");

    public void initialize(Map props) {
        m_scriptName = (String) props.get("script");
    }

    public String update(Integer patchNumber) {

        Connection con = null;
        Statement stmt = null;
        String status = FAILURE;

        try {
            InitialContext ctx = new InitialContext();
            // @JC Need to remove the hard coding from here
            DataSource ds = (DataSource) ctx.lookup("java:/PDSDataSource");
            con = ds.getConnection();

            BufferedReader is = new BufferedReader(new InputStreamReader(getClass().getResourceAsStream(m_scriptName)));            
            StringBuffer scriptContent = new StringBuffer();
            while (true) {
                String line = is.readLine();
                if( line == null )
                {
                    break;
                }
                scriptContent.append(line);
            }

            StringTokenizer st = new StringTokenizer(scriptContent.toString().trim(), ";");
            while (st.hasMoreTokens()) {
                String statementText = st.nextToken();

                m_logger.debug("statment text <" + statementText + ">");
                stmt = con.createStatement();
                stmt.execute(statementText);
            }
            status = SUCCESS;
        } catch (SQLException e) {
            m_logger.error("error in DatabaseScriptProcessor.update(): " + e.toString());
        } catch (IOException e) {
            m_logger.error("error in DatabaseScriptProcessor.update(): " + e.toString());
        } catch (NamingException e) {
            throw new RuntimeException(e);
        } finally {
            try {
                if (stmt != null)
                    stmt.close();
            } catch (SQLException ex) {
                // ignore
            }

            try {
                if (con != null)
                    con.close();
            } catch (SQLException ex) {
                // ignore
            }
        }
        return status;
    }
}