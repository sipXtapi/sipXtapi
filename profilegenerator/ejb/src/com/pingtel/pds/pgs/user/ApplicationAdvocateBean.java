/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/user/ApplicationAdvocateBean.java#4 $
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


import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.ejb.InternalToExternalIDTranslator;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;

import javax.ejb.*;
import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import java.rmi.RemoteException;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;



public class ApplicationAdvocateBean extends JDBCAwareEJB
    implements SessionBean, ApplicationAdvocateBusiness {

    //////////////////////////////////////////////////////////////////////////
    // Constants
    ////


    //////////////////////////////////////////////////////////////////////////
    // Attributes
    ////
    // Home interface references
    private ApplicationHome mApplicationHome;
    private UserHome mUserHome;
    private UserGroupHome mUserGroupHome;
    private ApplicationGroupAdvocateHome mApplicationGroupAdvocateHome;
    private ApplicationGroupHome mApplicationGroupHome;

    // Stateless Session Bean references;
    private ApplicationGroupAdvocate mApplicationGroupAdvocateEJBObject;

    /** The Session Context object */
    private SessionContext mCTX;


    //////////////////////////////////////////////////////////////////////////
    // Construction
    ////


    //////////////////////////////////////////////////////////////////////////
    // Public Methods
    ////
    /**
     *  Standard EJB implementation
     */
    public void ejbCreate () {}

    /**
     *  Standard EJB implementation
     */
    public void ejbRemove () {}

    /**
     *  Standard EJB implementation
     */
    public void ejbActivate () {}

    /**
     *  Standard EJB implementation
     */
    public void ejbPassivate () {}

    /**
     *  Standard EJB implementation
     */
    public void setSessionContext ( SessionContext ctx ) {

        mCTX = ctx;

        try {
            Context initial = new InitialContext();
            mApplicationHome = (ApplicationHome) initial.lookup ( "Application" );
            mUserHome = (UserHome) initial.lookup( "User" );
            mUserGroupHome = (UserGroupHome) initial.lookup( "UserGroup" );
            mApplicationGroupAdvocateHome = (ApplicationGroupAdvocateHome) initial.lookup( "ApplicationGroupAdvocate" );
            mApplicationGroupHome = (ApplicationGroupHome) initial.lookup( "ApplicationGroup" );

            mApplicationGroupAdvocateEJBObject = mApplicationGroupAdvocateHome.create();
        }
        catch ( NamingException ex ) {
            throw new EJBException ( ex.toString() );
        }
        catch ( CreateException ex ) {
            throw new EJBException ( ex.toString() );
        }
        catch ( RemoteException ex ) {
            throw new EJBException ( ex.toString() );
        }
    }

    /**
     *  Standard EJB implementation
     */
    public void unsetSessionContext () {
        mCTX = null;
    }

    /**
     * assignApplicationToUser is used to add an Application to the list of
     * Applications that get sent to a User's Devices in the Application
     * profile.
     *
     * @param userID PK of the User whom you wish to add the Application
     * to.
     * @param applicationID PK of the Application that you wish to assign
     * to the User.
     * @throws PDSException is thrown for application errors.
     */
    public void assignApplicationToUser (String userID, String applicationID )
        throws PDSException {

        String appExt = null;
        String userExt = null;
        try {
            User user = findUser ( userID );
            logDebug ("found user: " + user.getExternalID() );
            userExt = user.getExternalID();

            Application app = getApplication ( applicationID );
            logDebug ("found application: " + app.getExternalID() );

            appExt = app.getExternalID();

            createSingleApplicationGroup(   userID,
                                            "user",
                                            app,
                                            user.getOrganizationID().toString() );

            String externalUserID =
                InternalToExternalIDTranslator.getInstance().translate( mUserHome,
                                                                        userID );

            String externalAppID =
                InternalToExternalIDTranslator.getInstance().translate( mApplicationHome,
                                                                        Integer.valueOf( applicationID ) );

            logTransaction ( mCTX, "assigned application " + externalAppID +
                " to user " + externalUserID );

        }
        catch ( PDSException ex ) {
            mCTX.setRollbackOnly();
            throw new PDSException ( collateErrorMessages (  "UC355" ), ex );
        }

        catch ( RemoteException ex ) {
            logFatal ( ex.toString() );
            throw new EJBException (
                collateErrorMessages (  "UC355",
                                        "E4058",
                                        new Object [] { appExt, userExt } ) );
        }
    }

    /**
     * assignApplicationToUserGroup is used to add an Application to a UserGroup.
     * The Application is added to the list of Applications that get sent to a
     * each of the User's Devices in the given UserGroup.
     *
     * @param userGroupID PK of the UserGroup that you want to add the Application
     * to.
     * @param applicationID PK of the Application that is to be added to the
     * UserGroup
     * @throws PDSException is thrown for application errors.
     */
    public void assignApplicationToUserGroup (  String userGroupID,
                                                String applicationID )
        throws PDSException {

        String appExt = null;
        String userGroupExt = null;
        try {
            UserGroup userGroup = findUserGroup ( userGroupID );
            logDebug ("found user group: " + userGroup.getExternalID() );
            userGroupExt = userGroup.getExternalID();

            Application app = getApplication ( applicationID );
            logDebug ("found application: " + app.getExternalID() );

            appExt = app.getExternalID();

            createSingleApplicationGroup(   userGroupID,
                                            "usergroup",
                                            app,
                                            userGroup.getOrganizationID().toString() );

            String externalUserGroupID =
                InternalToExternalIDTranslator.getInstance().translate( mUserGroupHome,
                                                                        Integer.valueOf ( userGroupID ) );

            String externalAppID =
                InternalToExternalIDTranslator.getInstance().translate( mApplicationHome,
                                                                        Integer.valueOf( applicationID ) );

            logTransaction ( mCTX, "assigned application " + externalAppID +
                " to user group " + externalUserGroupID );

        }
        catch ( PDSException ex ) {
            mCTX.setRollbackOnly();
            throw new PDSException ( collateErrorMessages (  "UC360" ), ex );
        }

        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );
            throw new EJBException (
                collateErrorMessages (  "UC360",
                                        "E4059",
                                        new Object [] { appExt, userGroupExt } ) );
        }
    }


    /**
     * unassignAllUsersApplications removes all of the Applications from the given
     * User's application profile.
     *
     * @param userID PK of the User whom you want to remove the Applications from.
     * @throws PDSException is thrown for all application errors.
     */
    public void unassignAllUsersApplications (  String userID )
        throws PDSException {

        String userExt = null;

        try {
            User user = findUser ( userID );
            userExt = user.getExternalID();

            Collection userApplicationGroups = null;
            try {
                userApplicationGroups = mApplicationGroupHome.findByUserID( userID );
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "E1046",
                                            new Object [] { userExt } ),
                    ex );
            }

            for ( Iterator i = userApplicationGroups.iterator(); i.hasNext(); ) {
                ApplicationGroup ag = (ApplicationGroup) i.next();
                String agName = ag.getName();

                if ( agName.startsWith( "u" + userID )  )
                    mApplicationGroupAdvocateEJBObject.deleteApplicationGroup( ag.getID().toString() );

            }

        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            throw new EJBException  (
                collateErrorMessages (  "E4062",
                                        new Object [] { userExt } ) );
        }
    }


    /**
     * unassignAllUserGroupsApplications removes all of the Applications from
     * the application profile's of all of the Users in the given UserGroup.
     *
     * @param userGroupID PK of the UserGroup whom you wish to remove the
     * Applications from.
     * @throws PDSException is thrown for application errors.
     */
    public void unassignAllUserGroupsApplications (  String userGroupID )
        throws PDSException {

        String userGroupExt = null;

        try {
            UserGroup userGroup = findUserGroup ( userGroupID );
            userGroupExt = userGroup.getExternalID();

            Collection userApplicationGroups = null;
            try {
                userApplicationGroups = mApplicationGroupHome.findByUserGroupID ( new Integer ( userGroupID ) );
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "E1046",
                                            new Object [] { userGroupExt } ),
                    ex );
            }

            for ( Iterator i = userApplicationGroups.iterator(); i.hasNext(); ) {
                ApplicationGroup ag = (ApplicationGroup) i.next();
                String agName = ag.getName();

                if ( agName.startsWith( "ug" + userGroupID )  )
                    mApplicationGroupAdvocateEJBObject.deleteApplicationGroup( ag.getID().toString() );

            }
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            throw new EJBException  (
                collateErrorMessages (  "E4063",
                                        new Object [] { userGroupExt } ) );
        }
    }


    /**
     * installAndAssignForUser creates a new Application and assigns it to the
     * given User.
     *
     * @param name the name by which the new Application will be known to
     * users of the sytem.
     * @param organizationID PK of the Organization which this Application
     * will belong to.
     * @param deviceTypeID PK of the DeviceType that this Application is
     * available to.
     * @param refPropertyID PK of the RefProperty which this Application is
     * and instance of.  Currently all Applications should be created using
     * the Pingtel 'USER' RefProperty.   When we get around to it there are
     * other types off Applications that can run on the the phone such as
     * 'VoiceMail'.   We differentiate Applications using different
     * RefProperties because their cardinalities may be different.
     * Reason, we don't want to have Applications competing for resources,
     * such as VoiceMail or CallLogging.
     * @param url URL that will be sent to the Devices as being the location
     * of the the Application.
     * @param description description of the new Application.
     * @throws PDSException
     * @see #assignApplicationToUser
     */
    public void installAndAssignForUser   ( String userID,
                                            String name,
                                            String organizationID,
                                            String deviceTypeID,
                                            String refPropertyID,
                                            String url,
                                            String description )
        throws PDSException {

        String userExt = null;
        String appExt = null;

        try {
            User user = findUser ( userID );
            userExt = user.getExternalID();

            Application app = createApplication (   name,
                                                    organizationID,
                                                    deviceTypeID,
                                                    refPropertyID,
                                                    url,
                                                    description );

            appExt = app.getExternalID();

            createSingleApplicationGroup (  userID,
                                            "user",
                                            app,
                                            user.getOrganizationID().toString() );

            String externalUserID =
                InternalToExternalIDTranslator.getInstance().translate( mUserHome,
                                                                        userID );


            logTransaction ( mCTX, "installed and assigned application " +
                app.getExternalID() + " to user  " + externalUserID );

        }
        catch ( PDSException ex ) {
            mCTX.setRollbackOnly();
            throw new PDSException ( collateErrorMessages (  "UC355" ), ex );
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            throw new EJBException  (
                collateErrorMessages (  "UC355",
                                        "E4058",
                                        new Object [] { appExt, userExt } ) );
        }
    }


    /**
     * uninstallApplicationFromUser removes the given Application from the
     * User's application profile.
     *
     * @param userID PK of the User you want to remove the Application from
     * @param applicationID PK of the Application that you want to remove
     * from the User's application profile.
     * @throws PDSException is thrown for application errors.
     */
    public void uninstallApplicationFromUser (  String userID,
                                                String applicationID )
            throws PDSException {

        String userExt = null;
        String appExt = null;

        logDebug ( "uninstalling application " + applicationID + " from user  " + userID );

        try {
            User user = findUser ( userID );
            userExt = user.getExternalID();
            Application app = getApplication ( applicationID );

            ApplicationGroup ag =  null;

            try {
                Collection c = mApplicationGroupHome.findByName( "u" + userID + "_" + applicationID );
                for ( Iterator iC = c.iterator(); iC.hasNext(); )
                    ag = (ApplicationGroup) iC.next();
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC365",
                                            "E1045",
                                            new Object [] { "u" + userID + "_" + applicationID } ),
                    ex );

            }
            mApplicationGroupAdvocateEJBObject.deleteApplicationGroup( ag.getID().toString() );

            String externalUserID =
                InternalToExternalIDTranslator.getInstance().translate( mUserHome,
                                                                        userID );

            String externalAppID =
                InternalToExternalIDTranslator.getInstance().translate( mApplicationHome,
                                                                        Integer.valueOf ( applicationID ) );


            logTransaction ( mCTX, "uninstalled application " +
                externalAppID + " from user  " + externalUserID );
        }
        catch ( PDSException ex ) {
            mCTX.setRollbackOnly();
            throw new PDSException ( collateErrorMessages (  "UC365" ), ex );
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            throw new EJBException  (
                collateErrorMessages (  "UC365",
                                        "E4060",
                                        new Object [] { appExt, userExt } ) );
        }
    }


    /**
     * uninstallApplicationFromUserGroup removes the given Application from
     * the application profiles of all of the Users in the given UserGroup.
     *
     * @param userGroupID PK of the UserGroup who you want to remove the
     * Application from.
     * @param applicationID PK of the Application that you are removing from
     * the UserGroup.
     * @throws PDSException is thrown for application errors.
     */
    public void uninstallApplicationFromUserGroup ( String userGroupID,
                                                    String applicationID )
        throws PDSException {

        String userGroupExt = null;
        String appExt = null;

        try {
            UserGroup userGroup = findUserGroup ( userGroupID );
            userGroupExt = userGroup.getExternalID();
            Application app = getApplication ( applicationID );

            ApplicationGroup ag =  null;

            try {
                Collection c = mApplicationGroupHome.findByName( "ug" + userGroupID + "_" + applicationID );
                for ( Iterator iC = c.iterator(); iC.hasNext(); )
                    ag = (ApplicationGroup) iC.next();
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC370",
                                            "E1045",
                                            new Object [] { "ug" + userGroupID + "_" + applicationID } ),
                    ex );

            }

            mApplicationGroupAdvocateEJBObject.deleteApplicationGroup( ag.getID().toString() );

            String externalUserGroupID =
                InternalToExternalIDTranslator.getInstance().translate( mUserGroupHome,
                                                                        Integer.valueOf ( userGroupID ) );

            String externalAppID =
                InternalToExternalIDTranslator.getInstance().translate( mApplicationHome,
                                                                        Integer.valueOf ( applicationID ) );


            logTransaction ( mCTX, "uninstalled application " +
                externalAppID + " from user  " + externalUserGroupID );
        }
        catch ( PDSException ex ) {
            mCTX.setRollbackOnly();
            throw new PDSException ( collateErrorMessages (  "UC370" ), ex );
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            throw new EJBException  (
                collateErrorMessages (  "UC370",
                                        "E4061",
                                        new Object [] { appExt, userGroupExt } ) );
        }
    }


    /**
     * createApplication create a new Application in the sytem.
     *
     * @param name the name by which the new Application will be known to
     * users of the sytem.
     * @param organizationID PK of the Organization which this Application
     * will belong to.
     * @param deviceTypeID PK of the DeviceType that this Application is
     * available to.
     * @param refPropertyID PK of the RefProperty which this Application is
     * and instance of.  Currently all Applications should be created using
     * the Pingtel 'USER' RefProperty.   When we get around to it there are
     * other types off Applications that can run on the the phone such as
     * 'VoiceMail'.   We differentiate Applications using different
     * RefProperties because their cardinalities may be different.
     * Reason, we don't want to have Applications competing for resources,
     * such as VoiceMail or CallLogging.
     * @param url URL that will be sent to the Devices as being the location
     * of the the Application.
     * @param description description of the new Application.
     * @throws PDSException
     */
    public Application createApplication (  String name,
                                            String organizationID,
                                            String deviceTypeID,
                                            String refPropertyID,
                                            String url,
                                            String description )
        throws PDSException {

        Application newApp = null;

        try {
            ArrayList rows =
                    executePreparedQuery (  "SELECT ID FROM APPLICATIONS WHERE URL = ?",
                                            new Object [] {url},
                                            1,
                                            1 );

            if ( !rows.isEmpty() ) {
                mCTX.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages ( "UC300", "E2037", new Object [] {url}));
            }

            newApp =  mApplicationHome.create( name,
                                        new Integer ( organizationID ),
                                        new Integer ( deviceTypeID ),
                                        new Integer ( refPropertyID ),
                                        url,
                                        description );

            logTransaction ( mCTX, "created application " + newApp.getExternalID() );
        }
        catch ( CreateException ex ) {
            mCTX.setRollbackOnly();

            throw new PDSException (
                collateErrorMessages (  "UC300",
                                        "E2007",
                                        new Object [] { name,
                                                        url,
                                                        description } ),
                ex );
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            throw new EJBException (
                collateErrorMessages (  "UC300",
                                        "E4013",
                                        new Object [] { name,
                                                        url,
                                                        description } ) );

        }
        catch ( SQLException ex ) {
            logFatal ( ex.toString(), ex );

            throw new EJBException (
                collateErrorMessages (  "UC300",
                                        "E4013",
                                        new Object [] { name,
                                                        url,
                                                        description } ) );

        }

        return newApp;
    }


    /**
     * deleteApplication removes an Application and all it's assignments to
     * User and User Groups, from the system.
     *
     * @param applicationID
     * @throws PDSException
     */
    public void deleteApplication ( String applicationID )
        throws PDSException {

        Application app = null;

        try {
            String externalAppID =
                InternalToExternalIDTranslator.getInstance().translate(
                        mApplicationHome,
                        Integer.valueOf ( applicationID ) );

            try {
                app = mApplicationHome.findByPrimaryKey( new Integer ( applicationID ) );
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC310",
                                            "E1037",
                                            new Object [] { applicationID } ),
                    ex );
            }

            unassignApplicationFromAllApplicationGroups ( app );

            ////////////////////////////////////////////////////////////////////
            //
            // This coded added to delete application sets which are bound to
            // only the application to be deleted.   This occurs when an app
            // is installed to a user/user group
            //
            ////////////////////////////////////////////////////////////////////
            try {
                Collection applicationGroups =
                    mApplicationGroupHome.findByOrganizationID( app.getOrganizationID() );

                for ( Iterator iGroups = applicationGroups.iterator(); iGroups.hasNext(); ) {
                    ApplicationGroup ag = (ApplicationGroup) iGroups.next();
                    String agName = ag.getName();
                    if ( agName.endsWith( app.getID().toString() ) ) {
                        mApplicationGroupAdvocateEJBObject.deleteApplicationGroup( ag.getID().toString() );
                    }
                }
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC310",
                                            "E1044",
                                            null ),
                    ex );
            }

            try {
                app.remove();
            }
            catch ( RemoveException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC310",
                                            "E6017",
                                            new Object [] { app.getExternalID() } ),
                    ex );
            }

            logTransaction ( mCTX, "deleted application " + externalAppID  );
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            String external = null;

            try { external = app.getExternalID(); } catch ( RemoteException rex ) {}

            throw new EJBException (
                collateErrorMessages (  "UC310",
                                        "E4054",
                                        new Object [] { external == null ?
                                                            applicationID :
                                                            external } ) );

        }
    }

    /**
     * editApplication modifies the attributes of an existing Application.
     *
     * @param applicationID PK of the Application you want to modify.
     * @param name the new name that you want to call the Application.
     * @param url the new URL that you want to use for the Application.
     * @param description the new description you want to use for the
     * Application.
     * @throws PDSException is thrown for application errors.
     */
    public void editApplication (   String applicationID,
                                    String name,
                                    String url,
                                    String description )
        throws PDSException {

        Application app = null;
        try {
            try {
                app = mApplicationHome.findByPrimaryKey( new Integer ( applicationID ) );
            }
            catch ( FinderException ex ) {
                mCTX.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC315",
                                            "E1037",
                                            new Object [] { applicationID } ),
                                            ex );
            }
            if ( name != null &&
                    ( app.getName() == null ||
                    !name.equals( app.getName() ) ) ) {

                app.setName( name );
            }
            if ( url != null &&
                    ( app.getURL() == null ||
                    !url.equals( app.getURL() ) ) ) {

                app.setURL( url );
            }
            if ( description != null &&
                    ( app.getDescription() == null ||
                    !description.equals( app.getDescription() ) ) ) {

                app.setDescription( description );
            }
        }
        catch ( RemoteException ex )
        {
            logFatal ( ex.toString(), ex );

            throw new EJBException (
                collateErrorMessages (  "UC315",
                                        "E4069",
                                        new Object [] { applicationID } ) );
        }

    }




    //////////////////////////////////////////////////////////////////////////
    // Implementation Methods
    ////
    private void createSingleApplicationGroup ( String entityID,
                                                String entityType,
                                                Application application,
                                                String organizationID )
        throws PDSException, RemoteException {

            StringBuffer entityString = new StringBuffer();

            if ( entityType.equals( "user" ) ) {
                entityString.append( "u");
            }
            else {
                entityString.append( "ug");
            }

            entityString.append( entityID );

            ApplicationGroup appGroup =
                mApplicationGroupAdvocateEJBObject.
                    createApplicationGroup( entityString.toString() +
                                                "_" + application.getID(),
                                            organizationID );


            logDebug ("created applcationgroup: " + appGroup.getExternalID() );


            mApplicationGroupAdvocateEJBObject.
                assignApplicationToApplicationGroup(
                                            application.getID().toString(),
                                            appGroup.getID().toString() );

            logDebug ("assigned app to applcationgroup"  );

            if ( entityType.equals( "user" ) ) {
                mApplicationGroupAdvocateEJBObject.
                    assignUserAndApplicationGroup(  entityID,
                                                    appGroup.getID().toString() );
            }
            else {
                mApplicationGroupAdvocateEJBObject.
                    assignUserGroupAndApplicationGroup( entityID,
                                                        appGroup.getID().toString() );
            }

            logDebug ("assigned entity to application group"  );

    }


    private void unassignApplicationFromAllApplicationGroups ( Application application )
        throws PDSException, RemoteException {

        try {
            executePreparedUpdate ( "DELETE FROM APP_AG_ASSOC " +
                                    "WHERE apps_id = ?",
                                    new Object [] { application.getID() } );
        }
        catch ( SQLException ex ) {
            mCTX.setRollbackOnly();

            throw new PDSException (
                collateErrorMessages (  "UC310",
                                        "E5016",
                                        new Object [] { application.getExternalID() } ),
                ex );
        }
    }



    private User findUser ( String userID )
        throws PDSException, RemoteException {

        try {
            logDebug ( "top of find User" );
            if ( mUserHome == null )
                logDebug ( "userHOme is null" );
            User user = mUserHome.findByPrimaryKey( userID ) ;
            logDebug ( "found user: " + user.getExternalID() );
            return user;
        }
        catch ( FinderException ex ) {
            mCTX.setRollbackOnly();

            throw new PDSException (
                collateErrorMessages (  "E1026",
                                        new Object [] { userID } ),
                ex );
        }

    }


    private UserGroup findUserGroup ( String userGroupID )
        throws PDSException, RemoteException {

        try {
            UserGroup userGroup =
                mUserGroupHome.findByPrimaryKey( Integer.valueOf( userGroupID ));

            return userGroup;
        }
        catch ( FinderException ex ) {
            mCTX.setRollbackOnly();

            throw new PDSException (
                collateErrorMessages (  "E1019",
                                        new Object [] { userGroupID } ),
                ex );
        }

    }


    private Application getApplication ( String applicationID )
        throws PDSException, RemoteException {

        Application app = null;

        try {
            app = this.mApplicationHome.findByPrimaryKey( new Integer ( applicationID ) );
            return app;
        }
        catch ( FinderException ex ) {
            mCTX.setRollbackOnly();

            throw new PDSException (
                collateErrorMessages (  "E1037",
                                        new Object [] { applicationID } ),
                ex );
        }
    }


    //////////////////////////////////////////////////////////////////////////
    // Nested / Inner classes
    ////


    //////////////////////////////////////////////////////////////////////////
    // Native Method Declarations
    ////

}
