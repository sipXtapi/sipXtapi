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


package com.pingtel.pds.pgs.jsptags;


import java.io.IOException;
import java.rmi.RemoteException;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;

import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.servlet.jsp.JspException;
import javax.sql.DataSource;
import javax.xml.transform.TransformerException;

import org.jdom.Element;

import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;



public class ExtensionPoolDetailsTag extends StyleTagSupport {

    private Integer m_extensionPoolID = null;
    private DataSource m_dataSource = null;

    public void setExtensionpoolid ( String extensionpoolid ) {
        if ( extensionpoolid != null )
            m_extensionPoolID = Integer.valueOf( extensionpoolid );
    }


    public int doStartTag() throws JspException {

        Connection con = null;
        PreparedStatement ps = null;
        ResultSet rs = null;

        try {
            Element root = new Element ( "extensionpool");
            if ( m_dataSource == null ) {


                InitialContext initial = new InitialContext();

                m_dataSource = (DataSource) initial.lookup( "java:/PDSDataSource" );
            }

            con = m_dataSource.getConnection();

            ps = con.prepareStatement(  "SELECT NAME  " +
                                        "FROM EXTENSION_POOLS " +
                                        "WHERE ID = ? " );

            ps.setInt( 1, m_extensionPoolID.intValue() );

            rs = ps.executeQuery();

            while ( rs.next() ) {

                Element attributes = new Element ( "attributes" );
                attributes.addContent(
                    new Element ("id").setText( m_extensionPoolID.toString() ) );

                attributes.addContent(
                    new Element ("name").setText( rs.getString( 1 ) ) );
                // and add it to the devices element
                root.addContent ( attributes );
            }

            root.addContent( getExtensions() );

            outputTextToBrowser ( root );
        }
        catch( TransformerException ex ) {
             throw new JspException( ex.getMessage());
        }
        catch( RemoteException ex ) {
            throw new JspException( ex.getMessage());
        }
        catch( IOException ex ){
             throw new JspException( ex.getMessage());
        }
        catch( NamingException ex ){
             throw new JspException( ex.getMessage());
        }
        catch ( SQLException e) {
            throw new JspException( e.getMessage());
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


    private Element getExtensions() throws JspException {

        Connection con = null;
        PreparedStatement ps = null;
        ResultSet rs = null;

        try {
            Element root = new Element ( "extensions");
            if ( m_dataSource == null ) {


                InitialContext initial = new InitialContext();

                m_dataSource = (DataSource) initial.lookup( "java:/PDSDataSource" );
            }

            con = m_dataSource.getConnection();

            ps = con.prepareStatement(  "SELECT EXTENSION_NUMBER, STATUS  " +
                                        "FROM EXTENSIONS " +
                                        "WHERE EXT_POOL_ID = ? " );

            ps.setInt( 1, m_extensionPoolID.intValue() );

            rs = ps.executeQuery();

            ArrayList sortedExtensions = new ArrayList();

            while ( rs.next() ) {
                Integer extensionInt = Integer.valueOf ( rs.getString( 1 ) );
                sortedExtensions.add( extensionInt );
            }

            if ( !sortedExtensions.isEmpty() ) {
                Collections.sort( sortedExtensions );

                ArrayList extensionRanges = new ArrayList();
                extensionRanges.add( new ArrayList() );

                for ( Iterator extensionI = sortedExtensions.iterator(); extensionI.hasNext(); ){
                    ArrayList currentRange = (ArrayList) extensionRanges.get( extensionRanges.size() - 1 );

                    Integer nextExtension = (Integer) extensionI.next();

                    if ( !currentRange.isEmpty() ) {
                        Integer lastExtension = (Integer) currentRange.get( currentRange.size() -1 );

                        if ( nextExtension.intValue() == ( lastExtension.intValue() + 1 ) ) {
                            currentRange.add( nextExtension );
                        }
                        else {
                            ArrayList newRange = new ArrayList();
                            newRange.add( nextExtension );
                            extensionRanges.add( newRange );
                        }
                    }
                    else {
                        currentRange.add( nextExtension );
                    }
                }



                for ( Iterator rangesI = extensionRanges.iterator(); rangesI.hasNext(); ) {
                    ArrayList range = (ArrayList) rangesI.next();
                    Element extensionRange = new Element ( "extensionrange");
                    root.addContent( extensionRange );

                    Integer min = (Integer) range.get( 0 );
                    Integer max = (Integer) range.get( range.size() - 1);

                    extensionRange.addContent( new Element ( "min").setText( min.toString() ) );
                    extensionRange.addContent( new Element ( "max").setText( max.toString() ) );
                }

            }

            return root;

        }
        catch( NamingException ex ){
             throw new JspException( ex.getMessage());
        }
        catch ( SQLException e) {
            throw new JspException( e.getMessage());
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
    }



    protected void clearProperties() {
        m_extensionPoolID = null;
        super.clearProperties();
    }


}
