/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/entity/EntityDetailsModifierBean.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.pgs.entity;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.PGSDefinitions;
import com.pingtel.pds.pgs.common.ProfileEncryptionCache;
import com.pingtel.pds.pgs.common.ejb.InternalToExternalIDTranslator;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.phone.*;
import com.pingtel.pds.pgs.profile.ConfigurationSet;
import com.pingtel.pds.pgs.profile.ConfigurationSetHome;
import com.pingtel.pds.pgs.sipxchange.datasets.DataSetBuilderFacade;
import com.pingtel.pds.pgs.user.*;
import org.jdom.Document;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.xml.sax.InputSource;

import javax.ejb.*;
import javax.jms.*;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.StringReader;
import java.rmi.RemoteException;
import java.util.*;


/**
 * EntityDetailsModifierBean is the bean implemenation class for the
 * EntityDetailsModifier EJ Bean.  Its purpose is to provide the
 * back-end functions for 'save' on the User, UserGroup, Device and
 * DeviceGroup details pages.
 *
 * @see #updateEntityDetails
 * @author IB
 */
public class EntityDetailsModifierBean extends JDBCAwareEJB
    implements SessionBean, EntityDetailsModifierBusiness {

//////////////////////////////////////////////////////////////////////////
// Constants
////


//////////////////////////////////////////////////////////////////////////
// Attributes
////

    // standard session bean stuff
    private SessionContext mCTX;

    // Home i/f references
    private ConfigurationSetHome mConfigurationSetHome = null;
    private DeviceHome mDeviceHome = null;
    private DeviceGroupHome mDeviceGroupHome = null;
    private UserHome mUserHome = null;
    private UserGroupHome mUserGroupHome = null;
    private UserAdvocateHome mUserAdvocateHome = null;
    private DeviceAdvocateHome mDeviceAdvocateHome = null;

    private UserAdvocate mUserAdvocate = null;
    private DeviceAdvocate mDeviceAdvocate = null;
    private UserGroupAdvocate mUserGroupAdvocate = null;
    private DeviceGroupAdvocate mDeviceGroupAdvocate = null;

    // Misc.
    private JSStringToXMLConverter mJavaScriptToXMLConverter =
            new JSStringToXMLConverter();
    private ConfigurationSetMerger mConfigurationSetMerger =
            new ConfigurationSetMerger();

    private SAXBuilder mSAXBuilder = new SAXBuilder();

    // used to cache whether this is an entprise of (in the future) a
    // service provider installation.
    private boolean mIsEnterprise;

//////////////////////////////////////////////////////////////////////////
// Construction
////


//////////////////////////////////////////////////////////////////////////
// Public Methods
////

    /**
     *  Standard EJB implementation
     */
    public void ejbCreate() { }

    /**
     *  Standard EJB implementation
     */
    public void ejbRemove() { }

    /**
     *  Standard EJB implementation
     */
    public void ejbActivate() { }

    /**
     *  Standard EJB implementation
     */
    public void ejbPassivate() { }

    /**
     *  Standard EJB implementation
     */
    public void setSessionContext(SessionContext ctx) {
        mCTX = ctx;

        if(getInstallStereoType().equalsIgnoreCase(PDSDefinitions.ENTERPRISE_ST)){
            mIsEnterprise = true;
        }

        try {
            Context initial = new InitialContext();

            mConfigurationSetHome =
                    (ConfigurationSetHome) initial.lookup("ConfigurationSet");

            mDeviceHome =
                    (DeviceHome) initial.lookup("Device");

            mDeviceGroupHome =
                    (DeviceGroupHome) initial.lookup("DeviceGroup");
            mUserHome = (UserHome) initial.lookup("User");
            mUserGroupHome = (UserGroupHome) initial.lookup("UserGroup");
            mUserAdvocateHome =
                    (UserAdvocateHome) initial.lookup("UserAdvocate");

            mDeviceAdvocateHome =
                    (DeviceAdvocateHome) initial.lookup("DeviceAdvocate");

            UserGroupAdvocateHome ugaHome =
                (UserGroupAdvocateHome) initial.lookup("UserGroupAdvocate");

            DeviceGroupAdvocateHome dgaHome =
                (DeviceGroupAdvocateHome) initial.lookup("DeviceGroupAdvocate");

            mUserAdvocate = mUserAdvocateHome.create();
            mDeviceAdvocate = mDeviceAdvocateHome.create();
            mUserGroupAdvocate = ugaHome.create();
            mDeviceGroupAdvocate = dgaHome.create();
        }
        catch (Exception ex) {
            logFatal (ex.toString(), ex);
            throw new EJBException (ex);
        }

    }

    /**
     *  Standard EJB implementation
     */
    public void unsetSessionContext() {
        mCTX = null;
    }

    /**
     * updateEntityDetails takes the input fields names and values from the
     * details pages (User,Device,UserGroup and DeviceGroup), saves any
     * values to the that entities configuration set and takes any further
     * action needed.   This includes triggering the (re)building of our
     * sipxchange datasets and changing the actual attributes of the entities
     * themselves (changing user's names, extension, etc.)
     *
     * @param formStrings the form fields/inputs from the details pages
     * @throws PDSException for application errors
     */
    public void updateEntityDetails (Collection formStrings)
                throws PDSException {

        boolean linesChanged = false;
        boolean permissionsChanged = false;
        boolean forwardingPropertiesChanged = false;

        boolean [] attributesChanged =
                new boolean [PGSDefinitions.USER_CHANGE_ARRAY_SIZE];

        attributesChanged [PGSDefinitions.ALIASES_CHANGED] = false;
        attributesChanged [PGSDefinitions.EXTENSION_CHANGED] = false;
        attributesChanged [PGSDefinitions.LINE_INFO_CHANGED] = false;
        attributesChanged [PGSDefinitions.HIERARCHY_CHANGED] = false;

        String entityType = null;
        String entityID = null;
        ArrayList inputStrings = new ArrayList();
        HashMap generalStrings = new HashMap();

        for (   Iterator formStringsI = formStrings.iterator();
                formStringsI.hasNext(); ) {

            String line = (String) formStringsI.next();
            StringTokenizer st = new StringTokenizer ( line, "^^^" );
            String paramName = st.nextToken();
            String paramValue = null;

            if ( st.hasMoreTokens() )
                paramValue = st.nextToken();

            if ( paramName.equals( "x_entity_id" ) ) {
                entityID = paramValue;
                logDebug( "x_entity_id is " + entityID );
                continue;
            }
            else if ( paramName.equals( "x_entity_type" ) ) {
                entityType = paramValue;
                logDebug( "x_entity_type is " + entityType );
                continue;
            }
            else if ( paramName.startsWith( "general") ) {
                generalStrings.put(
                        new String(paramName.substring(paramName.indexOf(".")+1)),
                        paramValue == null ? null : paramValue );
            }
            else if ( paramName.equals( "redir" ) ) {
                continue;
            }
            else {
                logDebug ( "adding string " +  line );

                if (    line.indexOf( "PRIMARY_LINE") != -1 ||
                        line.indexOf( "PHONESET_LINE") != -1 ||
                        line.indexOf( "USER_LINE") != -1 ) {

                    linesChanged = true;
                }
                else if ( line.indexOf( "PERMISSIONS") != -1 ) {
                    permissionsChanged = true;
                }
                else if (line.indexOf ("SIP_FORWARD_ON_BUSY") != -1 ||
                            line.indexOf ( "SIP_FORWARD_ON_NO_ANSWER") != -1 ||
                            line.indexOf ( "SIP_FORWARD_UNCONDITIONAL") != -1) {

                    forwardingPropertiesChanged = true;
                }

                inputStrings.add( line );
            }
        }

        if ( inputStrings != null ){
            updateEntityConfigSet (inputStrings, entityType, entityID);
        }

        if ( generalStrings.size() > 0 ){
            attributesChanged =
                    updateEntityAttributes (
                            generalStrings, entityType, entityID);
        }

        String externalID = null;

        if (entityType.equals("user")){
            externalID =
                InternalToExternalIDTranslator.getInstance().translate(
                        mUserHome,
                        entityID);
        }
        else if (entityType.equals("usergroup")){
            externalID =
                InternalToExternalIDTranslator.getInstance().translate(
                        mUserGroupHome,
                        Integer.valueOf(entityID) );
        }
        else if (entityType.equals("device")){
            externalID =
                InternalToExternalIDTranslator.getInstance().translate(
                        mDeviceHome,
                        Integer.valueOf(entityID));
        }
        else {
            externalID =
                InternalToExternalIDTranslator.getInstance().translate(
                        mDeviceGroupHome,
                        Integer.valueOf(entityID));
        }


        ////////////////////////////////////////////////////////////////////////
        //
        // The following code determines whether to generate any of the XML
        // databases which get sent to the other sipxchange servers.   The
        // logic is pretty simple, if the line information changed (user ID,
        // password, etc. got modified) then we regenerate all.   In all other
        // cases we look at the boolean values which are returned from
        // updateEntityAttributes to see if we need to regenerated.
        //
        ////////////////////////////////////////////////////////////////////////
        if(mIsEnterprise){

            if (attributesChanged[PGSDefinitions.LINE_INFO_CHANGED] ||
                    attributesChanged[PGSDefinitions.EXTENSION_CHANGED] ||
                    linesChanged) {
                sendJMSMessage ("credentials", null, null);
            }

            if (attributesChanged[PGSDefinitions.ALIASES_CHANGED] ||
                    attributesChanged[PGSDefinitions.LINE_INFO_CHANGED] ||
                    attributesChanged[PGSDefinitions.EXTENSION_CHANGED] ||
                    linesChanged) {
                sendJMSMessage ("aliases", null, null);
            }

            if (attributesChanged[PGSDefinitions.EXTENSION_CHANGED] ||
                    attributesChanged[PGSDefinitions.LINE_INFO_CHANGED] ||
                    linesChanged ) {
                sendJMSMessage ("extensions", null, null);
            }

            ////////////////////////////////////////////////////////////////////
            //
            // In the case of permissions we do an OR to see if either the
            // boolean array value has changed (this will happen if the user
            // group ID is changed for a user/user group) OR if a PERMISSIONS
            // setting has been modified for this user/user group.
            //
            ////////////////////////////////////////////////////////////////////
            if (permissionsChanged ||
                    attributesChanged[PGSDefinitions.HIERARCHY_CHANGED] ||
                    attributesChanged[PGSDefinitions.LINE_INFO_CHANGED] ||
                    linesChanged ) {
                sendJMSMessage ("permissions", null, null);
            }

            if (forwardingPropertiesChanged ||
                    permissionsChanged ||
                    attributesChanged [ PGSDefinitions.HIERARCHY_CHANGED ]) {
                sendJMSMessage ("authexceptions", entityType, entityID);
            }
        }

        logTransaction( mCTX, "Updated details for " +  entityType + " " +
                externalID );
    }


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    private void sendJMSMessage (   String contentType,
                                    String changePointId,
                                    String changePointType ) {

        if(mIsEnterprise){
            try {
                Context context = new InitialContext();

                QueueConnectionFactory queueFactory =
                    (QueueConnectionFactory)context.lookup(
                            "QueueConnectionFactory");

                QueueConnection queueConnection =
                        queueFactory.createQueueConnection();

                QueueSession queueSession =
                        queueConnection.createQueueSession(
                                false,
                                Session.AUTO_ACKNOWLEDGE);

                Queue queue = (Queue)context.lookup("queue/B");
                QueueSender queueSender = queueSession.createSender(queue);

                MapMessage message = queueSession.createMapMessage();
                message.setString ("datasettype", contentType);

                if(changePointId != null){
                    message.setString(
                            DataSetBuilderFacade.CHANGEPOINT_ID_KEY,
                            changePointId);
                }

                if(changePointId != null){
                    message.setString(
                            DataSetBuilderFacade.CHANGEPOINT_TYPE_KEY,
                            changePointType);
                }

                queueSender.send(queue, message);
            }
            catch (NamingException e) {
                throw new EJBException( e.getMessage());
            }
            catch (JMSException e) {
                throw new EJBException( e.getMessage());
            }
        }
    }


    private void updateEntityConfigSet (    Collection formConfigSetStrings,
                                            String entityType,
                                            String entityID )
            throws PDSException {

        try {
            // call JSStringToXMLConvert.convert on them
            String newXMLString = mJavaScriptToXMLConverter.convert( formConfigSetStrings );

            //logDebug( "new content: " + newXMLString );
            logDebug( "entity type is: " + entityType );
            logDebug( "entity id: " + entityID);

            ConfigurationSet cs = null;
            Collection csC = null;


            if ( entityType.equals( "device") ) {

                Device device =
                        mDeviceHome.findByPrimaryKey(new Integer(entityID));

                ProfileEncryptionCache.getInstance().flushEntry(device);

                try {
                    csC =
                            mConfigurationSetHome.findByLogicalPhoneID(
                                    new Integer(entityID));
                }
                catch ( FinderException ex ) {
                    mCTX.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages ( "E1002", new Object [] {entityID} ),
                        ex );
                }
            }
            else if ( entityType.equals( "devicegroup") ) {
                // totally flushing the cache is much quicker than
                // recursively finding all of the Device in this
                // or any child groups and then flushing each one.
                ProfileEncryptionCache.getInstance().flushAll();

                try {
                    csC = mConfigurationSetHome.findByDeviceGroupID(
                            new Integer(entityID));
                }
                catch ( FinderException ex ) {
                    mCTX.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages ( "E1010", new Object [] {entityID} ),
                        ex );
                }
            }
            else if ( entityType.equals( "user") ) {
                try {
                    csC = mConfigurationSetHome.findByUserID( entityID );
                }
                catch ( FinderException ex ) {
                    mCTX.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages ( "E1024", new Object [] {entityID} ),
                        ex );
                }
            }
            else {
                try {
                    csC = mConfigurationSetHome.findByUserGroupID( new Integer ( entityID ) );
                }
                catch ( FinderException ex ) {
                    mCTX.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages ( "E1031", new Object [] {entityID} ),
                        ex );
                }
            }


            for ( Iterator csI = csC.iterator(); csI.hasNext(); )
                cs = (ConfigurationSet) csI.next();

            // check to see if we need to create a new configuration set
            if ( cs == null ) {

                // This is a nasty hack.  We call the merging code as it has a very
                // nice routine which removes the blank settings from the values
                // that we get back from the HTML from.  Refactor some day.
                ByteArrayInputStream bas =
                        new ByteArrayInputStream ( newXMLString.getBytes());

                Document masterXMLContent = mSAXBuilder.build( bas );

                String mergedXMLString =
                        mConfigurationSetMerger.merge(
                                masterXMLContent.getRootElement(),
                                newXMLString );

                try {
                    if ( entityType.equals( "device") ) {

                        Device device =
                                mDeviceHome.findByPrimaryKey(
                                        new Integer(entityID));

                        try {
                            mConfigurationSetHome.create(  device.getRefConfigSetID(),
                                              PDSDefinitions.PROF_TYPE_PHONE,
                                              device,
                                              mergedXMLString );
                        }
                        catch ( CreateException ex ) {
                            mCTX.setRollbackOnly();

                            throw new PDSException (
                                collateErrorMessages (  "E2016",
                                                        new Object [] { entityID,
                                                        device.getRefConfigSetID(),
                                                        mergedXMLString } ),
                                ex );

                        }

                    } else if ( entityType.equals( "devicegroup") ) {

                        DeviceGroup deviceGroup = null;
                        try {
                            deviceGroup =
                                    mDeviceGroupHome.findByPrimaryKey(
                                            new Integer(entityID));

                            mConfigurationSetHome.create(
                                    deviceGroup.getRefConfigSetID(),
                                    PDSDefinitions.PROF_TYPE_PHONE,
                                    deviceGroup,
                                    mergedXMLString );
                        }
                        catch ( CreateException ex ) {
                            mCTX.setRollbackOnly();

                            throw new PDSException (
                                collateErrorMessages (  "E2017",
                                                        new Object [] { entityID,
                                                        deviceGroup.getRefConfigSetID(),
                                                        mergedXMLString } ),
                                ex );
                        }

                    } else if ( entityType.equals( "user") ) {
                        User user = null;
                        try {
                            user = mUserHome.findByPrimaryKey( entityID );

                            mConfigurationSetHome.create( user.getRefConfigSetID(),
                                    PDSDefinitions.PROF_TYPE_USER,
                                    user,
                                    mergedXMLString );
                        }
                        catch ( CreateException ex ) {
                            mCTX.setRollbackOnly();

                            throw new PDSException (
                                collateErrorMessages (  "E2018",
                                                        new Object [] { entityID,
                                                        user.getRefConfigSetID(),
                                                        "user",
                                                        mergedXMLString } ),
                                ex );
                        }
                    } else {
                        UserGroup userGroup = null;
                        try {
                            userGroup =
                                    mUserGroupHome.findByPrimaryKey(
                                            new Integer(entityID));

                            mConfigurationSetHome.create(
                                    userGroup.getRefConfigSetID(),
                                    PDSDefinitions.PROF_TYPE_USER,
                                    userGroup,
                                    mergedXMLString );
                        }
                        catch ( CreateException ex ) {
                            mCTX.setRollbackOnly();

                            throw new PDSException (
                                collateErrorMessages (
                                        "E2019",
                                        new Object [] {
                                            entityID,
                                            userGroup.getRefConfigSetID(),
                                            "user",
                                            mergedXMLString } ),
                                ex );
                        }

                    }
                }
                catch ( FinderException ex ) {
                    mCTX.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages ( "E1049",
                                new Object [] { entityType, entityID } ),
                        ex );
                }
            }
            else {
                Document oldXMLContent =
                        mSAXBuilder.build(
                                new InputSource(
                                        new StringReader(cs.getContent())));

                String mergedXMLString =
                        mConfigurationSetMerger.merge(
                                oldXMLContent.getRootElement(),
                                newXMLString);

                cs.setContent(mergedXMLString);
            }
        }
        catch (Exception ex ) {
            logFatal ( ex.toString(), ex  );

            throw new EJBException (
                collateErrorMessages (  "E4076",
                        new Object [] { entityType, entityID } ) );
        }

    }


    private boolean [] updateEntityAttributes ( Map formAttributeStrings,
                                                String entityType,
                                                String entityID )
            throws PDSException {

        try {
            boolean [] changedValues =
                    new boolean [PGSDefinitions.USER_CHANGE_ARRAY_SIZE];

            changedValues [PGSDefinitions.EXTENSION_CHANGED] = false;
            changedValues [PGSDefinitions.ALIASES_CHANGED] = false;
            changedValues [PGSDefinitions.LINE_INFO_CHANGED] = false;
            changedValues [PGSDefinitions.HIERARCHY_CHANGED] = false;

            logDebug( "inside generalStrings processing" );

            if ( entityType.equals( "user") ) {
                 changedValues =
                    mUserAdvocate.editUser(
                            entityID,
                            (String) formAttributeStrings.get("DisplayID"),
                            (String) formAttributeStrings.get("FirstName"),
                            (String) formAttributeStrings.get("LastName"),
                            (String) formAttributeStrings.get("Password"),
                            (String) formAttributeStrings.get("RefConfigSetID"),
                            (String) formAttributeStrings.get("UserGroupID"),
                            (String) formAttributeStrings.get("Aliases"),
                            (String) formAttributeStrings.get("Extension"));
            }
            else if ( entityType.equals( "device") ) {
                changedValues =
                    mDeviceAdvocate.editDevice(
                            entityID,
                            (String)formAttributeStrings.get("UserID" ),
                            (String)formAttributeStrings.get("DeviceGroupID"),
                            (String)formAttributeStrings.get("RefConfigSetID"),
                            (String)formAttributeStrings.get("CoreSoftwareDetailsID"),
                            (String)formAttributeStrings.get("Description"),
                            (String)formAttributeStrings.get("ShortName"),
                            (String)formAttributeStrings.get("SerialNumber"));
            }
            else if ( entityType.equals( "usergroup") ) {
                changedValues =
                    mUserGroupAdvocate.editUserGroup(
                            entityID,
                            (String) formAttributeStrings.get("Name"),
                            (String) formAttributeStrings.get("ParentID"),
                            (String) formAttributeStrings.get("RefConfigSetID"));
            }
            else {
                mDeviceGroupAdvocate.editDeviceGroup(
                        entityID,
                        (String)formAttributeStrings.get("ParentID"),
                        (String)formAttributeStrings.get("Name"),
                        (String)formAttributeStrings.get("RefConfigSetID"));
            } //else

            return changedValues;
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex  );

            throw new EJBException (
                collateErrorMessages (
                        "E4076",
                        new Object [] { entityType, entityID } ) );
        }

    }


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////

//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
