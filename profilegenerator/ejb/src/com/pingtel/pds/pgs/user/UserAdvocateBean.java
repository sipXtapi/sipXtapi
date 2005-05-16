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



package com.pingtel.pds.pgs.user;

import java.io.StringReader;
import java.rmi.RemoteException;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import javax.ejb.CreateException;
import javax.ejb.DuplicateKeyException;
import javax.ejb.EJBException;
import javax.ejb.FinderException;
import javax.ejb.RemoveException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import javax.naming.Context;
import javax.naming.InitialContext;

import org.jdom.CDATA;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.XMLOutputter;
import org.sipfoundry.sipxconfig.legacy.LegacyNotifyService;

import com.pingtel.pds.common.MD5Encoder;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.ProfileEncryptionKeyCalculator;
import com.pingtel.pds.common.XMLSupport;
import com.pingtel.pds.pgs.common.PGSDefinitions;
import com.pingtel.pds.pgs.common.RMIConnectionManager;
import com.pingtel.pds.pgs.common.ejb.BaseEJB;
import com.pingtel.pds.pgs.common.ejb.InternalToExternalIDTranslator;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.jobs.JobManager;
import com.pingtel.pds.pgs.jobs.JobManagerHome;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationHome;
import com.pingtel.pds.pgs.phone.Device;
import com.pingtel.pds.pgs.phone.DeviceAdvocate;
import com.pingtel.pds.pgs.phone.DeviceAdvocateHome;
import com.pingtel.pds.pgs.phone.DeviceHome;
import com.pingtel.pds.pgs.profile.ConfigurationSet;
import com.pingtel.pds.pgs.profile.ConfigurationSetHome;
import com.pingtel.pds.pgs.profile.ProfileTypeStringParser;
import com.pingtel.pds.pgs.profile.ProjectionHelper;
import com.pingtel.pds.pgs.profile.ProjectionHelperHome;
import com.pingtel.pds.pgs.profile.ProjectionInput;
import com.pingtel.pds.pgs.profile.RefProperty;
import com.pingtel.pds.pgs.profile.RefPropertyHome;
import com.pingtel.pds.pgs.profile.RenderProfile;
import com.pingtel.pds.pgs.profile.RenderProfileHome;


/**
 *  UserAdvocateBean is the EJ Bean implementation for the session facade for
 *  the User-based use cases in the PGS.
 *
 *@author     ibutcher
 */
