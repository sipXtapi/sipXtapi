/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ListExtensionPoolsTag.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.jsptags;


import javax.naming.InitialContext;
import javax.servlet.jsp.JspException;
import javax.sql.DataSource;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

import org.jdom.Element;

import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.pgs.user.ExtensionPoolAdvocateHome;
import com.pingtel.pds.pgs.user.ExtensionPoolAdvocate;


/**
 * ListExtensionPoolsTag is a tag handler which can produces a listing
 * (in either XML or HTML) of the ExtensionPools in the config server.
 *
 * @author IB
 */
public class ListExtensionPoolsTag extends StyleTagSupport {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////

    private Integer mOrganizationID = null;
    private DataSource mDataSource = null;
    private ExtensionPoolAdvocate mExtensionAdvocateEJBObject = null;


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////

    public void setOrganizationid ( String organizationid ) {
        if ( organizationid != null )
            mOrganizationID = Integer.valueOf( organizationid );
    }


    public int doStartTag() throws JspException {

        Connection con = null;
        PreparedStatement ps = null;
        ResultSet rs = null;

        try {
            Element root = new Element ( "extensionpools");

            if ( mExtensionAdvocateEJBObject == null ) {
                ExtensionPoolAdvocateHome uaHome = (ExtensionPoolAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  ExtensionPoolAdvocateHome.class,
                                                                    "ExtensionPoolAdvocate" );

                mExtensionAdvocateEJBObject = uaHome.create();
            }

            if ( mDataSource == null ) {


                InitialContext initial = new InitialContext();

                mDataSource = (DataSource) initial.lookup( "java:/PDSDataSource" );
            }

            con = mDataSource.getConnection();

            ps = con.prepareStatement(  "SELECT ID, NAME  " +
                                        "FROM EXTENSION_POOLS " +
                                        "WHERE ORG_ID = ? " );

            ps.setInt( 1, mOrganizationID.intValue() );

            rs = ps.executeQuery();

            while ( rs.next() ) {

                Element extensionPool = new Element ( "extensionpool" );
                extensionPool.addContent(
                    new Element ("id").setText( String.valueOf( rs.getInt( 1 ) ) ) );
                extensionPool.addContent(
                    new Element ("name").setText( rs.getString( 2 ) ) );
                extensionPool.addContent(
                    new Element ("nextavailable").setText(
                            mExtensionAdvocateEJBObject.getNextExtension(
                                    String.valueOf( rs.getInt( 1 ) ) ) ));
                // and add it to the devices element
                root.addContent ( extensionPool );
            }

            outputTextToBrowser ( root );
        }
        catch( Exception ex ) {
             throw new JspException( ex.getMessage());
        }
        finally {
            if ( rs != null ) {
                try {
                    rs.close();
                }
                catch ( SQLException ex ) {
                }
            }
            if ( ps != null ) {
                try {
                    ps.close();
                }
                catch ( SQLException ex ) {
                }
            }
            if ( con != null ) {
                try {
                    con.close();
                }
                catch ( SQLException ex ) {
                }
            }
        }

        return SKIP_BODY;
    }


    protected void clearProperties() {
        mOrganizationID = null;
        super.clearProperties();
    }


    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////

    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////


}
