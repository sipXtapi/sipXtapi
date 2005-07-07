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

/*
 *  error messages updated
 */
package com.pingtel.pds.pgs.profile;

import java.io.ByteArrayInputStream;
import java.rmi.RemoteException;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;

import javax.ejb.CreateException;
import javax.ejb.EJBException;
import javax.ejb.FinderException;
import javax.ejb.RemoveException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.PropertyGroupLink;
import com.pingtel.pds.pgs.common.PGSDefinitions;
import com.pingtel.pds.pgs.common.MasterDetailsMap;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.phone.DeviceType;
import com.pingtel.pds.pgs.phone.DeviceTypeHome;
import com.pingtel.pds.pgs.phone.Manufacturer;
import com.pingtel.pds.pgs.phone.ManufacturerHome;

/**
 *  Description of the Class
 *
 *@author     ibutcher
 *@created    December 13, 2001
 */
public class RefDataAdvocateBean extends JDBCAwareEJB
         implements SessionBean, PDSDefinitions, PGSDefinitions, RefDataAdvocateBusiness {


    // Bean Context
    private SessionContext m_ctx;

    // Home interface objects

    private RefPropertyHome m_rpHome;
    private RefConfigurationSetHome m_rcsHome;
    private DeviceTypeHome m_dtHome;
    private ManufacturerHome m_manuHome;

    // Misc.
    private SAXBuilder m_saxBuilder = new SAXBuilder();


    private final static String CREATE_REF_PROP_RCS_ASSOC_SQL =
            "CREATE_REF_PROP_RCS_ASSOC_SQL";

    private final static String DELETE_REF_PROP_RCS_ASSOC_SQL =
            "DELETE_REF_PROP_RCS_ASSOC_SQL";

    private final static Integer INT_PROF_TYPE_PHONE =
            new Integer(PROF_TYPE_PHONE);

    private String m_createRPRCSAssocSQL;
    private String m_deleteRPRCSAssocSQL;

    private HashMap m_refPropertyEJBObjectMap = new HashMap();
    private HashMap m_RCSEJBObjectMap = new HashMap();
    private MasterDetailsMap m_deviceTypeRPCache = new MasterDetailsMap();


    /**
     *  Mask for mask parameter for assignRefPropertytoRCS
     */

    public final static Integer FINAL = new Integer(1);
    /**
     *  Description of the Field
     */
    public final static Integer APPENDABLE = new Integer(2);
    /**
     *  Description of the Field
     */
    public final static Integer OVERRIDABLE = new Integer(3);


    public RefProperty createRefProperty(   String name,
                                            String profileType,
                                            String code,
                                            String content)
        throws PDSException {

        return createRefProperty(   name,
                                    new Integer ( profileType ).intValue(),
                                    code,
                                    content);
    }

    /**
     *  Description of the Method
     *
     *@param  name                  Description of the Parameter
     *@param  refPropertyGroupID    Description of the Parameter
     *@param  deviceTypeID           Description of the Parameter
     *@param  profileType           Description of the Parameter
     *@param  content               Description of the Parameter
     *@return                       Description of the Return Value
     *@exception  PDSException  Description of the Exception
     */
    public RefProperty createRefProperty(   String name,
                                            int profileType,
                                            String code,
                                            String content)
        throws PDSException {

        RefProperty rp = null;

        try {
            Collection c = this.m_rpHome.findByCode( code );

            if ( c.isEmpty() ) {
                rp = m_rpHome.create(   name,
                                        profileType,
                                        code,
                                        content);
            }
            else {
                for ( Iterator i = c.iterator(); i.hasNext(); ) {
                    rp = (RefProperty) i.next();
                }
            }


            // New code to clear the Device type / ref property cache

            /**@todo when we externalize the device type/ref property mapping
            xml doc we should put the flushing code in the use case which
            assigns ref properties to device types. */
            m_deviceTypeRPCache.clear();
        }
        catch ( FinderException ex ) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC530",
                                        "E2013",
                                        new Object[]{   name,
                                                        new Integer ( profileType ),
                                                        content}),
                ex);
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            throw new EJBException(
                collateErrorMessages(   "UC530",
                                        "E4029",
                                        new Object[]{   name,
                                                        new Integer ( profileType ),
                                                        content}));
        }
        catch (CreateException ce) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC530",
                                        "E2013",
                                        new Object[]{   name,
                                                        new Integer ( profileType ),
                                                        content}),
                ce);
        }

        return rp;
    }



    /**
     *  Description of the Method
     *
     *@param  name                  Description of the Parameter
     *@param  organizationID        Description of the Parameter
     *@return                       Description of the Return Value
     *@exception  PDSException  Description of the Exception
     */
    public RefConfigurationSet createRefConfigurationSet(   String name,
                                                            String organizationID)
        throws PDSException {

        RefConfigurationSet rcs = null;

        try {
            rcs = m_rcsHome.create(name, new Integer(organizationID));
        }
        catch (CreateException ce) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC520",
                                        "E2014",
                                        new String[]{   name,
                                                        organizationID}),
                ce);
        }
        catch (RemoteException re) {
            throw new EJBException(
                collateErrorMessages(   "UC520",
                                        "E4030",
                                        new String[]{   name,
                                                        organizationID}));
        }

        return rcs;
    }


     public void deleteRefConfigSet( String refConfigSetID)
        throws PDSException {

        RefConfigurationSet rcs = null;

        try {
            rcs = getRefConfigurationSet ( new Integer ( refConfigSetID ) );
        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC540",
                                        "E1021",
                                        new Object[]{refConfigSetID}),
                ex);
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC540",
                                        "E4016",
                                        new Object[]{refConfigSetID}));
        }

        deleteRefConfigSet ( rcs );
    }

    /**
     *  Description of the Method
     *
     *@param  refConfigSetID        Description of the Parameter
     *@exception  PDSException  Description of the Exception
     */
    public void deleteRefConfigSet( RefConfigurationSet rcs )
        throws PDSException {

        Integer rcsID = null;

        try {
            try {
                executePreparedUpdate(  "DELETE FROM CS_PROPERTY_PERMISSIONS " +
                                        "WHERE RCS_ID = ? ",
                                        new Object[]{ rcs.getID() });

                rcs.remove();
            }
            catch (SQLException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC540",
                                            "E6007",
                                            new Object[]{ rcs.getExternalID() }),
                    ex);
            }
            catch (RemoveException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC540",
                                            "E6008",
                                            new Object[]{ rcs.getExternalID() }),
                    ex);
            }
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            String external = null;

            try { external = rcs.getExternalID(); } catch ( RemoteException rex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC540",
                                        "E4016",
                                        new Object[]{external}));
        }
    }


    public void assignRefPropertytoRCS( String refConfigSetID,
                                        RefConfigSetAssignment rule)
        throws PDSException {

        RefConfigurationSet rcs = null;

        try {
            rcs = getRefConfigurationSet ( new Integer ( refConfigSetID ) );
        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC500",
                    "E1021",
                    new Object[]{refConfigSetID}),
                    ex);
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            throw new EJBException(
                    collateErrorMessages("UC500",
                    "E4031",
                    new Object[]{   rule.getRefPropertyID(),
                                    refConfigSetID}));
        }

        assignRefPropertytoRCS( rcs, rule);
    }


    /**
     *  Description of the Method
     *
     *@param  refConfigSetID        Description of the Parameter
     *@param  rule                  Description of the Parameter
     *@exception  PDSException  Description of the Exception
     */
    public void assignRefPropertytoRCS( RefConfigurationSet rcs,
                                        RefConfigSetAssignment rule)
        throws PDSException {

        RefProperty rp = null;
        //RefConfigurationSet rcs = null;
        try {
        logDebug ( "assignRefPropertyToRCS RCS: " + rcs.getExternalID() +
            " rule: " + rule.getRefPropertyID() );

        }
        catch ( RemoteException ex ) {
            ex.printStackTrace();
        }

        Integer refPropertyID = rule.getRefPropertyID();

        try {
            try {
                rp = m_rpHome.findByPrimaryKey(refPropertyID);
                //logDebug ( "found ref property: " + rp.getExternalID() );
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages("UC500",
                        "E1029",
                        new Object[]{refPropertyID}),
                        ex);
            }


            try {
                // print out complete statement.
                executePreparedUpdate(  m_createRPRCSAssocSQL,
                                        new Object[]{rcs.getID(),
                                        rp.getID(),
                                        new Integer(rule.getIsReadOnly() ? 1 : 0),
                                        new Integer(rule.getIsFinal() ? 1 : 0)});

            }
            catch (SQLException se) {
                m_ctx.setRollbackOnly();
                logFatal ( se.toString(), se );

                throw new PDSException(
                    collateErrorMessages(   "UC500",
                                            "E5008",
                                            new Object[]{   rp.getExternalID() ,
                                                            rcs.getExternalID()}),
                    se);
            }

        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            String external = null;

            try { external = rcs.getExternalID();} catch ( RemoteException rex ) {}

            throw new EJBException(
                    collateErrorMessages("UC500",
                    "E4031",
                    new Object[]{   refPropertyID,
                                    external}));
        }
    }


    /**
     *@param  refConfigSetID        is the String representation of the PK of
     *      the Ref Config Set that you are trying to remove a Ref Property
     *      from.
     *@param  refPropertyID         is the String representation of the PK of
     *      the Ref Property that you are trying to remove from the Ref Config
     *      Set.
     *@exception  PDSException  is thrown if an application level error
     *      occurs.
     */
    public void unassignRefPropertyFromRCS( String refConfigSetID,
                                            String refPropertyID)
        throws PDSException {

        HashMap thingsToProject = new HashMap();

        Integer rcsID = new Integer(refConfigSetID);
        Integer rpID = new Integer(refPropertyID);
        RefConfigurationSet rcs = null;
        RefProperty rp = null;

        try {
            try {
                rp = m_rpHome.findByPrimaryKey(rpID);
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC510",
                                            "E1029",
                                            new Object[]{refPropertyID}),
                    ex);
            }

            try {
                //rcs = m_rcsHome.findByPrimaryKey(rcsID);
                rcs = getRefConfigurationSet (rcsID);
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC510",
                                            "E1021",
                                            new Object[]{refConfigSetID}),
                    ex);
            }

            try {
                executePreparedUpdate(  m_deleteRPRCSAssocSQL,
                                        new Object[]{rcs.getID(), rp.getID()});
            }
            catch (SQLException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC510",
                                            "E5007",
                                            new Object[]{   rp.getExternalID(),
                                                            rcs.getExternalID() }),
                    ex);
            }

        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC510",
                                        "E4032",
                                        new Object[]{   refPropertyID,
                                                        refConfigSetID}));
        }
    }


    /**
     *  getRefConfigSetsProperties returns a Collection of ProjectionRules
     *
     *@param  refConfigSetID        the String representation of the PK of the
     *      Ref Configuration Set that you want to find which Reference
     *      Properties meta data for.
     *@return                       a Collection of PDSPropertyMetaData objects.
     *@exception  PDSException  is thrown for application level exceptions.
     *@see                          com.pingtel.pds.pgs.profile.PDSPropertyMetaData
     */
    public Collection getRefConfigSetsProperties(String refConfigSetID )
             throws PDSException {

        RefConfigurationSet rcs = null;
        ArrayList properties = new ArrayList();
        RefProperty rp = null;

        try {

            ArrayList results = null;

            try {
                results =   executePreparedQuery(   "SELECT CS.REF_PROP_ID, " +
                                                    "       CS.IS_FINAL,  " +
                                                    "       CS.IS_READ_ONLY  " +
                                                    "FROM   CS_PROPERTY_PERMISSIONS CS " +
                                                    "WHERE  CS.RCS_ID = ? ",
                                                    new Object[]{refConfigSetID},
                                                    3,
                                                    1000000);
            }
            catch (SQLException ex) {
                this.m_ctx.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages("UC180",
                        "E1030",
                        new Object[]{ rcs.getExternalID()}),
                        ex);
            }

            ArrayList row = null;

            Integer refPropertyID = null;

            for (int i = 0; i < results.size(); ++i) {
                row = (ArrayList) results.get(i);

                boolean isFinal = false;
                boolean isReadOnly = false;
                refPropertyID =  Integer.valueOf( ( String ) row.get( 0 ) );


                if (row.get(1).equals(this.PR_FINAL)) {
                    isFinal = true;
                }

                if (row.get(2).equals(this.PR_READ_ONLY)) {
                    isFinal = true;
                }

                String cardinality = getCardiniality ( refPropertyID );

                ProjectionRule pr = new ProjectionRule( refPropertyID,
                                                        isFinal,
                                                        isReadOnly,
                                                        cardinality );

                properties.add(pr);
            }
        }
        catch ( RemoteException ex ) {
            logFatal( ex.toString(), ex  );

            throw new EJBException(
                collateErrorMessages(   "UC180",
                                        "E4036",
                                        new Object[]{ refConfigSetID } ) );
        }


        return properties;
    }


    /**
     *  Gets the refConfigSetsProperty attribute of the
     *  ReferenceDataAdvocateBean object
     *
     *@param  refConfigSetID        Description of the Parameter
     *@param  refPropertyID         Description of the Parameter
     *@return                       The refConfigSetsProperty value
     *@exception  PDSException  Description of the Exception
     */
    public ProjectionRule getRefConfigSetsProperty(String refConfigSetID,
            String refPropertyID)
             throws PDSException {

        RefConfigurationSet rcs = null;
        ProjectionRule pr = null;
        ArrayList properties = new ArrayList();
        Integer rcsID = new Integer(refConfigSetID);
        Integer rpID = new Integer(refPropertyID);

        try {

            try {
                rcs = getRefConfigurationSet(rcsID);
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC510",
                                            "E1021",
                                            new Object[]{refConfigSetID}),
                    ex);
            }

            ArrayList results = null;

            try {
                results =
                        executePreparedQuery("SELECT CS.REF_PROP_ID, " +
                        "       CS.IS_FINAL,  " +
                        "       CS.IS_READ_ONLY,  " +
                        "FROM   CS_PROPERTY_PERMISSIONS CS " +
                        "WHERE  CS.RCS_ID = ? " +
                        "AND    CS.REF_PROP_ID = ?",
                        new Object[]{rcsID, rpID},
                        3,
                        1);
            }
            catch (SQLException ex) {
                this.m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC180",
                                            "E1030",
                                            new Object[]{ rcs.getExternalID()}),
                    ex);
            }

            ArrayList row = null;

            for (int i = 0; i < results.size(); ++i) {
                row = (ArrayList) results.get(i);

                boolean isFinal = false;
                boolean isReadOnly = false;
                Integer intRefPropertyID = new Integer( ( String ) row.get( 0 ) );

                if (row.get(1).equals(this.PR_FINAL)) {
                    isFinal = true;
                }

                if (row.get(2).equals(this.PR_READ_ONLY)) {
                    isFinal = true;
                }

                String cardinality = getCardiniality ( intRefPropertyID );

                pr = new ProjectionRule(    intRefPropertyID,
                                            isFinal,
                                            isReadOnly,
                                            cardinality);

            }
        }
        catch ( RemoteException ex ) {
            logFatal( ex.toString(), ex  );

            throw new EJBException(
                collateErrorMessages(   "UC180",
                                        "E4036",
                                        new Object[]{ refConfigSetID } ) );
        }

        return pr;
    }


    private String getCardiniality (Integer refPropertyID) {
        return RefPropertyCache.getInstance().getCardinality(refPropertyID);
    }


    public Collection getRefPropertiesForDeviceType ( Integer deviceTypeID )
        throws PDSException {

        return getRefPropertiesForDeviceTypeAndProfile ( deviceTypeID, null );
    }



    public Collection getRefPropertiesForDeviceTypeAndProfile ( Integer deviceTypeID, Integer profileType )
        throws PDSException {


        if ( !m_deviceTypeRPCache.contains( deviceTypeID, profileType ) ) {

            try {
                DeviceType devType = null;

                try {
                    devType = m_dtHome.findByPrimaryKey( deviceTypeID );
                }
                catch ( FinderException ex ) {
                    m_ctx.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages (  "E1033",
                                                new Object [] { deviceTypeID } ),
                        ex );
                }

                logDebug ( "found device type: " + devType.getExternalID() );

                Manufacturer manu = null;

                try {
                    manu = m_manuHome.findByPrimaryKey( devType.getManufacturerID() );
                }
                catch ( FinderException ex ) {
                    m_ctx.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages (  "E1034",
                                                new Object [] { devType.getManufacturerID() } ),
                        ex );
                }

                logDebug ( "found manufacturer: " + manu.getExternalID() );

                String manufacturerName = manu.getName();

                ArrayList validRefPropIDs = new ArrayList();

                Element root = PropertyGroupLink.getInstance().getMappings();
                Collection manufacturers = root.getChildren( "manufacturer" );
                for ( Iterator iMan = manufacturers.iterator(); iMan.hasNext(); ) {
                    Element manufacturer = (Element) iMan.next();
                    String name = manufacturer.getAttributeValue( "name" );

                    if ( name.equalsIgnoreCase( manufacturerName ) ) {

                        Collection deviceTypes = manufacturer.getChildren( "device_type" );
                        for ( Iterator iDT = deviceTypes.iterator(); iDT.hasNext(); ) {
                            Element deviceType = (Element) iDT.next();
                            String dtName = deviceType.getAttributeValue( "model" );

                            if (    dtName.equalsIgnoreCase( "common" ) ||
                                    dtName.equalsIgnoreCase( devType.getModel() ) ) {
                                Collection rpgs = deviceType.getChildren( "ref_property_group" );

                                for ( Iterator iRPGS = rpgs.iterator(); iRPGS.hasNext(); ) {
                                    Element refPropertyGroup = (Element) iRPGS.next();

                                    String userType =
                                        refPropertyGroup.getAttributeValue( "usertype");

                                    if ( !userType.equalsIgnoreCase( "user" ) ) {

                                        Collection refProperties =
                                            refPropertyGroup.getChildren( "ref_property");

                                        for ( Iterator iRP = refProperties.iterator(); iRP.hasNext(); ) {
                                            Element refProperty = (Element) iRP.next();

                                            String refPropertyCode = refProperty.getAttributeValue( "code" );

                                            try {
                                                RefProperty rp = null;
                                                Collection rpC =
                                                    m_rpHome.findByCode( refPropertyCode );
                                                for ( Iterator iCode = rpC.iterator(); iCode.hasNext(); )
                                                    rp = (RefProperty) iCode.next();

                                                if ( profileType != null ) {
                                                    if ( rp.getProfileType() == profileType.intValue() ) {
                                                        validRefPropIDs.add( rp );
                                                    }
                                                }
                                                else {
                                                    validRefPropIDs.add( rp );
                                                }
                                            }
                                            catch ( FinderException ex ) {
                                                ex.printStackTrace();
                                            }
                                        } // for ref properties in group
                                    }

                                } // for ref property groups in dev type

                            } // if device type

                        } // for device types in manu

                        break;
                    } // if manufacturer

                } // for manufacturers

                m_deviceTypeRPCache.storeDetail( deviceTypeID, profileType, validRefPropIDs );
            }
            catch ( RemoteException ex ) {
                logDebug ( ex.toString() );
                throw new EJBException ( ex );
            }

        } // if

        return (ArrayList) m_deviceTypeRPCache.getDetail( deviceTypeID, profileType );
    }


    private RefProperty getRefProperty ( Integer id )
        throws FinderException, RemoteException {

        RefProperty returnValue = null;

        if ( !m_refPropertyEJBObjectMap.containsKey( id ) ) {
            returnValue = m_rpHome.findByPrimaryKey( id );
            m_refPropertyEJBObjectMap.put( id, returnValue );
        }
        else {
            returnValue = (RefProperty) m_refPropertyEJBObjectMap.get( id );
        }

        return returnValue;
    }



    private RefConfigurationSet getRefConfigurationSet ( Integer id )
        throws FinderException, RemoteException {

        RefConfigurationSet returnValue = null;

        if ( !m_RCSEJBObjectMap.containsKey( id ) ) {
            returnValue = m_rcsHome.findByPrimaryKey( id );
            m_refPropertyEJBObjectMap.put( id, returnValue );
        }
        else {
            returnValue = (RefConfigurationSet) m_RCSEJBObjectMap.get( id );
        }

        return returnValue;
    }

    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbCreate() { }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbRemove() { }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbActivate() { }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void ejbPassivate() { }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     *
     *@param  ctx  The new sessionContext value
     */
    public void setSessionContext(SessionContext ctx) {
        m_ctx = ctx;

        try {
            Context initial = new InitialContext();
            m_createRPRCSAssocSQL = getEnvEntry(CREATE_REF_PROP_RCS_ASSOC_SQL);
            m_deleteRPRCSAssocSQL = getEnvEntry(DELETE_REF_PROP_RCS_ASSOC_SQL);
            m_rpHome = (RefPropertyHome) initial.lookup("RefProperty");
            m_rcsHome = (RefConfigurationSetHome)
                    initial.lookup("RefConfigurationSet");
            m_dtHome = (DeviceTypeHome) initial.lookup( "DeviceType" );
            m_manuHome = (ManufacturerHome) initial.lookup( "Manufacturer" );

        } catch (NamingException ne) {
            logFatal( ne.toString(), ne );
            throw new EJBException(ne);
        }
    }


    /**
     *  Description of the Method
     */
    public void unsetSessionContext() {
        m_ctx = null;
    }

}