public class UserAdvocateBean extends JDBCAwareEJB
         implements SessionBean, UserAdvocateBusiness {

//////////////////////////////////////////////////////////////////////////
// Constants
////
    private final static String STRING_PROF_TYPE_PHONE =
            String.valueOf( PDSDefinitions.PROF_TYPE_PHONE);

    private static final String EXT_PROF_ENCRYPTION_SOURCE =
            "externalProfileEncryptionSource";

//////////////////////////////////////////////////////////////////////////
// Attributes
////
    // Home references
    private UserHome mUserHome;
    private ConfigurationSetHome mConfigurationSetHome;
    private DeviceHome mDeviceHome;
    private RefPropertyHome mRefPropertyHome;
    private UserGroupHome mUserGroupHome;
    private OrganizationHome mOrganizationHome;

    // Stateless Session Bean references
    private DeviceAdvocate mDeviceAdvocateEJBObject;
    private ProjectionHelper mProjectionHelperEJBObject;
    private JobManager mJobManagerEJBObject;
    private RenderProfile mRenderProfileEJBObject;

    // Bean Context
    private SessionContext mCTX;

    // Misc.
    private SAXBuilder mSaxBuilder = new SAXBuilder();
    private XMLOutputter mXmlOutputter = new XMLOutputter();

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

        try {
            Context initial = new InitialContext();

            mUserHome = (UserHome) initial.lookup("User");
            mConfigurationSetHome = (ConfigurationSetHome)
                    initial.lookup("ConfigurationSet");
            mDeviceHome = (DeviceHome) initial.lookup("Device");
            mRefPropertyHome = (RefPropertyHome) initial.lookup("RefProperty");
            mUserGroupHome = (UserGroupHome) initial.lookup("UserGroup");

            ProjectionHelperHome projectionHelperHome = (ProjectionHelperHome)
                    initial.lookup("ProjectionHelper");

            DeviceAdvocateHome deviceAdvocateHome = (DeviceAdvocateHome)
                    initial.lookup("DeviceAdvocate");

            mRefPropertyHome = (RefPropertyHome) initial.lookup("RefProperty");
            mOrganizationHome =
                (OrganizationHome) initial.lookup("Organization");

            JobManagerHome jobManagerHome =
                    (JobManagerHome) initial.lookup ("JobManager");

            RenderProfileHome m_renderProfileHome =
                            (RenderProfileHome) initial.lookup("RenderProfile");

            mRenderProfileEJBObject = m_renderProfileHome.create();

            mDeviceAdvocateEJBObject = deviceAdvocateHome.create();
            mProjectionHelperEJBObject = projectionHelperHome.create();
            mJobManagerEJBObject = jobManagerHome.create();
        }
        catch (Exception ne) {
            logFatal( ne.toString(), ne );
            throw new EJBException(ne);
        }

        mCTX = ctx;
    }

    /**
     *  Description of the Method
     */
    public void unsetSessionContext() {
        mCTX = null;
    }

    /**
     *  createUser is the session facade for the "Create User" use case.
     *
     * @param firstName is the first name of the user to be created
     * @param lastName is the last name of the user to be created
     * @param password is the new users password - used for authentication
     * @param id is the unique user ID (user visible) for the new user.
     * @param refConfigSetID is the primary key of the Reference
     * Configuration Set that the User is assigned.
     * @param organizationID The internal ID of the organization that this user
     * belongs to
     * @param userGroupID the internal ID of the user group that this user
     * belongs to.
     * @param extension extension to be assigned to the user.
     * @return EJBObject of the new user.
     * @exception PDSException is thrown for non-fatal application exceptions
     */
    public User createUser( String organizationID,
                            String firstName,
                            String lastName,
                            String password,
                            String id,
                            String refConfigSetID,
                            String userGroupID,
                            String extension ) throws PDSException {

        User user = createUser( new Integer ( organizationID ),
                                firstName,
                                lastName,
                                password,
                                id,
                                new Integer (refConfigSetID ),
                                new Integer (userGroupID),
                                extension );

        return user;
    }


    /**
     * This version of createUser is the session facade for the
     * "Create User" use case which is used 'locally' i.e. between
     * EJ Beans (not the web UI).
     *
     * @param firstName is the first name of the user to be created
     * @param lastName is the last name of the user to be created
     * @param password is the new users password - used for authentication
     * @param id is the unique user ID (user visible) for the new user.
     * @param refConfigSetID is the primary key of the Reference
     * Configuration Set that the User is assigned.
     * @param organizationID The internal ID of the organization that this user
     * belongs to
     * @param userGroupID the internal ID of the user group that this user
     * belongs to.
     * @param extension extension to be assigned to the user.
     * @return EJBObject of the new user.
     * @exception PDSException is thrown for non-fatal application exceptions
     */
    public User createUser( Integer organizationID,
                            String firstName,
                            String lastName,
                            String password,
                            String id,
                            Integer refConfigSetID,
                            Integer userGroupID,
                            String extension ) throws PDSException {

        try {
            Organization org = null;

            try {
                org = mOrganizationHome.findByPrimaryKey( organizationID );
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "UC620",
                                                "E1018",
                                                new Object[]{ organizationID }),
                        ex);
            }

            String digestedPassword = UserHelper.digestPassword( id, org, password );

            User newUser =
                mUserHome.create(   organizationID,
                                    firstName,
                                    lastName,
                                    digestedPassword,
                                    id,
                                    refConfigSetID,
                                    userGroupID,
                                    extension );

            if(shouldUpdateExternalProfileEncryptionKey()){
                newUser.setProfileEncryptionKey(
                    ProfileEncryptionKeyCalculator.calculateProfileEncryptionKey(
                            id,
                            password));
            }

            assignSecurityRole( newUser, "END_USER", null );

            String sipPassword = password;
            if (!id.equals("superadmin") && !id.equals("SDS"))  {
                createInitialLine (newUser, sipPassword);
            }

            // new users, sip password and pin token will be same
            // user has to explicitly set sip password afterward
            setUsersPinToken(newUser, id, password);

            logTransaction (mCTX, "created user " + newUser.getExternalID());
            return newUser;
        }
        catch (PDSException ex) {
            mCTX.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC620"), ex);
        }
        catch (DuplicateKeyException de) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC620",
                    "E2010",
                    new Object[]{id}),
                    de);
        }
        catch (CreateException ce) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC620",
                    "E2011",
                    new Object[]{   id,
                                    firstName,
                                    lastName}),
                    ce);
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re);

            throw new EJBException(
                    collateErrorMessages("UC620",
                    "E4018",
                    new Object[]{id,
                    organizationID,
                    firstName,
                    lastName,
                    refConfigSetID}));
        }
    }    


    /**
     * deleteUser implements the "delete user" use case.  It deletes
     * the user entity and all the user's dependent information
     * (configuration set, roles).  It doesn't delete any devices
     * that the user currently owns.
     *
     * @param userID the PK of the user to be deleted
     * @throws PDSException is thrown for application level errors.
     */
    public void deleteUser(String userID) throws PDSException {

        User user = null;

        try {
            user = mUserHome.findByPrimaryKey( userID );
            deleteUser ( user );
        }
        catch ( FinderException ex ) {
            mCTX.setRollbackOnly();

            throw new PDSException (
                collateErrorMessages (  "UC625",
                                        "E1026",
                                        new Object [] {userID} ),
                ex );
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            throw new EJBException(
                    collateErrorMessages("UC625",
                    "E4019",
                    new String[]{ userID }));
        }
    }


    /**
     * NOTE: this is the 'local' implementation of the deleteUser,
     * the webUI should user the other version of this method.
     *
     * deleteUser implements the delete user use case.  It deletes
     * the user entity and all the user's dependent information
     * (configuration set, roles).  It doesn't delete any devices
     * that the user currently owns.
     *
     * @param user the EJBObject of the user to be deleted.
     * @throws PDSException is thrown for application level errors
     */
    public void deleteUser(User user) throws PDSException {
        

        try {
            String serviceUrl = BaseEJB.getPGSProperty("legacynotifyservice.rmi.url");

            LegacyNotifyService notifyService = 
                (LegacyNotifyService) RMIConnectionManager.getInstance().getConnection(serviceUrl);
            try { 
                Integer id = Integer.valueOf(user.getID());
                notifyService.onUserDelete(id);
            }
            catch(NumberFormatException e) {
                // ignore:
                // some users such as administrator, installer or SDS have non-numeric ids 
                // - we do not have to worry about them here
            }
            
            unassignDevices(user);
            logDebug("unassigned user's devices");
            deleteConfigSetsForUser(user);
            logDebug("deleted user's config sets");
            deleteUserRoles(user);
            logDebug("deleted user's security roles");

            String externalUserID = user.getExternalID();

            try {
                user.remove();
            }
            catch (RemoveException re) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages("UC625",
                        "E6010",
                        new String[]{ user.getExternalID()}),
                        re);
            }

            logTransaction (mCTX, "deleted user " +  externalUserID);
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            String external = null;

            try { external = user.getExternalID();} catch ( RemoteException rex ) {}

            throw new EJBException(
                    collateErrorMessages("UC625",
                    "E4019",
                    new String[]{ external }));
        }
    }


    /**
     * editUser is the session facade for the "Edit User" use case.  If null
     * values are supplied for parameters then the existing values are kept.
     * The exception being the 'id' parameter as this is used to identify the
     * user.
     *
     * @param id is the unique user ID for the user to be modified
     * @param displayID the ID that is displayed to users in the UI.
     * @param firstName the value to substitute for the existing
     * firstName if this is null then no change is made.
     * @param lastName the value to substitute for the existing
     * lastName if this is null then no change is made.
     * @param password the value to substitute for the existing
     * password if this is null then no change is made.
     * @param refConfigSetID the value to substitute for the existing
     * reference configuration set ID if this is null then no change is made.
     * @param userGroupID the user group ID that you wish to assign to the user.
     * @param aliases users may have >0 aliases which are used by the comm.
     * server if present.   These are supplied in a comma separated list.
     * @param extension a user may have a numeric extension
     * @exception PDSException is thrown for non-fatal application exceptions
     */
    public boolean [] editUser( String id,
                                String displayID,
                                String firstName,
                                String lastName,
                                String pintoken,
                                String refConfigSetID,
                                String userGroupID,
                                String aliases,
                                String extension ) throws PDSException {

        User thisUser = null;

        boolean [] changedValues = new boolean [ PGSDefinitions.USER_CHANGE_ARRAY_SIZE ];

        changedValues[PGSDefinitions.EXTENSION_CHANGED] = false;
        changedValues[PGSDefinitions.ALIASES_CHANGED] = false;
        changedValues[PGSDefinitions.LINE_INFO_CHANGED] = false;
        changedValues[PGSDefinitions.HIERARCHY_CHANGED] = false;

        try {
            boolean fixLine = false;
            boolean fixCredential = false;

            try {
                thisUser = mUserHome.findByPrimaryKey( id );
            }
            catch (FinderException fe) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages("UC630",
                        "E1026",
                        new String[]{id}),
                        fe);
            }


            if ( pintoken != null && pintoken.equalsIgnoreCase("e_m_p_t_y") ) {
                pintoken = "";
            }

            ////////////////////////////////////////////////////////////////////////////
            //
            // Only change the display ID _IF_ the displayID parameter is different
            // the the User's existing displayID _OR_ the password parameter has
            // changed.
            //
            ////////////////////////////////////////////////////////////////////////////
            if ( ( displayID != null && !displayID.equals( thisUser.getDisplayID() ) && pintoken != null ) ||
                    ( pintoken != null && !pintoken.equals( thisUser.getPassword() ) && displayID != null ) ) {

                if ( displayID != null && !displayID.equals( thisUser.getDisplayID() ) ) {

                    Collection existing = mUserHome.findByDisplayID( displayID );

                    if ( existing.size() > 0 ) {
                        String errorMessage =
                            collateErrorMessages(   "E3018",
                                                new Object[]{   thisUser.getDisplayID(),
                                                                displayID } );

                        mCTX.setRollbackOnly();

                        throw new PDSException( errorMessage );
                    }
                }

                thisUser.setDisplayID( displayID );
                // NOT the pintoken is notpart of the XMLob in DB
                setUsersPinToken(thisUser, displayID, pintoken);

                // Fix primary line
                changedValues[PGSDefinitions.LINE_INFO_CHANGED] = true;
            }

            String existingFirstName = thisUser.getFirstName();

            if (( firstName != null &&
                    (existingFirstName == null || !firstName.equals(existingFirstName ))) ||
                  firstName == null && existingFirstName != null ) {

                thisUser.setFirstName( firstName );
                fixLine = true;
                changedValues [ PGSDefinitions.LINE_INFO_CHANGED ] = true;
            }

            String existingLastName = thisUser.getLastName();

            if ((lastName != null &&
                    (existingLastName == null || !lastName.equals(existingLastName))) ||
                lastName == null && existingLastName != null ) {

                thisUser.setLastName(lastName);
                fixLine = true;
                changedValues [ PGSDefinitions.LINE_INFO_CHANGED ] = true;
            }


            Integer existingRCSID = thisUser.getRefConfigSetID();

            if (refConfigSetID != null &&
                    (existingRCSID == null || !refConfigSetID.equals(existingRCSID.toString()))){

                thisUser.setRefConfigSetID( Integer.valueOf( refConfigSetID ) );
            }


            Integer existingUGID = thisUser.getUserGroupID();

            if (userGroupID != null &&
                    (existingUGID == null || !userGroupID.equals(existingUGID.toString()))){

                thisUser.setUserGroupID( new Integer( userGroupID ) );
                changedValues [ PGSDefinitions.HIERARCHY_CHANGED ] = true;
            }

            String existingExtension = thisUser.getExtension();

            if ((extension == null && existingExtension != null) ||
                       (extension != null && !extension.equals(existingExtension))){

                thisUser.setExtension( extension );
                fixLine = true;
                changedValues [ PGSDefinitions.EXTENSION_CHANGED ] = true;
            }

            // this validation was moved here from user bean as we had a chicken and
            // egg problem putting it in setExtension and setDisplayID
            if (thisUser.getExtension() != null &&
                    thisUser.getExtension().equals(thisUser.getDisplayID())){

                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC630",
                                            "E3023",
                                            null ) );
            }

            String existingAliases = thisUser.getAliases();

            if (!( existingAliases.equals("") && aliases == null) &&
                    !existingAliases.equals(aliases)){

                thisUser.setAliases( aliases );
                changedValues [ PGSDefinitions.ALIASES_CHANGED ] = true;
            }

            if ( fixLine ) {
                fixPrimaryLine( thisUser, fixCredential, null );
            }

            return changedValues;
        }
        catch (PDSException e) {
            mCTX.setRollbackOnly();

            throw new PDSException( collateErrorMessages("UC630"), e);
        }
        catch ( Exception re) {
            logFatal( re.toString(), re );

            String userExternal = null;

            try { userExternal = thisUser.getExternalID();} catch ( RemoteException ex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC630",
                                        "E4020",
                                        new String[]{ userExternal == null ? id : userExternal } ) );
        }
    }


    /**
     * changes the given Users sip passtoken. (local)
     *
     * @param user User whose password is to be set
     * @param displayID the display ID or externally visible ID for the User
     * @param password the plain-text password to be used
     * @throws PDSException is thrown for application level errors
     */
    public void setUsersSipPassword(User user, String displayID, String password)
            throws PDSException {

        try {
            if ( password != null && password.equalsIgnoreCase("e_m_p_t_y") ) {
                password = "";
            }

            Organization org = null;
            Integer organizationID = user.getOrganizationID();

            try {
                org = mOrganizationHome.findByPrimaryKey( organizationID );
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "UC630",
                                                "E1018",
                                                new Object[]{ organizationID }),
                        ex);
            }

            Collection roles = getSecurityRoles( user.getID().toString());
            if ( !roles.contains( "SUPER") && !roles.contains ("SDS") ){
                fixPrimaryLine( user, true, password );
            }

            if(shouldUpdateExternalProfileEncryptionKey()){
                user.setProfileEncryptionKey(
                        ProfileEncryptionKeyCalculator.calculateProfileEncryptionKey(
                                displayID,
                                password));
            }
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            String userExternal = null;

            try { userExternal = user.getExternalID();} catch ( RemoteException ex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC630",
                                        "E4020",
                                        new Object []{ userExternal } ) );
        }
    }


    /**
     * changes the given Users password.
     *
     * @param id PK of the User whose password is to be set
     * @param displayID the display ID or externally visible ID for the User
     * @param password the plain-text password to be used
     * @throws PDSException is thrown for application level errors
     */
    public void setUsersSipPassword(String id, String displayID, String password)
            throws PDSException {

        User thisUser = null;

        try {
            thisUser = mUserHome.findByPrimaryKey( id );

            setUsersSipPassword ( thisUser, displayID, password );
        }
        catch (FinderException fe) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC630",
                    "E1026",
                    new String[]{id}),
                    fe);
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            String userExternal = null;

            try { userExternal = thisUser.getExternalID();} catch ( RemoteException ex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC630",
                                        "E4020",
                                        new String[]{   userExternal == null ?
                                                            id :
                                                            userExternal } ) );
        }
    }
    
    public void setUsersPinToken(String id, String displayID, String password) throws PDSException,
            RemoteException {
        User thisUser = null;

        try {
            thisUser = mUserHome.findByPrimaryKey( id );

            setUsersPinToken ( thisUser, displayID, password );
        }
        catch (FinderException fe) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC630",
                    "E1026",
                    new String[]{id}),
                    fe);
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            String userExternal = null;

            try { userExternal = thisUser.getExternalID();} catch ( RemoteException ex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC630",
                                        "E4020",
                                        new String[]{   userExternal == null ?
                                                            id :
                                                            userExternal } ) );
        }        
    }

    public void setUsersPinToken(User user, String displayID, String password) throws PDSException,
            RemoteException {

        try {
            if ( password != null && password.equalsIgnoreCase("e_m_p_t_y") ) {
                password = "";
            }

            Organization org = null;
            Integer organizationID = user.getOrganizationID();

            try {
                org = mOrganizationHome.findByPrimaryKey( organizationID );
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "UC630",
                                                "E1018",
                                                new Object[]{ organizationID }),
                        ex);
            }

            String digestedPassword = UserHelper.digestPassword( displayID, org, password );
            user.setPassword( digestedPassword );
        }
        catch (RemoteException re) {
            logFatal( re.toString(), re );

            String userExternal = null;

            try { userExternal = user.getExternalID();} catch ( RemoteException ex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC630",
                                        "E4020",
                                        new Object []{ userExternal } ) );
        }
    }
    
    
    /**
     * Implementation of the bean method
     * @see com.pingtel.pds.pgs.user.UserAdvocateBusiness#fixDnsDomain(com.pingtel.pds.pgs.organization.Organization)
     */
    public void fixDnsDomain(Organization organization, String oldDomainName) throws PDSException, RemoteException {
        try {
            Integer orgID = organization.getID();
            Collection users = mUserHome.findByOrganizationID(orgID);
            for (Iterator i = users.iterator(); i.hasNext();) {
                User user = (User) i.next();
                // false == do not touch credentials
                fixPrimaryLine(user,false,null);
                fixUserLines(user,oldDomainName);
            }
        } catch (FinderException e) {
            throw new PDSException("Attempt to fix DNS name for users.", e);
        }
    }

   /**
    * fixPrimaryLine modifies previously generated primary line property settings.   This
    * now includes the Pingtel 'PRIMARY_LINE' settings and the Cisco 79xx 'line1' setting.
    * The undigested password is needed as we can't determine the plain-text password that
    * the user originally entered from the digested value that we store.  (local)
    *
    * @param user User whose line definitions need modifying
    * @param fixCredential do we need to reset the credential values for the Pingtel primary
    * line setting
    * @param undigestedPassword see method description.  Plain text password
    * @throws PDSException is thrown for application level errors
    */
   public void fixPrimaryLine ( User user, boolean fixCredential, String undigestedPassword )
        throws PDSException {

        String externalID = null;

        try {
            externalID = user.getExternalID();

            Organization organization =
                    mOrganizationHome.findByPrimaryKey( user.getOrganizationID() );
            final String realm = organization.getAuthenticationRealm();
            final String displayID = user.getDisplayID();

            ConfigurationSet cs = getUserConfigurationSet(user);
            
            if( null == cs )
            {
                logError("No configuration sets for user: " + displayID);
                return;
            }
            
            String xmlContent = cs.getContent();
            Document doc = mSaxBuilder.build(new StringReader(xmlContent));

            Element profile = doc.getRootElement();

            RefProperty primaryLineRP = getRefProperty ( "xp_10001" );
            String primaryLineRPID = primaryLineRP.getID().toString();

            RefProperty defaultLineRP = getRefProperty ( "xp_2028" );
            String defaultLineRPID = defaultLineRP.getID().toString();

            RefProperty cisco79xxLineRP = getRefProperty ( "cs_1020" );
            String cisco79xxRPID = cisco79xxLineRP.getID().toString();
            
            UserHelper helper = new UserHelper( user );
            String passtoken = null;
            if( undigestedPassword != null )
            {
                passtoken = helper.digestPassword(organization, undigestedPassword);                
            }
            
            String oldLineURL = null;
            Collection userLines = profile.getChildren( );
            for ( Iterator lineI = userLines.iterator(); lineI.hasNext(); ) {
                Element setting = (Element) lineI.next();
                String rpID = setting.getAttributeValue( "ref_property_id" );

                if ( rpID.equals( primaryLineRPID )  ) {
                    Element url = setting.getChild( "PRIMARY_LINE" ).getChild( "URL" );
                    List l = url.getContent();
                    for ( Iterator iList = l.iterator(); iList.hasNext(); ) {
                        CDATA existing = (CDATA) iList.next();
                        oldLineURL = existing.getText();
                    }
                    helper.fixUserPrimaryLine(setting, organization, realm, fixCredential, passtoken);                    
                } // if
                else if ( rpID.equals( defaultLineRPID ) ) {
                    Element holder = setting.getChild( "USER_DEFAULT_OUTBOUND_LINE" );
                    String oldOutboundLineURL = null;

                    List list = holder.getContent();
                    for ( Iterator iList = XMLSupport.detachableIterator(list.iterator()); iList.hasNext(); ) {
                        CDATA existing = (CDATA) iList.next();
                        oldOutboundLineURL = existing.getText();

                        if ( oldOutboundLineURL.equals( oldLineURL )) {
                            holder.removeContent( existing );
                            holder.addContent( new CDATA ( user.calculatePrimaryLineURL() ) );
                        }
                    }
                } // else
                else if ( rpID.equals( cisco79xxRPID) ) {
                    Element holder = setting.getChild( "container" );

                    if (fixCredential) {
                        Element password = holder.getChild ( "line1_password" );
                        removeCDATAFromElement( password );
                        password.addContent( new CDATA ( undigestedPassword ) );
                    }

                    Element displayName = holder.getChild ( "line1_displayname" );
                    removeCDATAFromElement( displayName );
                    displayName.addContent(
                            new CDATA ( new String ("\"" +user.getFirstName() + " " +
                            user.getLastName() + "\"").trim() ) );

                    Element authName = holder.getChild ( "line1_authname" );
                    removeCDATAFromElement( authName );
                    authName.addContent( new CDATA ( displayID ) );

                    Element identity = holder.getChild ( "line1_name" );
                    removeCDATAFromElement( identity );
                    identity.addContent( new CDATA ( displayID ) );

                    Element shortName = holder.getChild ( "line1_shortname" );
                    removeCDATAFromElement( shortName );
                    shortName.addContent( new CDATA ( displayID + "_line" ) );
                }
            } // for all elements in config set

            cs.setContent( mXmlOutputter.outputString ( profile ) );
        }
        catch ( JDOMException ex ) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages(   "E7008",
                                            new Object[]{ externalID }),
                    ex);

        }
        catch (  Exception ex ) {
            logFatal ( ex.toString(), ex );
            throw new EJBException (
                collateErrorMessages (  "E4068",
                                        new Object [] { externalID } ) );

        }
    }
   
   
    /**
     * Modify user's configuration set. Updates all references to user lines that 
     * had old domain name to current (organization) domain name.
     * @param user
     * @param oldDomainName
     * @throws PDSException
     */
    private void fixUserLines(User user, String oldDomainName) throws PDSException {
        String externalID = null;

        try {
            externalID = user.getExternalID();

            Integer organizationID = user.getOrganizationID();
            Organization organization = mOrganizationHome.findByPrimaryKey(organizationID);
            final String domainName = organization.getDNSDomain();

            ConfigurationSet cs = getUserConfigurationSet(user);

            if (null == cs) {
                return;
            }

            String xmlContent = cs.getContent();
            Document doc = mSaxBuilder.build(new StringReader(xmlContent));

            Element profile = doc.getRootElement();

            RefProperty userLineRP = getRefProperty("xp_2029");
            String userLineRPID = userLineRP.getID().toString();

            UserHelper helper = new UserHelper(user);

            Collection userLines = profile.getChildren();
            for (Iterator i = userLines.iterator(); i.hasNext();) {
                Element line = (Element) i.next();
                String rpID = line.getAttributeValue("ref_property_id");

                if (userLineRPID.equals(rpID)) {
                    helper.fixUserLineUrl(line, domainName, oldDomainName);
                }
            }

            cs.setContent(mXmlOutputter.outputString(profile));
        } catch (JDOMException ex) {
            mCTX.setRollbackOnly();

            throw new PDSException(collateErrorMessages("E7008", new Object[] { externalID }), ex);

        } catch (Exception ex) {
            logFatal(ex.toString(), ex);
            throw new EJBException(collateErrorMessages("E4068", new Object[] { externalID }));
        }
    }
  
   

	/**
     * Find the first configuration set for the use
     * 
     * @param user
     * @return first configuration set matching user id (or null if none found)
     * @throws FinderException
     * @throws RemoteException
     */
	private ConfigurationSet getUserConfigurationSet(User user) 
	    throws FinderException, RemoteException {
	    
	    ConfigurationSet cs = null;
	
	    Collection userCSC = mConfigurationSetHome.findByUserID( user.getID () );
	    for ( Iterator iCSC = userCSC.iterator(); iCSC.hasNext(); )
	    {
	        cs = (ConfigurationSet) iCSC.next();
	    }
	    return cs;
	}


    /**
     * copies a User - to be continued...
     *
     * @param sourceUserID
     * @return
     * @throws PDSException
     * @deprecated need to re-implement this with an eye to which settings
     * get copied.
     */
    public User copyUser (String sourceUserID) throws PDSException {

        User source = null;
        User copy = null;

        try {
            try {
                source = mUserHome.findByPrimaryKey( sourceUserID );
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC627",
                                            "E1026",
                                            new Object[]{ sourceUserID } ),
                    ex);
            }

           try {
                copy = createUser ( source.getOrganizationID().toString(),
                                    source.getFirstName(),
                                    source.getLastName(),
                                    null,
                                    "Copy of " + source.getDisplayID(),
                                    source.getRefConfigSetID().toString(),
                                    source.getUserGroupID().toString(),
                                    source.getExtension() );
            }
            catch ( PDSException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException( collateErrorMessages( "UC627" ), ex );
            }


            /*ConfigurationSet sourceCS =  null;

            try {
                Collection c = m_configSetHome.findByUserID( sourceUserID );
                for ( Iterator i = c.iterator(); i.hasNext(); ) {
                    sourceCS = (ConfigurationSet) i.next();

                    m_configSetHome.create( source.getRefConfigSetID(),
                                            sourceCS.getProfileType(),
                                            copy,
                                            sourceCS.getContent() );
                }
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC627",
                                            "E1024",
                                            new Object[]{ source.getExternalID() } ),
                    ex);
            }
            catch ( CreateException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC627",
                                            "E2018",
                                            new Object[]{   source.getExternalID(),
                                                            source.getRefConfigSetID(),
                                                            String.valueOf(sourceCS.getProfileType()),
                                                            sourceCS.getContent() } ),
                    ex);
            }*/

            ///////////////////////////////////////////////////////////////////////
            // Copy the soure user's security roles
            ///////////////////////////////////////////////////////////////////////
            try {
                Collection sourceRoles = getSecurityRoles ( source.getID() );
                for ( Iterator i = sourceRoles.iterator(); i.hasNext(); ) {
                    this.assignSecurityRole( copy.getID(), (String) i.next(), null );
                }
            }
            catch ( PDSException ex ) {
                this.mCTX.setRollbackOnly();

                throw new PDSException( collateErrorMessages( "UC627" ), ex );
            }
        }
        catch ( RemoteException ex ) {
            logFatal( ex.toString(), ex );

            String userExternal = null;

            try { userExternal = source.getExternalID();} catch ( RemoteException rex ) {}

            throw new EJBException(
                    collateErrorMessages(   "UC627",
                                            "E4040",
                                            new String[]{   userExternal == null ?
                                                                sourceUserID :
                                                                userExternal } ) );
        }

        return copy;
    }

    /**
     * returns a collection of Strings, each of which contains one of the
     * security roles for the given User.
     *
     * @param userID PK for the User whose roles are being queried
     * @return collection of Strings representing the User's security roles.
     * @throws PDSException is thrown for application level errors.
     */
    public Collection getSecurityRoles(String userID) throws PDSException {

        ArrayList results = new ArrayList();

        try {
            ArrayList roles =
                executePreparedQuery(   "SELECT roles_name " +
                                        "FROM   USER_ROLES " +
                                        "WHERE  usrs_id = ?",
                                        new Object [] { userID },
                                        1,
                                        1000 );

            for ( Iterator i = roles.iterator(); i.hasNext(); ) {
                ArrayList row = (ArrayList) i.next();
                String role = (String) row.get(0);
                results.add( role );
            }

        }
        catch ( SQLException ex ) {
            this.mCTX.setRollbackOnly();

            throw new PDSException (
                collateErrorMessages (  "E1039",
                                        new Object [] { userID } ) );
        }

        return results;

    }


    /**
     * assignSecurityRole gives the given User a new security role.
     *
     * @param userID userID PK for the User you are given the role to.
     * @param role security role name.  Currently 'SUPER', 'ADMIN' or
     * 'END_USER'.
     * @param password
     * @throws PDSException is thrown for application errors.
     */
    public void assignSecurityRole( String userID,
                                    String role,
                                    String password ) throws PDSException {

        User user = null;

        try {
            user = mUserHome.findByPrimaryKey(userID);
            assignSecurityRole ( user, role, password  );
        }
        catch (FinderException ex) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC635",
                                        "E1026",
                                        new Object[]{userID}),
                ex);
        }
        catch (RemoteException ex) {
            mCTX.setRollbackOnly();

            String external = null;

            try { external = user.getExternalID();} catch ( RemoteException rex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC635",
                                        "E4021",
                                        new Object[]{   role,
                                                        external }));
        }

    }

    /**
     * assignSecurityRole gives the given User a new security role.
     * (local)
     *
     * @param user User you are giving the role to.
     * @param role security role name.  Currently 'SUPER', 'ADMIN' or
     * 'END_USER'.
     * @param password
     * @throws PDSException is thrown for application errors.
     */
    public void assignSecurityRole( User user,
                                    String role,
                                    String password ) throws PDSException {

        try {

            try {
                ArrayList result =
                        this.executePreparedQuery("SELECT USRS_ID FROM USER_ROLES " +
                        "WHERE USRS_ID = ? " +
                        "AND ROLES_NAME = ?",
                        new Object[]{ user.getID(), role},
                        1,
                        1);

                if (result.size() == 0) {

                        executePreparedUpdate(  "INSERT INTO USER_ROLES ( USRS_ID, ROLES_NAME ) " +
                                                "VALUES ( ?,? )",
                                                new Object[]{user.getID(), role});
                }
            }
            catch (SQLException ex) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC635",
                                            "E5005",
                                            new Object[]{   role,
                                                            user.getExternalID()}),
                    ex);
            }

            logTransaction ( mCTX, "assigned security role " + role +
                " to user " + user.getExternalID() );
        }
        catch (RemoteException ex) {
            mCTX.setRollbackOnly();

            String external = null;

            try { external = user.getExternalID();} catch ( RemoteException rex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC635",
                                        "E4021",
                                        new Object[]{   role,
                                                        external }));
        }
    }


    /**
     * unassignSecurityRole removes a PDS security role (currently
     * 'SUPER','ADMIN' and 'END_USER' are defined) from a particular user.
     *
     * @param userID PK of the User
     * @param role the role that you are unassigning.
     * @exception PDSException  is thrown for application level errors.
     * @see #assignSecurityRole
     */
    public void unassignSecurityRole(   String userID,
                                        String role) throws PDSException {

        User user = null;

        try {
            // check user exists
            user = mUserHome.findByPrimaryKey(userID);

            try {
                ArrayList result =
                        this.executePreparedQuery("SELECT USRS_ID FROM USER_ROLES " +
                        "WHERE USRS_ID = ? " +
                        "AND ROLES_NAME = ?",
                        new Object[]{userID, role},
                        1,
                        1);

                if (result.size() != 0) {

                    executePreparedUpdate("DELETE FROM USER_ROLES  " +
                            "WHERE USRS_ID = ? " +
                            "AND ROLES_NAME = ?",
                            new Object[]{userID, role});
                }
            }
            catch (SQLException ex) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC636",
                                            "E5006",
                                            new Object[]{role,
                                            user.getExternalID()}),
                    ex);
            }

            String externalUserID =
                InternalToExternalIDTranslator.getInstance().translate( mUserHome,
                                                                        userID );

            logTransaction ( mCTX, "unassigned security role " + role +
                " to user " + externalUserID );
        }
        catch (FinderException ex) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC636",
                    "E1026",
                    new Object[]{userID}),
                    ex);
        }
        catch (RemoteException ex) {
            mCTX.setRollbackOnly();

            String userExternal = null;

            try { userExternal = user.getExternalID();} catch ( RemoteException rex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC636",
                                        "E4022",
                                        new Object[]{   role,
                                                        userExternal == null ?
                                                            userID :
                                                            userExternal } ) );
        }
    }


    /**
     * assignDevice is used to make the given User the owner of the given Device.
     * Amongst other things this means that when projections are done for either
     * the User or Device the user-profile for the Device will pertain to this
     * owning User.
     *
     * @param deviceID PK of the Device that is to be owned by the User
     * @param userID PK of the User who is to own the given Device.
     * @throws PDSException is thrown for application errors.
     */
    public void assignDevice( String deviceID, String userID)
        throws PDSException {

        Device lp = null;
        User user = null;

        try {
            try {
                lp = mDeviceHome.findByPrimaryKey( new Integer( deviceID ) );
            }
            catch (FinderException ex) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC600",
                                            "E1001",
                                            new Object[]{ deviceID }),
                    ex);
            }

            try {
                user = mUserHome.findByPrimaryKey( userID );
            }
            catch (FinderException ex) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC600",
                                            "E1026",
                                            new Object[]{userID}),
                    ex);
            }

            if ( !( user.getOrganizationID().equals( lp.getOrganizationID() ) ) ){
                mCTX.setRollbackOnly();

                throw new PDSException( collateErrorMessages(   "UC600",
                                                                "E3012",
                                                                null ) );
            }

            lp.setUserID(userID);

            String externalUserID =
                InternalToExternalIDTranslator.getInstance().translate( mUserHome,
                                                                        userID );

            String externalDeviceID =
                InternalToExternalIDTranslator.getInstance().translate( mDeviceHome,
                                                                        Integer.valueOf( deviceID ) );

            logTransaction ( mCTX, "assigned device " + externalDeviceID +
                " to user " + externalUserID );
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            String deviceExternal = null;

            try { deviceExternal = lp.getExternalID();} catch ( RemoteException rex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC600",
                                        "E4023",
                                        new Object[]{   deviceExternal == null ?
                                                            deviceID :
                                                            deviceExternal,
                                        userID}));
        }
    }

    /**
     * unassigns the given device from its current onwner.   It deletes the User and
     * Application profiles (for pingtel devices) from the ProfileWriter/CDS.
     *
     * @param deviceID PK of the Device you want to unassign
     * @throws PDSException is thrown for application errors
     */
    public void unassignDevice(String deviceID ) throws PDSException {

        Device device = null;

        try {
            device = mDeviceHome.findByPrimaryKey( Integer.valueOf( deviceID ) );

            String externalDeviceID =
                InternalToExternalIDTranslator.getInstance().translate(
                        mDeviceHome,
                        Integer.valueOf( deviceID ) );

            mRenderProfileEJBObject.deleteProfile( device, PDSDefinitions.PROF_TYPE_USER );
            logDebug ( "Deleted USER profile for device: " + externalDeviceID );

            String externalUserID =
                InternalToExternalIDTranslator.getInstance().translate( mUserHome,
                                                                        device.getUserID() );

            device.setUserID(null);

            logTransaction ( mCTX, "unassigned device " + externalDeviceID +
                " from user " + externalUserID );
        }
        catch (FinderException ex) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC602",
                    "E1001",
                    new Object[]{ deviceID }),
                    ex);
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            String deviceExternal = null;

            try { deviceExternal = device.getExternalID();} catch ( RemoteException rex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC602",
                                        "E4024",
                                        new Object[]{   deviceExternal == null ?
                                                            deviceID :
                                                            deviceExternal } ) );
        }
    }

    /**
     * restartDevices sends a signal to reset is sent to all of the Devices
     * owned by the given User.
     *
     * @param userID PK of the User whose Devices you want to reset.
     * @throws PDSException is thrown for application errors.
     */
    public void restartDevices ( String userID ) throws PDSException {

        String userExternal = null;

        try {
            User user = null;

            try {
                user = mUserHome.findByPrimaryKey( userID );
                userExternal = user.getExternalID();
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "E1026",
                                            new Object[]{ userID } ),
                    ex);
            }

            Collection devices = mDeviceHome.findByUserID( userID );

            for ( Iterator iDev = devices.iterator(); iDev.hasNext(); ) {
                Device device = (Device) iDev.next();
                mDeviceAdvocateEJBObject.restartDevice( device.getID().toString() );
            }

            String externalUserID =
                InternalToExternalIDTranslator.getInstance().translate( mUserHome,
                                                                        userID );

            logTransaction ( mCTX, "restared user " + externalUserID +
                " devices "  );
        }
        catch (PDSException ex) {
            throw new PDSException ( collateErrorMessages ( "UC466" ), ex );
        }
        catch (Exception ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                    collateErrorMessages(   "UC466",
                                            "E4072",
                                            new Object[]{ userExternal } ) );
        }
    }


    /**
     * generateProfiles create the profiles (Device configuration files) for
     * each of the Devices that the given User owns.
     *
     * @param userID PK of the User whose profiles you want to (re)create.
     * @param profileTypes a comma-separated list of profile types that
     * should be created for each of the User's Devices
     * @param projAlg fully qualified class name for the projection Class
     * to use.  If none is supplied then the default class is used for the
     * Device's DeviceType.
     * @throws PDSException is thrown for application errors
     */
    public void generateProfiles(   String userID,
                                    String profileTypes,
                                    String projAlg) throws PDSException {
        User user = null;

        try {
            user = mUserHome.findByPrimaryKey(userID);
        }
        catch (FinderException ex) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC670",
                                        "E1026",
                                        new Object[]{ userID }),
                ex);
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC670",
                                        "E4035",
                                        new Object[]{   userID }));
        }

        // This version of generate profiles is always used
        // for separate projections; The other Beans which
        // piggy back this functionality use the non-string
        // version of generateProfiles
        generateProfiles ( user, profileTypes, projAlg, true );
    }

    /**
     * generateUserProfiles generates the USER and APPLICATION profiles only
     * for a given Device.   The only place this code should be called from
     * is DeviceAdvocateBean.
     *
     * @param user EJBObject of User whose profiles you want to generate.
     * @param device EJBObject of the Device whose users profiles you want
     * to generate.
     * @param profileTypes comma-separated list of profile types.  Can only be
     * User and Application Ref
     * @param projectionAlg
     * @throws PDSException is thrown for non-critical errors.
     * @see PDSDefinitions#PROF_TYPE_APPLICATION_REF
     * PDSDefinitions#PROF_TYPE_USER
     */
    public void generateUserProfiles(   User user,
                                        Device device,
                                        String profileTypes,
                                        String projectionAlg ) throws PDSException {

        try {
            boolean[] profTypesToCreate = parseProfileTypes(profileTypes);

            ArrayList projectionInputs = new ArrayList();

            if (profTypesToCreate[PDSDefinitions.PROF_TYPE_USER]) {
                projectionInputs.addAll(
                        getProjectionInputs(user,
                                PDSDefinitions.PROF_TYPE_USER));

                logDebug ( "creating USER profile for device " +
                        device.getExternalID() );

                mProjectionHelperEJBObject.projectAndPersist(
                        projectionAlg,
                        device,
                        PDSDefinitions.PROF_TYPE_USER,
                        projectionInputs);
            }
        }
        catch (RemoteException e) {
            logFatal (e.getMessage(), e);
            throw new EJBException (e.getMessage());
        }

    }


    /**
     * generateProfiles create the profiles (Device configuration files) for
     * each of the Devices that the given User owns.
     *
     * @param user User whose profiles you want to (re)create.
     * @param profileTypes a comma-separated list of profile types that
     * should be created for each of the User's Devices
     * @param projAlg fully qualified class name for the projection Class
     * to use.  If none is supplied then the default class is used for the
     * Device's DeviceType.
     * @throws PDSException is thrown for application errors
     */
    public void generateProfiles(   User user,
                                    String profileTypes,
                                    String projAlg,
                                    boolean separateProjection) throws PDSException {


        int jobID = -1;
        try {
            boolean[] profTypesToCreate = parseProfileTypes(profileTypes);

            if ( separateProjection ) {
                jobID = createStartProjectionJobRecord(user, profTypesToCreate);
            }

            if (profTypesToCreate[PDSDefinitions.PROF_TYPE_USER] ) {

                generateUserCentricProfiles(
                                    user,
                                    projAlg,
                                    profTypesToCreate[PDSDefinitions.PROF_TYPE_USER]);

            }
            if (profTypesToCreate[PDSDefinitions.PROF_TYPE_PHONE]) {
                generatePhoneProfile(user, projAlg);
            }

            if ( separateProjection ) {
                mJobManagerEJBObject.updateJobProgress(
                        jobID,
                        "projected 1 of 1 users.");

                mJobManagerEJBObject.updateJobStatus(
                        jobID,
                        JobManager.COMPLETE,
                        null );
            }
        }
        catch (RemoteException e) {
            logFatal(e.toString(), e);
            // TODO - updating status should not be handled
            try {
                mJobManagerEJBObject.updateJobStatus(
                        jobID,
                        JobManager.FAILED ,
                        e.toString());

            } catch ( Exception e1) {e1.printStackTrace();}

            throw new EJBException ( e.toString() );
        }
    }

    private int createStartProjectionJobRecord( User user,
                                                boolean[] profTypesToCreate)
            throws PDSException, RemoteException {

        int jobID;
        StringBuffer jobDetails = new StringBuffer ();
        jobDetails.append( "Projection for user: ");
        jobDetails.append( user.getExternalID());
        jobDetails.append ( " profile types: ");

        if ( profTypesToCreate[ PDSDefinitions.PROF_TYPE_USER ] ) {
            jobDetails.append ( "user ");
        }
        if ( profTypesToCreate[ PDSDefinitions.PROF_TYPE_PHONE ] ) {
            jobDetails.append ( "device ");
        }

        jobID =
            mJobManagerEJBObject.createJob(    JobManager.PROJECTION,
                                                jobDetails.toString(),
                                                JobManager.STARTED );

        mJobManagerEJBObject.updateJobProgress( jobID, "projected 0 of 1 users.");
        return jobID;
    }

    private boolean[] parseProfileTypes(String profileTypes)
            throws PDSException {

        boolean[] profTypesToCreate = null;
        try {
            profTypesToCreate = ProfileTypeStringParser.parse(profileTypes);
        }
        catch (PDSException ex) {
            mCTX.setRollbackOnly();

            throw new PDSException( collateErrorMessages( "UC670" ),  ex);
        }
        return profTypesToCreate;
    }

    /**
     * Returns a digested value of the given password for the given User.
     *
     * @param displayID the user-visible ID for the User whose password is to be
     * calculated
     * @param realm - it used to be DNS domain 
     * @param password plain-text password
     * @return digested password.
     * @see MD5Encoder.digestPassword
     * @deprecated 
     */
    public String digestUsersPassword ( String displayID,
                                        String realm,
                                        String password ) {
        return MD5Encoder.digestPassword(displayID,realm,password);
    }

    
    
    
    
    
    
    
    
    
    
    
    


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    /////////////////////////////////////////////////////////////////////////
    //
    // Delete's user configuration set(s).
    //
    /////////////////////////////////////////////////////////////////////////
    private void deleteConfigSetsForUser(User user) throws
        PDSException, RemoteException {

        ConfigurationSet cs = null;

        try {
            Collection csUsers = mConfigurationSetHome.findByUserID(user.getID());
            for (Iterator i = csUsers.iterator(); i.hasNext(); ) {
                cs = (ConfigurationSet) i.next();
                cs.remove();
            }
        }
        catch (FinderException fe) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC625",
                                        "E1024",
                                        new Object[]{user.getExternalID()}),
                fe);
        }
        catch (RemoveException re) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC625",
                                        "E6013",
                                        new Object[]{user.getExternalID()}),
                re);
        }
    }


    /////////////////////////////////////////////////////////////////////////
    //
    // Removes all the users devices.
    //
    /////////////////////////////////////////////////////////////////////////
    private void unassignDevices( User user )
        throws PDSException, RemoteException {

        try {
            Collection ptUsers = mDeviceHome.findByUserID( user.getID() );

            for (Iterator i = ptUsers.iterator(); i.hasNext(); ) {
                Device device = (Device) i.next();
                unassignDevice( device.getID().toString() );
            }
        }
        catch (FinderException fe) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC625",
                                        "E1025",
                                        new Object[]{ user.getExternalID() }),
                fe);
        }
    }



    private void deleteUserRoles( User user)
            throws PDSException, RemoteException {

        try {
            executePreparedUpdate(  "DELETE FROM USER_ROLES WHERE USRS_ID = ?",
                                    new Object[]{ user.getID() });
        } catch (SQLException ex) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC625",
                    "E5004",
                    new Object[]{ user.getExternalID() }),
                    ex);
        }
    }


    private void removeCDATAFromElement(Element password) {
        List list = password.getContent();
        // :FIXME: should be replaced by list.clear()?
        for ( Iterator iList = list.iterator(); iList.hasNext(); ) {
            CDATA existing = (CDATA) iList.next();
            iList.remove();
        }
    }


    private void createInitialLine ( User user, String sipPassword )
        throws PDSException {

        try {
            Organization org = null;
            Integer organizationID = user.getOrganizationID();

            try {
                org = mOrganizationHome.findByPrimaryKey( organizationID );
                logDebug ( "found organization: " + org.getExternalID() );
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "E1018",
                                                new Object[]{ organizationID }),
                        ex);
            }
            
            RefProperty rpXp = getRefProperty ( "xp_10001" );
            logDebug ( "found PRIMARY_LINE ref property" );
            RefProperty rpCs = getRefProperty ( "cs_1020" ); // line1
            logDebug ( "found PRIMARY_LINE ref property" );
            RefProperty defaultLine = getRefProperty ( "xp_2028" );

            UserHelper helper = new UserHelper( user );
            String xmlContent = helper.createInitialLine( org, rpXp, defaultLine, rpCs, sipPassword );
            
            try {
                mConfigurationSetHome.create( user.getRefConfigSetID(),
                                        PDSDefinitions.PROF_TYPE_USER,
                                        user,
                                        xmlContent );
                logDebug ( "created configuration set" );
            }
            catch ( CreateException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "E2018",
                                                new Object[]{   user.getExternalID(),
                                                                user.getRefConfigSetID(),
                                                                xmlContent } ),
                        ex);
            }

        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );
            throw new EJBException  ( ex.toString() );
        }
    }


    private RefProperty getRefProperty ( String code ) throws PDSException {
        RefProperty rp = null;

        try {
            try {
                Collection c = mRefPropertyHome.findByCode( code );
                for ( Iterator i = c.iterator(); i.hasNext(); )
                    rp = (RefProperty) i.next();
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages(   "E1029",
                                                new Object[]{ "xp_10001" }),
                        ex);
            }
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );
            throw new EJBException ( ex.toString() );
        }

        return rp;
    }



    private void generateUserCentricProfiles(User user, String projectionAlg,
            boolean shouldCreateUserProf) 
    			throws PDSException, RemoteException {

        ArrayList projectionInputs = new ArrayList();

        // //////////////////////////////////////////////////////////////
        // Note: a user may have > logical phone assigned to them
        // //////////////////////////////////////////////////////////////
        Collection usersDevices = null;

        try {
            usersDevices = mDeviceHome.findByUserID(user.getID());
            logDebug("found devices for user: " + user.getExternalID());
        } catch (FinderException ex) {
            mCTX.setRollbackOnly();

            throw new PDSException(collateErrorMessages("UC670", "E1025", new Object[] { user
                    .getExternalID() }), ex);
        }

        // ///////////////////////////////////////////////////////////////
        //
        // If a user has no devices then there is no way to write
        // profiles.
        //
        // //////////////////////////////////////////////////////////////
        if (usersDevices.isEmpty()) {
            return;
        }

        if (shouldCreateUserProf) {

            logDebug("creating USER profile for user: " + user.getExternalID());

            logDebug("got projectiong inputs for user");

            for (Iterator iDevice = usersDevices.iterator(); iDevice.hasNext();) {
                Device device = (Device) iDevice.next();

                projectionInputs.clear();
                projectionInputs.addAll(getProjectionInputs(user, PDSDefinitions.PROF_TYPE_USER));

                logDebug("creating USER profile for device " + device.getExternalID());

                mProjectionHelperEJBObject.projectAndPersist(projectionAlg, device,
                        PDSDefinitions.PROF_TYPE_USER, projectionInputs);

            }
        }

        // /////////////////////////////////////////////////////////////
        // remove the existing inputs which are USER profile specific
        // /////////////////////////////////////////////////////////////
        projectionInputs.clear();
    }



    private void generatePhoneProfile(User user, String projAlg)
        throws PDSException {

        try {
            Collection devices = null;

            try {
                devices = mDeviceHome.findByUserID( user.getID());
            }
            catch (FinderException ex) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC670",
                                            "E1025",
                                            new Object[]{ user.getExternalID() } ),
                    ex);
            }

            try {
                for (Iterator phoneI = devices.iterator(); phoneI.hasNext(); ) {
                    Device lp = (Device) phoneI.next();

                    mDeviceAdvocateEJBObject.generateProfiles( lp,
                                                                STRING_PROF_TYPE_PHONE,
                                                                projAlg,
                                                                false );
                }
            }
            catch (PDSException ex) {
                mCTX.setRollbackOnly();

                throw new PDSException( collateErrorMessages( "UC670" ), ex);
            }
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            String userExternal = null;

            try { userExternal = user.getExternalID();} catch ( RemoteException rex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC670",
                                        "E4025",
                                        new Object[]{   String.valueOf(PDSDefinitions.PROF_TYPE_PHONE),
                                                        userExternal } ) );
        }
    }


    /**
     *  Gets the projectionInputs attribute of the UserAdvocateBean object
     *
     *@param  user                 Description of the Parameter
     *@param  profileType          Description of the Parameter
     *@return                      The projectionInputs value
     *@exception  PDSException     Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    private Collection getProjectionInputs(User user, int profileType)
             throws PDSException, PDSException, RemoteException {

        ArrayList projectionInputs = new ArrayList();
        Integer ugID = null;

        try {
            ugID = user.getUserGroupID();
            UserGroup ug = mUserGroupHome.findByPrimaryKey(ugID);

            projectionInputs.addAll(
                mProjectionHelperEJBObject.addParentGroupConfigSets(
                        ug,
                        profileType) );

            Collections.reverse(projectionInputs);

            ProjectionInput c =
                    mProjectionHelperEJBObject.getProjectionInput(
                            user,
                            profileType);

            if (c != null){
                projectionInputs.add(c);
            }
        }
        catch (FinderException ex) {
            mCTX.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("UC670",
                    "E1019",
                    new Object[]{ugID}),
                    ex);
        }

        return projectionInputs;
    }


    private boolean shouldUpdateExternalProfileEncryptionKey() {
        boolean returnValue = true;
        String property = getPGSProperty(EXT_PROF_ENCRYPTION_SOURCE);

        if(property != null &&  property.equalsIgnoreCase("true")){
            returnValue = false;
        }

        return returnValue;
    }


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////


}

