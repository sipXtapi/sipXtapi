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

package com.pingtel.pds.pgs.profile;

import java.rmi.RemoteException;

import java.sql.SQLException;

import java.io.ByteArrayInputStream;
import java.io.UnsupportedEncodingException;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.IOException;

import java.util.Collection;
import java.util.Iterator;
import java.util.HashMap;
import java.util.ArrayList;

import javax.ejb.CreateException;
import javax.ejb.EJBException;
import javax.ejb.EntityBean;
import javax.ejb.EntityContext;
import javax.ejb.RemoveException;
import javax.ejb.FinderException;

import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import org.jdom.Element;
import org.jdom.Document;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.XMLOutputter;
import org.xml.sax.InputSource;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.XMLSupport;
import com.pingtel.pds.pgs.common.PGSDefinitions;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.phone.Device;
import com.pingtel.pds.pgs.phone.DeviceGroup;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserGroup;
import com.pingtel.pds.pgs.profile.propertysort.*;
import com.pingtel.pds.pgs.profile.RefProperty;
import com.pingtel.pds.pgs.profile.RefPropertyHome;

/**
 *  Description of the Class
 *
 *@author     ibutcher
 *@created    December 13, 2001
 */
public class ConfigurationSetBean extends JDBCAwareEJB
         implements EntityBean, PDSDefinitions, PGSDefinitions, ConfigurationSetBusiness {

    /**
     *  Description of the Field
     */
    public Integer id;
    /**
     *  Description of the Field
     */
    public Integer refConfigSetID;
    /**
     *  Description of the Field
     */
    public int profileType;
    /**
     *  Description of the Field
     */
    public String content;


    /**
     *  Gets the iD attribute of the ConfigurationSetBean object
     *
     *@return    The iD value
     */
    public Integer getID() {
        return this.id;
    }


    /**
     *  Gets the refConfigSetID attribute of the ConfigurationSetBean object
     *
     *@return    The refConfigSetID value
     */
    public Integer getRefConfigSetID() {
        return this.refConfigSetID;
    }


    /**
     *  Sets the refConfigSetID attribute of the ConfigurationSetBean object
     *
     *@param  refConfigSetID  The new refConfigSetID value
     */
    public void setRefConfigSetID(Integer refConfigSetID) {
        this.refConfigSetID = refConfigSetID;
    }


    /**
     *  Gets the content attribute of the ConfigurationSetBean object
     *
     *@return                       The content value
     */
    public String getContent() {
        return content;
    }


    /**
     *  Sets the content attribute of the ConfigurationSetBean object
     *
     *@param  content  The new content value
     */
    public void setContent(String content) {
        String sortedString = null;

        try {
            /////////////////////////////////////////////////////////////
            //
            // sortedValues is an optimization.  One we have encountered
            // (and sorted) the first instance of a ref property we have
            // sorted them all.   We use sortedValues to keep track of
            // the ones that we have encountered already
            //
            /////////////////////////////////////////////////////////////
            HashMap sortedValues = new HashMap();
            Element root = m_saxBuilder.build(new InputSource(new StringReader(content))).getRootElement();

            Collection allProperties = root.getChildren();
            
            for ( Iterator iAllProps = XMLSupport.detachableIterator(allProperties.iterator()); iAllProps.hasNext(); ) {

                Element property = (Element) iAllProps.next();

                String refPropertyID = property.getAttributeValue( "ref_property_id" );
                if ( !sortedValues.containsKey( refPropertyID ) ) {

                    // Need to look up appropriate ref property
                    RefProperty refProperty = null;
                    try {
                        refProperty =
                            m_rpHome.findByPrimaryKey( Integer.valueOf( refPropertyID ) );
                    }
                    catch ( FinderException ex ) {
                        throw new EJBException  ( ex.getMessage() );
                    }

                    String stringContent = refProperty.getContent();
                    Document contentDoc = m_saxBuilder.build(new StringReader(stringContent));

                    Element definition = contentDoc.getRootElement();

                    String cardinality = definition.getAttributeValue( "cardinality");
                    /////////////////////////////////////////////////////////////
                    //
                    // Only 'N' cardinality properties can have > 1 instance.
                    //
                    /////////////////////////////////////////////////////////////
                    if (    cardinality.equals( "1..N" ) ||
                            cardinality.equals( "0..N" ) ) {

                        ArrayList peers = new ArrayList();
                        peers.add(property.detach());

                        for ( Iterator iClone = XMLSupport.detachableIterator(allProperties.iterator()); iClone.hasNext(); ) {
                            Element e = (Element) iClone.next();
                            String rpID = e.getAttributeValue( "ref_property_id" );
                            if ( rpID.equals( refPropertyID ) ) {
                                peers.add( e.detach() );
                            }
                        }

                        PropertySorter propertySorter = null;

                        String [] sortingInfo = new String [2];
                        getSortingInformation( definition, sortingInfo );
                        String sortingMethod = sortingInfo [ 0 ];
                        String sortingValue = sortingInfo [ 1 ];

                        if ( sortingMethod.equals( "alphanum-caseinsensitive-asc" ) ) {
                            propertySorter = new AlphanumInsensitiveAscending ();
                        }
                        else if ( sortingMethod.equals( "alphanum-casesensitive-asc" ) ) {
                            /*@todo */
                            propertySorter = new AlphanumInsensitiveAscending ();
                        }
                        else if ( sortingMethod.equals( "alphanum-caseinsensitive-des" ) ) {
                            propertySorter = new AlphanumInsensitiveDescending ();
                        }
                        else if ( sortingMethod.equals( "alphanum-casesensitive-des" ) ) {
                            /*@todo */
                            propertySorter = new AlphanumInsensitiveDescending ();
                        }
                        else if ( sortingMethod.equals( "numeric-asc" ) ) {
                            propertySorter = new NumericAscending ();
                        }
                        else {
                            throw new EJBException ( "bad sorting method" );
                        }

                        for ( Iterator iSort = peers.iterator(); iSort.hasNext(); ) {
                            Element sortElement = (Element) iSort.next();
                            String key = null;
                            key = getPropertySortingKey( sortElement, sortingValue );
                            propertySorter.addProperty( sortElement, key );
                        }

                        Collection sortedProperties = propertySorter.getValues();
                        for ( Iterator iSortedProps = sortedProperties.iterator(); iSortedProps.hasNext(); ) {
                            Element sorted = (Element) iSortedProps.next();
                            root.addContent( sorted );
                        }

                        sortedValues.put( refPropertyID, new Object() );

                    } // if card. N
                }
            }
            StringWriter stringWriter = new StringWriter();
            m_xmlOut.output(root, stringWriter);
            sortedString = stringWriter.toString();
        }
        catch (Exception ex) {
            logFatal(ex.getMessage());
            throw new EJBException (ex.getMessage());
        }

        this.content = sortedString;
//        this.content = content;
    }

    public int getProfileType () {
        return this.profileType;
    }


    public String getExternalID () {
        return "id: " + this.id;
    }

    private void setID() {
        this.id = new Integer(getNextSequenceValue("CONFIG_SET_SEQ"));
    }


    // Helper methods

    private boolean getSortingInformation ( Element node, String [] info ) {

        info [0] = node.getAttributeValue( "sort" );
        info [1] = node.getAttributeValue( "name" );

        if ( info [0] == null || info [0].length() == 0 ) {
            Collection children = node.getChildren();
            for ( Iterator i = children.iterator(); i.hasNext(); ) {
                if ( getSortingInformation ( (Element) i.next(), info ) )
                    return true;
            }
        }

        return true;
    }


    private String getPropertySortingKey ( Element node, String valueName ) {

        if ( !node.getName().equals( valueName ) ) {
            String str = null;
            Collection children = node.getChildren();
            for ( Iterator i = children.iterator(); i.hasNext(); ) {
                str = getPropertySortingKey ( (Element) i.next(), valueName );
                if ( str != null )
                    break;
            }

            return str;
        }

        return node.getText();
    }


    // EJ Bean callback methods


    /**
     *  Description of the Method
     *
     *@param  refConfigSet         Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@param  lp                   Description of the Parameter
     *@param  xmlContent           Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  CreateException  Description of the Exception
     */
    public Integer ejbCreate(Integer refConfigSet,
            int profileType, Device lp,
            String xmlContent) throws CreateException {

        commonCreate(refConfigSet, profileType, xmlContent);
        return null;
    }


    /**
     *  Description of the Method
     *
     *@param  refConfigSet         Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@param  user                 Description of the Parameter
     *@param  xmlContent           Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  CreateException  Description of the Exception
     */
    public Integer ejbCreate(Integer refConfigSet,
            int profileType,
            User user,
            String xmlContent) throws CreateException {

        commonCreate(refConfigSet, profileType, xmlContent);
        return null;
    }


    /**
     *  Description of the Method
     *
     *@param  refConfigSet         Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@param  pg                   Description of the Parameter
     *@param  xmlContent           Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  CreateException  Description of the Exception
     */
    public Integer ejbCreate(Integer refConfigSet,
            int profileType, DeviceGroup pg,
            String xmlContent) throws CreateException {

        commonCreate(refConfigSet, profileType, xmlContent);
        return null;
    }


    /**
     *  Description of the Method
     *
     *@param  refConfigSet         Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@param  ug                   Description of the Parameter
     *@param  xmlContent           Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  CreateException  Description of the Exception
     */
    public Integer ejbCreate(Integer refConfigSet,
            int profileType,
            UserGroup ug,
            String xmlContent) throws CreateException {

        commonCreate(refConfigSet, profileType, xmlContent);
        return null;
    }


    private void commonCreate(Integer refConfigSet,  int profileType, String content) {
        setID();
        setRefConfigSetID(refConfigSet);
        this.profileType = profileType;
        this.content = content;
    }


    /**
     *  Description of the Method
     *
     *@param  refConfigSet         Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@param  lp                   Description of the Parameter
     *@param  xmlContent           Description of the Parameter
     *@exception  CreateException  Description of the Exception
     */
    public void ejbPostCreate(Integer refConfigSet,
            int profileType, Device lp,
            String xmlContent)
             throws CreateException {

        try {
            executePreparedUpdate("INSERT INTO LOG_PHONE_CS_ASSOC (   CS_ID, " +
                    "                                   LOG_PN_ID ) " +
                    "VALUES ( ?,? )",
                    new Object[]{this.id, lp.getID()});

        } catch (SQLException ex) {
            m_ctx.setRollbackOnly();

            throw new CreateException(ex.toString());
        } catch (RemoteException ex) {
            m_ctx.setRollbackOnly();
            logFatal( ex.toString(), ex );

            throw new EJBException(ex.toString());
        }
    }


    /**
     *  Description of the Method
     *
     *@param  refConfigSet         Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@param  user                 Description of the Parameter
     *@param  xmlContent           Description of the Parameter
     *@exception  CreateException  Description of the Exception
     */
    public void ejbPostCreate(Integer refConfigSet,
            int profileType,
            User user,
            String xmlContent) throws CreateException {

        try {
            executePreparedUpdate("INSERT INTO USER_CS_ASSOC (    CS_ID, " +
                    "                               USRS_ID ) " +
                    "VALUES ( ?,? )",
                    new Object[]{this.id, user.getID()});

        } catch (SQLException ex) {
            m_ctx.setRollbackOnly();

            throw new CreateException(ex.toString());
        } catch (RemoteException ex) {
            m_ctx.setRollbackOnly();
            logFatal( ex.toString(), ex );

            throw new EJBException(ex.toString());
        }
    }


    /**
     *  Description of the Method
     *
     *@param  refConfigSet         Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@param  pg                   Description of the Parameter
     *@param  xmlContent           Description of the Parameter
     *@exception  CreateException  Description of the Exception
     */
    public void ejbPostCreate(Integer refConfigSet,
            int profileType, DeviceGroup pg,
            String xmlContent) throws CreateException {

        try {
            executePreparedUpdate("INSERT INTO PNG_CS_ASSOC ( CS_ID, " +
                    "                           PG_ID ) " +
                    "VALUES ( ?,? )",
                    new Object[]{this.id, pg.getID()});

        } catch (SQLException ex) {
            m_ctx.setRollbackOnly();

            throw new CreateException(ex.toString());
        } catch (RemoteException ex) {
            m_ctx.setRollbackOnly();
            logFatal( ex.toString(), ex );

            throw new EJBException(ex.toString());
        }
    }


    /**
     *  Description of the Method
     *
     *@param  refConfigSet         Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@param  ug                   Description of the Parameter
     *@param  xmlContent           Description of the Parameter
     *@exception  CreateException  Description of the Exception
     */
    public void ejbPostCreate(Integer refConfigSet,
            int profileType,
            UserGroup ug,
            String xmlContent) throws CreateException {

        try {
            executePreparedUpdate("INSERT INTO UG_CS_ASSOC (  CS_ID, " +
                    "                           UG_ID ) " +
                    "VALUES ( ?,? )",
                    new Object[]{this.id, ug.getID()});

        } catch (SQLException ex) {
            m_ctx.setRollbackOnly();

            throw new CreateException(ex.toString());
        } catch (RemoteException ex) {
            m_ctx.setRollbackOnly();
            logFatal( ex.toString(), ex );

            throw new EJBException(ex.toString());
        }
    }



    /**
     *  Description of the Method
     */
    public void ejbLoad() { }


    /**
     *  Description of the Method
     */
    public void ejbStore() { }


    /**
     *  Description of the Method
     *
     *@exception  RemoveException  Description of the Exception
     */
    public void ejbRemove() throws RemoveException {

        Object[] sqlArgs = new Object[]{this.id};

        try {

            if (executePreparedUpdate("DELETE FROM USER_CS_ASSOC WHERE CS_ID = ?",
                    sqlArgs) == 1) {
            }
            // we have deleted a row therefore we don't have to
            //look/delete any others.
            else if (executePreparedUpdate("DELETE FROM LOG_PHONE_CS_ASSOC WHERE CS_ID = ?",
                    sqlArgs)
                     == 1) {
                // do nothing
            } else if (executePreparedUpdate("DELETE FROM PNG_CS_ASSOC WHERE CS_ID = ?",
                    sqlArgs)
                     == 1) {
                // do nothing
            } else if (executePreparedUpdate("DELETE FROM UG_CS_ASSOC WHERE CS_ID = ?",
                    sqlArgs)
                     == 1) {
            } else {
                // we didn't find any record - very bad

                throw new RemoveException("Could not find any associations for " +
                        "Configuration Set: " +
                        this.getID());
            }

        } catch (SQLException se) {
            m_ctx.setRollbackOnly();

            throw new RemoveException(se.toString());
        }
    }


    /**
     *  Description of the Method
     */
    public void ejbActivate() { }


    /**
     *  Description of the Method
     */
    public void ejbPassivate() { }


    /**
     *  Sets the entityContext attribute of the ConfigurationSetBean object
     *
     *@param  ctx  The new entityContext value
     */
    public void setEntityContext(EntityContext ctx) {
        m_ctx = ctx;

        try {
            Context initial = new InitialContext();
            m_rpHome = (RefPropertyHome) initial.lookup( "RefProperty" );
        }
        catch ( NamingException ex ) {
            throw new EJBException ( ex.toString() );
        }
    }


    /**
     *  Description of the Method
     */
    public void unsetEntityContext() {
        m_ctx = null;
    }


    private EntityContext m_ctx;

    private RefPropertyHome m_rpHome;

    private SAXBuilder m_saxBuilder = new SAXBuilder();
    private XMLOutputter m_xmlOut = new XMLOutputter();
}
