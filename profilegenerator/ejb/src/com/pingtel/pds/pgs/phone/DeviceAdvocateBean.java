/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/phone/DeviceAdvocateBean.java#5 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.phone;

import com.pingtel.pds.common.MD5Encoder;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.XMLSupport;
import com.pingtel.pds.pgs.common.PGSDefinitions;
import com.pingtel.pds.pgs.common.RMIConnectionManager;
import com.pingtel.pds.pgs.common.ejb.InternalToExternalIDTranslator;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.jobs.JobManager;
import com.pingtel.pds.pgs.jobs.JobManagerHome;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationHome;
import com.pingtel.pds.pgs.profile.*;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserAdvocate;
import com.pingtel.pds.pgs.user.UserAdvocateHome;
import com.pingtel.pds.pgs.user.UserHome;
import com.pingtel.pds.sds.ProfileListener;
import org.jdom.CDATA;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.XMLOutputter;

import javax.ejb.*;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import java.io.ByteArrayInputStream;
import java.rmi.RemoteException;
import java.sql.SQLException;
import java.util.*;


/**
 * EJB Implementation class for DeviceAdvocate (session facacde for device-
 * based use cases).
 */
public class DeviceAdvocateBean extends JDBCAwareEJB
    implements SessionBean, PDSDefinitions, PGSDefinitions, DeviceAdvocateBusiness {


    /**
     *  The Session Context object
     */
    private SessionContext m_ctx;

    // Home references
    private ConfigurationSetHome m_csHome;
    private DeviceHome m_deviceHome;
    private DeviceGroupHome m_pgHome;
    private ProjectionHelperHome m_helperHome;
    private UserHome m_userHome;
    private UserAdvocateHome m_userAdvocateHome;
    private OrganizationHome m_organizationHome;
    private RefPropertyHome m_rpHome;

    // Stateless Session Bean references
    private UserAdvocate m_userAdvocateEJBObject;
    private ProjectionHelper m_projectionHelperEJBObject;
    private RenderProfile m_renderProfileEJBObject;
    private JobManager m_jobManagerEJBObject;


    private MD5Encoder m_MD5 = new MD5Encoder();
    private SAXBuilder m_saxBuilder = new SAXBuilder();
    private XMLOutputter m_xmlOut = new XMLOutputter();


    /**
     * createDevice models the create device use case.
     *
     * @param shortName is a short textual identifier.
     * @param organizationID Organization to which this Device belongs.
     * @param coreSoftwareDetailsID the PK of the core software (Entity)
     * which this device uses.
     * @param refConfigSetID PK of the Reference Configuration Set (Entity)
     * which should be associated with this Device (optional).
     * @param description a long free text field (optional).
     * @param deviceGroupID the PK of a DeviceGroup (entity) that this
     * Device should be assigned to (optional).
     * @param userID the PK of a User (entity) that is the owner of
     * this Device.
     * @param deviceTypeID the PK of the DeviceType (entity) that this
     * Device is an instance of (Pingtel VXWorks, NT, etc.)
     * @param serialNumber the unique manufacturers serial number for
     * this device.  For Pingtel phones this is their MAC address.
     *
     * @return the EJBObject for the new Device
     * @exception PDSException is thrown if an Application type error
     * occurs
     */
    public Device createDevice( String shortName,
                                String organizationID,
                                String coreSoftwareDetailsID,
                                String refConfigSetID,
                                String description,   // nullable
                                String deviceGroupID,  // nullable
                                String userID,
                                String deviceTypeID,
                                String serialNumber ) throws PDSException {


        return createDevice(    shortName,
                                new Integer ( organizationID ),
                                new Integer ( coreSoftwareDetailsID ),
                                new Integer ( refConfigSetID ),
                                description,
                                new Integer ( deviceGroupID ),
                                userID,
                                new Integer ( deviceTypeID ),
                                serialNumber );

    }


    /**
     * NOTE: This is the 'local' version of the create device use case.
     * The web UI should use the other version of this method.
     *
     * createDevice models the create device use case.
     *
     * @param shortName is a short textual identifier.
     * @param organizationID Organization to which this Device belongs.
     * @param coreSoftwareDetailsID the PK of the core software (Entity)
     * which this device uses.
     * @param refConfigSetID PK of the Reference Configuration Set (Entity)
     * which should be associated with this Device (optional).
     * @param description a long free text field (optional).
     * @param deviceGroupID the PK of a DeviceGroup (entity) that this
     * Device should be assigned to (optional).
     * @param userID the PK of a User (entity) that is the owner of
     * this Device.
     * @param deviceTypeID the PK of the DeviceType (entity) that this
     * Device is an instance of (Pingtel VXWorks, NT, etc.)
     * @param serialNumber the unique manufacturers serial number for
     * this device.  For Pingtel phones this is their MAC address.
     *
     * @return the EJBObject for the new Device
     * @exception PDSException is thrown if an Application type error
     * occurs
     */
    public Device createDevice( String shortName,
                                Integer organizationID,
                                Integer coreSoftwareDetailsID,
                                Integer refConfigSetID,
                                String description,
                                Integer deviceGroupID,
                                String userID,
                                Integer deviceTypeID,
                                String serialNumber ) throws PDSException {
        Device device = null;
        Organization organization = null;

        try {

            try {
                organization =
                    m_organizationHome.findByPrimaryKey( organizationID );
            }
            catch ( FinderException ex ) {
                this.m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC430",
                                            "E1018",
                                            new Object [] { organizationID } ),
                    ex);
            }

            ///////////////////////////////////////////////////////////////////////
            //
            // Service provider organizations can not contain devices.
            //
            ///////////////////////////////////////////////////////////////////////
            if ( organization.getStereotype() == ORG_SERVICE_PROVIDER ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC430",
                                            "E3011",
                                            null) );
            }

            Device existingDevice = null;
            Collection existing = m_deviceHome.findByShortName( shortName );
            for ( Iterator iExisting = existing.iterator(); iExisting.hasNext(); )
                existingDevice = (Device) iExisting.next();

            if ( existingDevice != null ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC430",
                                            "E2034",
                                            new Object [] { shortName } ) );
            }

            device = m_deviceHome.create(   shortName,
                                            organizationID,
                                            coreSoftwareDetailsID,
                                            refConfigSetID,
                                            description,
                                            deviceGroupID,
                                            userID,
                                            deviceTypeID,
                                            serialNumber );

            if (device.getModel().equals(PDSDefinitions.MODEL_HARDPHONE_XPRESSA) ||
                    device.getModel().equals(PDSDefinitions.MODEL_SOFTPHONE_WIN)){
                createInitialDeviceLine ( device );
            }

            logTransaction ( m_ctx, "Created device " + device.getExternalID() );
        }
        catch (CreateException ce) {
            m_ctx.setRollbackOnly();

            logError( ce.toString(), ce );

            throw new PDSException(
                collateErrorMessages(   "UC430",
                                        "E2001",
                                        new Object[]{   shortName,
                                                        serialNumber } ),
                ce);
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            throw new EJBException(
                collateErrorMessages(   "UC430",
                                        "E2002",
                                        new Object[]{   shortName,
                                                        coreSoftwareDetailsID,
                                                        refConfigSetID,
                                                        description,
                                                        deviceGroupID,
                                                        userID } ) );
        }
        catch ( FinderException e ) {
            m_ctx.setRollbackOnly();

            logError( e.toString(), e );

            throw new PDSException(
                collateErrorMessages(   "UC430",
                                        "E2001",
                                        new Object[]{   shortName,
                                                        serialNumber } ),
                e);
        }

        return device;
    }



    /**
     * Deletes the Device object and its associated entities.
     *
     * @param deviceID the PK of the device to be deleted
     *
     * @exception PDSException is thrown for application level
     * errors.
     */
    public void deleteDevice( String deviceID ) throws PDSException {

        Device device = null;

        try {
            device = m_deviceHome.findByPrimaryKey( new Integer ( deviceID ) );
        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC432",
                    "E1001",
                    new Object[]{deviceID}),
                    ex);
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC432",
                                        "E4001",
                                        new Object[]{ deviceID } ) );
        }

        deleteDevice ( device );
    }


    /**
     * NOTE: This is the 'local' version of the create device use case.
     * The web UI should use the other version of this method.
     *
     * Deletes the Device object and its associated entities.
     *
     * @param device the EJBObject of the device to be deleted
     *
     * @exception PDSException is thrown for application level
     * errors.
     */
    public void deleteDevice( Device device ) throws PDSException {

        String externalID = null;

        try {
            externalID = device.getExternalID();
            Collection csPhone = null;

            try {
                csPhone = m_csHome.findByLogicalPhoneID( device.getID() );
            }
            catch (FinderException ex) {
                logError ( "**FinderException: " + ex.toString() );
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC432",
                                            "E1002",
                                            new Object[]{ externalID } ),
                    ex);
            }

            m_renderProfileEJBObject.deleteProfile( device, PDSDefinitions.PROF_TYPE_PHONE );
            logDebug ( "Deleted PHONE profile for device: " + externalID );

            m_renderProfileEJBObject.deleteProfile( device, PDSDefinitions.PROF_TYPE_USER );
            logDebug ( "Deleted USER profile for device: " + externalID );

            if ( device.getModel().equals( PDSDefinitions.MODEL_HARDPHONE_XPRESSA ) ||
                    device.getModel().equals( PDSDefinitions.MODEL_SOFTPHONE_WIN )) {

                m_renderProfileEJBObject.deleteProfile( device,
                                                        PDSDefinitions.PROF_TYPE_APPLICATION_REF );

                logDebug ( "Deleted APPLICATION profile for device: " + externalID );
            }


            try {
                for (Iterator i = csPhone.iterator(); i.hasNext(); ) {
                    ((ConfigurationSet) i.next()).remove();
                }
            }
            catch (RemoveException ex) {
                logError ( "**RemoveException (CS): " + ex.toString() );
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC432",
                                            "E6002",
                                            new Object[]{ externalID } ),
                    ex);
            }


            try {
                executePreparedUpdate(  "DELETE FROM PROFILE_DATA " +
                                        "WHERE LOG_PN_ID = ?",
                                        new Object[]{ device.getID() });

                logDebug ( "Deleted PROFILE_DATA for device: " + externalID );
                device.remove();
                logDebug ( "Removed device: " + externalID );
            }
            catch (SQLException ex) {
                logError ( "**SQLException: " + ex.toString() );
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC432",
                                            "E6001",
                                            new Object[]{ externalID } ),
                    ex);
            }
            catch (RemoveException ex) {
                logError ( "**RemoveException: " + ex.toString() );
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC432",
                                            "E6012",
                                            new Object[]{ externalID } ),
                    ex);
            }


            logTransaction ( m_ctx, "Deleted device " + externalID );
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC432",
                                        "E4001",
                                        new Object[]{ externalID } ) );
        }
    }

    /**
     * @deprecated copyDevice implements the copy device use case.
     * This is currently deprecated.  The copied device is called
     * 'copy of <source devices name>'.
     * @param sourceDeviceID the PK of the device to be copied.
     * @return the EJBObject of the new (copied) device.
     * @throws PDSException
     */
    public Device copyDevice(String sourceDeviceID) throws PDSException {

        Device source = null;
        Device copy = null;
        Integer sourceID = null;

        try {
            try {
                source =
                        m_deviceHome.findByPrimaryKey(new Integer(sourceDeviceID));

                sourceID = source.getID();
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC433",
                                            "E1001",
                                            new Object[]{sourceID}),
                    ex);
            }

            try {
                copy = createDevice(    "Copy of " + source.getShortName(),
                                        source.getOrganizationID(),
                                        source.getCoreSoftwareDetailsID(),
                                        source.getRefConfigSetID(),
                                        source.getDescription(),
                                        source.getDeviceGroupID(),
                                        null,
                                        source.getDeviceTypeID(),
                                        null );

            }
            catch ( PDSException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException( collateErrorMessages( "UC433" ), ex );
            }

            /*ConfigurationSet sourceCS = null;

            try {
                Collection c = m_csHome.findByLogicalPhoneID(sourceID);

                for (Iterator i = c.iterator(); i.hasNext(); ) {
                    sourceCS = (ConfigurationSet) i.next();
                }
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC433",
                                            "E1002",
                                            new Object[]{ source.getExternalID() }),
                    ex);
            }

            if (sourceCS != null) {
                try {
                    m_csHome.create(    copy.getRefConfigSetID(),
                                        PROF_TYPE_PHONE,
                                        copy,
                                        sourceCS.getContent());
                }
                catch (CreateException ex) {
                    m_ctx.setRollbackOnly();

                    throw new PDSException(
                        collateErrorMessages(   "UC433",
                                                "E2016",
                                                new Object[]{   copy.getExternalID(),
                                                                copy.getRefConfigSetID(),
                                                                sourceCS.getContent()}),
                        ex);
                }

            } // if*/

            String externalID =
                InternalToExternalIDTranslator.getInstance().translate( m_deviceHome,
                                                                        Integer.valueOf( sourceDeviceID ) );

            logTransaction ( m_ctx, "Copied device " + externalID );
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                    collateErrorMessages(   "UC433",
                                            "E4038",
                                            new Object[]{ sourceID } ) );
        }

        return copy;
    }

    /**
     * editDevice implements the edit device use case.   All parameters
     * are optional except the id.  If a null value is passed for any
     * of the optional parameters then one of two things happens.  If
     * the existing value for the Device is null then nothing happens.
     * If there is an existing value then it will be set to null.
     *
     * @param id the PK of the Device that is to be edited.
     * @param userID the PK of the User (entity) that you wish to assign
     * to this Device.
     * @param deviceGroupID the PK of the DeviceGroup (entity) that you
     * wish to assign to this Device.
     * @param refConfigSetID the PK of the RefConfigSet (entity) that you
     * wish to assign to this Device.
     * @param coreSoftwareDetailsID the PK of the CoreSoftwareDetails
     * (entity) that you wish to assign to this Device.
     * @param description free text description for the Device.
     * @param shortName a unique display name associated with the device.
     * @param serialNumber the unique manufacturers serial number for
     * this device.  For Pingtel phones this is their MAC address.
     * @throws PDSException for application level errors.
     */
    public boolean [] editDevice(  String id,
                                String userID,
                                String deviceGroupID,
                                String refConfigSetID,
                                String coreSoftwareDetailsID,
                                String description,
                                String shortName,
                                String serialNumber ) throws PDSException {
        Device device = null;
        boolean fixLine = false;
        boolean [] changedValues = new boolean [ 4 ];

        try {

            try {
                device = m_deviceHome.findByPrimaryKey( Integer.valueOf( id ) );
            }
            catch (FinderException fe) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "UC495",
                                                "E1001",
                                                new String[]{ id } ),
                        fe);
            }

            String existingUserID = device.getUserID();

            if (    (userID != null &&
                        ( existingUserID == null || !userID.equals( existingUserID ) ) ) ) {

                device.setUserID( userID );
            }

            Integer existingDGID = device.getDeviceGroupID();

            if (    ( deviceGroupID != null &&
                    ( existingDGID == null || !deviceGroupID.equals( existingDGID.toString() ) ) ) ||
                    deviceGroupID == null && existingDGID != null ) {

                device.setDeviceGroupID( Integer.valueOf( deviceGroupID ) );
            }

            Integer existingRCSID = device.getRefConfigSetID();

            if (    refConfigSetID != null &&
                    ( existingRCSID == null || !refConfigSetID.equals( existingRCSID.toString() ) ) ) {

                device.setRefConfigSetID( Integer.valueOf( refConfigSetID ));
            }

            Integer existingCSDID = device.getCoreSoftwareDetailsID();

            if (    coreSoftwareDetailsID != null &&
                    ( existingCSDID == null ||  !coreSoftwareDetailsID.equals( existingCSDID.toString() ) ) ) {

                device.setCoreSoftwareDetailsID( Integer.valueOf( coreSoftwareDetailsID ) );
            }

            String existingDescription = device.getDescription();

            if (    ( description != null &&
                        ( existingDescription == null || !description.equals( existingDescription ) ) ) ||
                    description == null && existingDescription != null ) {

                device.setDescription( description );
            }

            String existingShortName = device.getShortName();

            if (    shortName != null &&
                    ( existingShortName == null || !shortName.equals( existingShortName ) ) ) {

                device.setShortName( shortName );
                fixLine = true;
            }

            String existingSerialNumber = device.getSerialNumber();

            if (    ( serialNumber != null &&
                        ( existingSerialNumber == null || !serialNumber.equals( existingSerialNumber ) ) ) ||
                    serialNumber == null && existingSerialNumber != null ) {

                // Delete profiles for the current serial number
                // common behaviour for Pingtel and Cisco devices
                m_renderProfileEJBObject.deleteProfile( device, PDSDefinitions.PROF_TYPE_PHONE );
                m_renderProfileEJBObject.deleteProfile( device, PDSDefinitions.PROF_TYPE_USER );

                if ( device.getManufaturerName().equalsIgnoreCase( "Pingtel" ) ) {
                    m_renderProfileEJBObject.deleteProfile( device, PDSDefinitions.PROF_TYPE_APPLICATION_REF );
                }

                device.setSerialNumber( serialNumber );
                fixLine = true;
            }

            if ( fixLine ) {
                fixDeviceLine( device );
                changedValues [ LINE_INFO_CHANGED ] = true;
            }

            String externalID =
                InternalToExternalIDTranslator.getInstance().translate( m_deviceHome,
                                                                        Integer.valueOf( id ) );

            logTransaction ( m_ctx, "Edited device " + externalID );
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            String userExternal = null;

            try { userExternal = device.getExternalID();} catch ( RemoteException ex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC495",
                                        "E4020",
                                        new String[]{   userExternal == null ?
                                                            id :
                                                            userExternal } ) );
        }
        catch (PDSException e) {
            m_ctx.setRollbackOnly();

            throw new PDSException (collateErrorMessages("UC495"), e);
        }


        return changedValues;
    }


    private void fixDeviceLine ( Device device ) throws PDSException {

        String externalID = null;
        try {
            externalID = device.getExternalID();

            Organization organization =
                m_organizationHome.findByPrimaryKey( device.getOrganizationID() );

            String dnsDomain = organization.getDNSDomain();

            ConfigurationSet cs = null;

            Collection deviceCSC = m_csHome.findByLogicalPhoneID( device.getID() );
            for ( Iterator iCSC = deviceCSC.iterator(); iCSC.hasNext(); )
                cs = (ConfigurationSet) iCSC.next();

            RefProperty deviceLineRP = this.getDeviceLineRP();
            String deviceLineRPID = deviceLineRP.getID().toString();

            Document doc =
                m_saxBuilder.build(
                    new ByteArrayInputStream ( cs.getContent().getBytes() ) );

            Element profile = doc.getRootElement();

            Collection userLines = profile.getChildren( );
            for ( Iterator lineI = userLines.iterator(); lineI.hasNext(); ) {
                Element setting = (Element) lineI.next();
                String rpID = setting.getAttributeValue( "ref_property_id" );

                if ( rpID.equals( deviceLineRPID ) ) {

                    Element holder = setting.getChild( "PHONESET_LINE" );
                    Element url = holder.getChild( "URL" );

                    List l = url.getContent();
                    for ( Iterator iList = XMLSupport.detachableIterator(l.iterator()); iList.hasNext(); ) {
                        CDATA existing = (CDATA) iList.next();
                        url.removeContent( existing );
                    }
                    url.addContent( new CDATA ( device.calculateDeviceLineURL() ) );

                    Collection credentials = holder.getChildren( "CREDENTIAL");
                    for ( Iterator iCred = credentials.iterator(); iCred.hasNext(); ) {
                        Element credential = (Element) iCred.next();

                        String realm = credential.getChild( "REALM" ).getText();
                        if ( realm.equals( dnsDomain ) ) {
                            Element userID = credential.getChild( "USERID" );
                            userID.setText(device.getSerialNumber() + "@" + dnsDomain);

                            String autogenerated = credential.getAttributeValue( "autogenerated" );
                            if ( autogenerated == null )
                                credential.setAttribute( "autogenerated", "true" );

                            Element passtoken = credential.getChild( "PASSTOKEN" );
                            List list = passtoken.getContent();
                            for ( Iterator iList = XMLSupport.detachableIterator(list.iterator()); iList.hasNext(); ) {
                                CDATA existing = (CDATA) iList.next();
                                passtoken.removeContent( existing );
                            }

                            String digestedPassword =
                                m_MD5.encode( device.getShortName() + ":" + new Date().getTime() );

                            passtoken.addContent( new CDATA ( digestedPassword ) );
                        }
                    }
                }
            }

            cs.setContent( m_xmlOut.outputString ( profile ) );
        }
        catch ( java.io.IOException ioex ) {
            if (ioex instanceof RemoteException) {
                logFatal ( ioex.toString(), ioex  );
                throw new EJBException (
                    collateErrorMessages (  "E4068",
                                            new Object [] { externalID } ) );
            }
            logFatal ( ioex.toString(), ioex  );
            throw new EJBException ( collateErrorMessages (  "E4068",
                                    new Object [] { externalID } ) );

        }
        catch ( JDOMException ex ) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages(   "E7008",
                                            new Object[]{ externalID }),
                    ex);

        }
        catch ( FinderException ex ) {
            logFatal ( ex.toString(), ex  );
            throw new EJBException (
                collateErrorMessages (  "E4068",
                                        new Object [] { externalID } ) );

        }
    }




    /**
     * restartDevice sends a signal to an managed Device to tell it to
     * reset itself.  The PGS delegates the work to the SDS (via its
     * RMI interface) as the SDS can talk SIP.
     *
     * @param deviceID the PK of the Device to be reset
     * @throws PDSException for application level errors.
     */
    public void restartDevice ( String deviceID )
        throws PDSException {

        ProfileListener sdsConnection = null;
        String SDSConnectionURL = getPGSProperty( "sds.rmi.url" );
        String deviceExternal = null;

        try {
            sdsConnection = (ProfileListener)
                RMIConnectionManager.getInstance().getConnection(SDSConnectionURL);
        }
        catch ( Exception ex ) {
            logFatal( ex.toString(), ex );
            try {
                RMIConnectionManager.getInstance().reestablishConnection(SDSConnectionURL);
            }
            catch (Exception e) {
                logFatal( ex.toString(), ex );

                throw new EJBException(
                        collateErrorMessages(   "UC466",
                                                "E4064",
                                                new Object[]{ deviceExternal } ) );
            }

            throw new EJBException(
                    collateErrorMessages(   "UC466",
                                            "E8004",
                                            new Object[]{ SDSConnectionURL } ) );
        }

        try {
            Device device = null;
            try {
                device =
                    m_deviceHome.findByPrimaryKey( Integer.valueOf( deviceID ) );

                deviceExternal = device.getExternalID();
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                    throw new PDSException(
                        collateErrorMessages(   "UC466",
                                                "E1001",
                                                new Object[]{ deviceID }),
                        ex);
            }


            String model = device.getModel();
            // see it's a pingtel model
            if ( model.equalsIgnoreCase( PDSDefinitions.MODEL_HARDPHONE_XPRESSA ) ||
                 model.equalsIgnoreCase( PDSDefinitions.MODEL_SOFTPHONE_WIN ) ||
                 model.equalsIgnoreCase( PDSDefinitions.MODEL_SOFTPHONE_LINUX ) ||
                 model.equalsIgnoreCase( PDSDefinitions.MODEL_SOFTPHONE_SPARC_SOLARIS ) ) {
                sdsConnection.resetPhone( device.getSerialNumber(), null );
            }
            else {// 79XX
                sdsConnection.resetPhone(
                        null,
                        m_renderProfileEJBObject.getDeviceNotifyURL(device));
            }

            String externalID =
                InternalToExternalIDTranslator.getInstance().translate( m_deviceHome,
                                                                        Integer.valueOf( deviceID ) );

            logTransaction ( m_ctx, "Reset device " + externalID );
        }
        catch (RemoteException ex ) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                    collateErrorMessages(   "UC466",
                                            "E4064",
                                            new Object[]{ deviceExternal } ) );
        }

    }




    private void createInitialDeviceLine ( Device device ) throws PDSException {

        String deviceExternal = null;

        try {
            Organization org = null;
            try {
                org = m_organizationHome.findByPrimaryKey( device.getOrganizationID() );
                logDebug ( "found organization: " + org.getExternalID() );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "E1018",
                                                new Object[]{ device.getOrganizationID() }),
                        ex);
            }

            deviceExternal = device.getExternalID();

            String deviceLineURL = device.calculateDeviceLineURL();
            String userID = device.getSerialNumber() + "@" + org.getDNSDomain();

            RefProperty rp = getDeviceLineRP();
            StringBuffer xmlContent = new StringBuffer();
            xmlContent.append( "<PROFILE>" +
                                "<PHONESET_LINE ref_property_id=\"" + rp.getID() + "\">" );

            String digestedPassword =
                m_MD5.encode(   userID + ":" + new Date().getTime()  );

            xmlContent.append( "<PHONESET_LINE>" );
            xmlContent.append( "<ALLOW_FORWARDING>" + CDATAIt ( "DISABLE" ) + "</ALLOW_FORWARDING>" );
            xmlContent.append( "<REGISTRATION>" + CDATAIt ( "REGISTER" ) + "</REGISTRATION>" );
            xmlContent.append( "<URL>" + CDATAIt ( deviceLineURL ) + "</URL>");
            xmlContent.append( "<CREDENTIAL autogenerated=\"true\">" );
            xmlContent.append( "<REALM>" + CDATAIt ( org.getDNSDomain() ) + "</REALM>" );
            xmlContent.append( "<USERID>" + CDATAIt ( userID ) + "</USERID>" );
            xmlContent.append( "<PASSTOKEN>" + CDATAIt (  digestedPassword ) + "</PASSTOKEN>" );
            xmlContent.append( "</CREDENTIAL>" );
            xmlContent.append( "</PHONESET_LINE>" );
            xmlContent.append( "</PHONESET_LINE>" );
            xmlContent.append( "</PROFILE>" );

            try {
                m_csHome.create(    device.getRefConfigSetID(),
                                    PDSDefinitions.PROF_TYPE_PHONE,
                                    device,
                                    xmlContent.toString() );
            }
            catch ( CreateException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "UC430",
                                                "E2016",
                                                new Object[]{   deviceExternal,
                                                                device.getRefConfigSetID(),
                                                                xmlContent } ),
                        ex);
            }
        }
        catch ( RemoteException ex ) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC430",
                                        "E4077",
                                        new Object[]{ deviceExternal } ) );
        }
    }



    private RefProperty getDeviceLineRP () throws PDSException {
        RefProperty rp = null;

        try {
            try {
                Collection c = m_rpHome.findByCode( "xp_1031" );
                for ( Iterator i = c.iterator(); i.hasNext(); )
                    rp = (RefProperty) i.next();
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "E1029",
                                                new Object[]{ "xp_1031" }),
                        ex);
            }
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex  );
            throw new EJBException ( ex.toString() );
        }

        return rp;
    }



    /**
     * generateProfiles creates the Device's profiles (configuration files)
     * which are written to disk by the ProfileWriter (which is called over
     * it's RMI interface).   If the Device is currently enrolled then it
     * is notified that new profiles exist for it.
     *
     * @param deviceID the PK of the Device which is to be projected.
     * @param profileTypes a comma-separated list of profile types.  The
     * valid values are PDSDefinitions.PROF_TYPE_USER,
     * PDSDefinitions.PROF_TYPE_DEVICE and
     * PDSDefinitions.PROF_TYPE_APPLICATION_REF.
     * @param projectionAlgorithm the fully qualified class name for the
     * projection algorithm (optional).  If none is supplied then the
     * default algorithm is used.
     * @throws PDSException is thrown for application level errors.
     */
    public void generateProfiles(   String deviceID,
                                    String profileTypes,
                                    String projectionAlgorithm) throws PDSException {

        Device device = null;

        logDebug(   "starting generateProfiles for device: " + deviceID +
                    " profile types: " + profileTypes );

        try {
            device = m_deviceHome.findByPrimaryKey( new Integer ( deviceID ));
        }
        catch (FinderException fe) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC465",
                    "E1001",
                    new Object[]{deviceID}),
                    fe);
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            throw new EJBException(
                collateErrorMessages(   "UC465",
                                        "E4002",
                                        new Object[]{ deviceID } ) );
        }

        generateProfiles (  device,
                            profileTypes,
                            projectionAlgorithm,
                            true );
    }


    /**
     * NOTE: This is the 'local' version of the generate profiles use case.
     * The web UI should use the other version of this method.
     *
     * generateProfiles creates the Device's profiles (configuration files)
     * which are written to disk by the ProfileWriter (which is called over
     * it's RMI interface).   If the Device is currently enrolled then it
     * is notified that new profiles exist for it.
     *
     * @param device the EJBObject of the Device which is to be projected.
     * @param profileTypes a comma-separated list of profile types.  The
     * valid values are PDSDefinitions.PROF_TYPE_USER,
     * PDSDefinitions.PROF_TYPE_DEVICE and
     * PDSDefinitions.PROF_TYPE_APPLICATION_REF.
     * @param projectionAlgorithm the fully qualified class name for the
     * projection algorithm (optional).  If none is supplied then the
     * default algorithm is used.
     * @throws PDSException is thrown for application level errors.
     */
    public void generateProfiles(   Device device,
                                    String profileTypes,
                                    String projectionAlgorithm,
                                    boolean separateProjection)
            throws PDSException {

        boolean[] profTypesToCreate = null;
        int jobID = -1;

        try {
            try {
                profTypesToCreate = ProfileTypeStringParser.parse(profileTypes);
            }
            catch (PDSException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException( collateErrorMessages( "UC465" ), ex );
            }


            if ( separateProjection ) {
                StringBuffer jobDetails = new StringBuffer ();
                jobDetails.append( "Projection for device: ");
                jobDetails.append( device.getExternalID());
                jobDetails.append ( " profile types: ");

                if ( !device.getModel().equals( PDSDefinitions.MODEL_HARDPHONE_CISCO_7940 ) &&
                    !device.getModel().equals( PDSDefinitions.MODEL_HARDPHONE_CISCO_7960 )) {

                    if ( profTypesToCreate[ PDSDefinitions.PROF_TYPE_APPLICATION_REF ] ) {
                        jobDetails.append ( "application ");
                    }
                }

                if ( profTypesToCreate[ PDSDefinitions.PROF_TYPE_USER ] ) {
                        jobDetails.append ( " user ");
                    }
                if ( profTypesToCreate[ PDSDefinitions.PROF_TYPE_PHONE ] ) {
                    jobDetails.append ( " device");
                }


                jobID =
                    m_jobManagerEJBObject.createJob(    JobManager.PROJECTION,
                                                        jobDetails.toString(),
                                                        JobManager.STARTED );

                m_jobManagerEJBObject.updateJobProgress( jobID, "projected 0 of 1 devices.");
            }

            if (    profTypesToCreate[PROF_TYPE_USER] ||
                    profTypesToCreate[PROF_TYPE_APPLICATION_REF]) {

                generateUserCentricProfiles(
                        device,
                        projectionAlgorithm,
                        profTypesToCreate[PROF_TYPE_USER],
                        profTypesToCreate[PROF_TYPE_APPLICATION_REF]);

            }

            if (profTypesToCreate[PROF_TYPE_PHONE]) {
                generatePhoneProfile( device, projectionAlgorithm);
            }

            if ( separateProjection ) {
                m_jobManagerEJBObject.updateJobProgress( jobID, "projected 1 of 1 devices.");
                m_jobManagerEJBObject.updateJobStatus( jobID, JobManager.COMPLETE, null );
            }
        }
        catch ( Exception e) {
            try {
                m_jobManagerEJBObject.updateJobStatus( jobID, JobManager.FAILED , e.toString() );
            } catch ( Exception e1) { e1.printStackTrace(); }

            if ( e instanceof RemoteException ) {
                logFatal ( e.toString() );
                throw new EJBException ( e.toString() );
            }
            else if ( e instanceof PDSException ){
                throw new PDSException ( e.getMessage() );
            }
            else {
                // add assert here some time.
            }
        }
    }




    private void generatePhoneProfile( Device device, String projAlg )
        throws PDSException {

        ArrayList configSets = new ArrayList();

        try {

            try {
                configSets.addAll( getProjectionInputs(device) );
            }
            catch (PDSException ex) {
                m_ctx.setRollbackOnly();
                throw new PDSException( collateErrorMessages( "UC465" ) , ex);
            }

            try {
                m_projectionHelperEJBObject.projectAndPersist(  projAlg,
                                                                device,
                                                                PROF_TYPE_PHONE,
                                                                configSets);
            }
            catch (PDSException pe) {
                m_ctx.setRollbackOnly();

                throw new PDSException( collateErrorMessages( "UC465" ), pe );
            }
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            String deviceExternal = null;
            try { deviceExternal = device.getExternalID(); } catch ( RemoteException rex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC465",
                                        "E4002",
                                        new Object[]{ deviceExternal } ) );
        }
    }



    private void generateUserCentricProfiles(   Device device,
                                                String projAlg,
                                                boolean shouldDoUser,
                                                boolean shouldDoApplication)
        throws PDSException {

        User owner = null;
        StringBuffer profileTypes = new StringBuffer();

        try {
            // Check that the device is assigned to a user.
            if ( device.getUserID() != null ) {
                try {
                    owner = m_userHome.findByPrimaryKey(device.getUserID());
                }
                catch (FinderException ex) {
                    m_ctx.setRollbackOnly();

                    throw new PDSException(
                        collateErrorMessages(   "UC465",
                                                "E1005",
                                                new Object[]{ device.getExternalID() } ),
                        ex);
                }

                if (shouldDoUser) {
                    profileTypes.append(PROF_TYPE_USER);
                }

                if ( !device.getModel().equals( PDSDefinitions.MODEL_HARDPHONE_CISCO_7940 ) &&
                                    !device.getModel().equals( PDSDefinitions.MODEL_HARDPHONE_CISCO_7960 )) {

                    if (shouldDoApplication) {
                        if (profileTypes.length() != 0) {
                            profileTypes.append(',');
                        }

                        profileTypes.append(PROF_TYPE_APPLICATION_REF);
                    }
                }

                m_userAdvocateEJBObject.generateUserProfiles(   owner,
                                                                device,
                                                                profileTypes.toString(),
                                                                projAlg);
            } // if user not null
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

             String deviceExternal = null;

            try { deviceExternal = device.getExternalID(); }catch ( RemoteException dex ) {}

            throw new EJBException(
                    collateErrorMessages("UC465",
                    "E4002",
                    new Object[]{deviceExternal}));
        }
        catch (PDSException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException( collateErrorMessages( "UC465" ), ex);
        }
    }



    private Collection getProjectionInputs(Device device)
             throws PDSException, RemoteException {

        ArrayList configSets = new ArrayList();
        Integer deviceGroupID = null;

        try {
            deviceGroupID = device.getDeviceGroupID();
            if ( deviceGroupID != null ) {
                DeviceGroup pg = m_pgHome.findByPrimaryKey(deviceGroupID);
                configSets.addAll(m_projectionHelperEJBObject.addParentGroupConfigSets(pg));
                Collections.reverse(configSets);
            }
            ProjectionInput deviceInput = m_projectionHelperEJBObject.getProjectionInput(device);
            if ( deviceInput != null )
                configSets.add( deviceInput );

        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC465",
                                        "E1003",
                                        new Object[]{deviceGroupID}),
                ex);
        }

        return configSets;
    }


   /**
    * fetchSequenceNumber is used by the SDS when a device enrolls with it
    * to discover the newest version of each profile for that device.
    *
    * @param serialNumber the serial number (MAC address for Pingtel phones)
    * of the Device you want the sequence number for.
    * @param profileType valid values are PDSDefinitions.PROF_TYPE_USER,
    * PDSDefinitions.PROF_TYPE_DEVICE and
    * PDSDefinitions.PROF_TYPE_APPLICATION_REF.
    * @return the sequence number for the given Device's profile type.
    * @throws PDSException for application level errors.
    */
    public int fetchSequenceNumber( String serialNumber, int profileType)
        throws PDSException {


        Device device = null;
        int sequenceNumber = -1;

        try {

            Collection devices = null;

            try {
                devices = m_deviceHome.findBySerialNumber ( serialNumber );

                for (Iterator iter = devices.iterator(); iter.hasNext(); ) {
                    device = (Device) iter.next();
                }

                if (device == null) {
                    throw new FinderException();
                }
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC745",
                                            "E1008",
                                            new Object[]{ serialNumber } ),
                    ex);
            }

            ArrayList results = null;

            try {
                results =
                        executePreparedQuery(   "SELECT SEQ_NUMBER " +
                                                "FROM   PROFILE_DATA " +
                                                "WHERE  LOG_PN_ID = ? " +
                                                "AND    TYPE = ?",
                                                new Object[]{device.getID(),
                                                new Integer(profileType)},
                                                1,
                                                1);
            }
            catch (SQLException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC745",
                                            "E1006",
                                            new Object[]{   serialNumber,
                                                            new Integer(profileType)}),
                    ex);
            }

            ArrayList row = null;

            for (Iterator iResults = results.iterator(); iResults.hasNext(); ) {
                row = (ArrayList) iResults.next();

                sequenceNumber = new Integer((String) row.get(0)).intValue();
            }

        }
        catch (RemoteException ex) {
            m_ctx.setRollbackOnly();
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC745",
                                        "E4003",
                                        new Object[]{   serialNumber,
                                                        new Integer(profileType)}));
        }

        return sequenceNumber;
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

            m_csHome =
                    (ConfigurationSetHome) initial.lookup("ConfigurationSet");
            m_deviceHome = (DeviceHome) initial.lookup("Device");
            m_pgHome = (DeviceGroupHome) initial.lookup("DeviceGroup");
            m_helperHome = (ProjectionHelperHome) initial.lookup("ProjectionHelper");

            m_userHome = (UserHome) initial.lookup("User");
            m_userAdvocateHome =
                    (UserAdvocateHome) initial.lookup("UserAdvocate");

            m_organizationHome =
                (OrganizationHome) initial.lookup( "Organization" );

            m_rpHome =
                (RefPropertyHome) initial.lookup( "RefProperty" );

            RenderProfileHome m_renderProfileHome =
                (RenderProfileHome) initial.lookup( "RenderProfile" );

            JobManagerHome m_jobManagerHome =
                    (JobManagerHome) initial.lookup ("JobManager");

            m_renderProfileEJBObject = m_renderProfileHome.create();
            m_userAdvocateEJBObject = m_userAdvocateHome.create();
            m_projectionHelperEJBObject = m_helperHome.create();
            m_jobManagerEJBObject = m_jobManagerHome.create();
        }
        catch (NamingException ne) {
            logFatal( ne.toString(), ne);
            throw new EJBException(ne);
        }
        catch (CreateException ne) {
            logFatal( ne.toString(), ne);
            throw new EJBException(ne);
        }
        catch (RemoteException ne) {
            logFatal( ne.toString(), ne);
            throw new EJBException(ne);
        }
    }


    /**
     *  Standard Boiler Plate Session Bean Method Implementation
     */
    public void unsetSessionContext() {
        m_ctx = null;
    }
}
