/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/user/UserGroupAdvocateBean.java#4 $
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

import javax.ejb.CreateException;
import javax.ejb.EJBException;
import javax.ejb.FinderException;
import javax.ejb.RemoveException;
import javax.ejb.SessionBean;
import javax.ejb.SessionContext;
import javax.naming.Context;
import javax.naming.InitialContext;

import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.Collections;

import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.MasterDetailsMap;
import com.pingtel.pds.pgs.common.PGSDefinitions;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.common.ejb.InternalToExternalIDTranslator;
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
import com.pingtel.pds.pgs.jobs.JobManager;
import com.pingtel.pds.pgs.jobs.JobManagerHome;


/**
 * UserGroupAdvocateBean is the EJBean implmentation class
 * for UserGroupAdvocate.
 *
 * @author IB
 */
public class UserGroupAdvocateBean extends JDBCAwareEJB
         implements SessionBean, UserGroupAdvocateBusiness {


    // Home references
    private UserHome m_userHome;
    private UserGroupHome m_userGroupHome;
    private DeviceHome m_deviceHome;
    private ConfigurationSetHome m_configurationSetHome;

    // Stateless Session Bean references
    private ApplicationAdvocate m_applicationAdvocateEJBObject;
    private ApplicationGroupAdvocate m_applicationGroupAdvocateEJBObject;
    private UserAdvocate m_userAdvocateEJBObject;
    private DeviceAdvocate m_deviceAdvocateEJBObject;
    private ProjectionHelper m_projectionHelperEJBObject;
    private JobManager m_jobManagerEJBObject;


    /**
     *  The Session Context object
     */
    private SessionContext m_ctx;

    /**
     * createUserGroup implements the create user group use case.
     *
     * @param name the name that you want to give the new User Group.
     * @param parentID the PK of the user group that should be the
     * parent group of this user group (optional)
     * @param refConfigSetID the PK of the ref config set (Entity)
     * that will be the assigned to the user group.
     * @param organizationID the PK of the organization (Entity) that
     * will be assigned to the user group.
     * @return the EJBObject for the new user group.
     * @throws PDSException
     */
    public UserGroup createUserGroup(   String name,
                                        String parentID,
                                        String refConfigSetID,
                                        String organizationID)
        throws PDSException {

        return createUserGroup( name,
                                parentID != null ? new Integer ( parentID ) : null,
                                new Integer ( refConfigSetID ),
                                new Integer ( organizationID ));
    }


    /**
     * NOTE: this is the local version of the use case, the web UI
     * should use the other version of this method.
     *
     * createUserGroup implements the create user group use case.
     *
     * @param name the name that you want to give the new User Group.
     * @param parentID the PK of the user group that should be the
     * parent group of this user group (optional)
     * @param refConfigSetID the PK of the ref config set (Entity)
     * that will be the assigned to the user group.
     * @param organizationID the PK of the organization (Entity) that
     * will be assigned to the user group.
     * @return the EJBObject for the new user group.
     * @throws PDSException
     */
    public UserGroup createUserGroup(   String name,
                                        Integer parentID,
                                        Integer refConfigSetID,
                                        Integer organizationID)
        throws PDSException {

        try {
            checkSiblingUnique ( parentID, name );

            UserGroup ug =
                m_userGroupHome.create( name, organizationID, refConfigSetID, parentID );

            logTransaction ( m_ctx, "create user group" + ug.getExternalID() );

            return ug;
        }
        catch ( PDSException ex) {
            // We are catching PDS exceptions thrown from checkSiblingUnique
            m_ctx.setRollbackOnly();

            throw new PDSException( collateErrorMessages( "UC610"), ex );
        }
        catch ( CreateException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC610",
                                        "E2012",
                                        new Object[]{ name }),
                ex);
        }
        catch ( FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC610",
                                        "E2012",
                                        new Object[]{ name }),
                ex);
        }
        catch ( RemoteException ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC610",
                                        "E4026",
                                        new Object[]{name}));
        }
    }


    /**
     * editUserGroup implements the edit user group use case.
     * It provides a session facade to set the attributes of
     * a user group.    All parameters are optional with the exception
     * of userGroupID.   If a null is passed for a parameter then
     * one of two things happens.   If the existing attribute value
     * is null then nothing happens.   If the existing attribute value
     * is not null then it is set to null.
     *
     * @param userGroupID the PK of user group that you wish to edit.
     * @param name the new name for the user group (optional).
     * @param parentID the PK of the new parent user group (optional).
     * @param refConfigSetID the PK of the new ref config set (optional).
     * @throws PDSException is thrown for application level errors.
     */
    public boolean [] editUserGroup(  String userGroupID,
                                String name,
                                String parentID,
                                String refConfigSetID) throws PDSException {

        UserGroup ug = null;

        boolean [] changedValues = new boolean [ PGSDefinitions.USER_CHANGE_ARRAY_SIZE ];

        changedValues [ PGSDefinitions.EXTENSION_CHANGED ] = false;
        changedValues [ PGSDefinitions.ALIASES_CHANGED ] = false;
        changedValues [ PGSDefinitions.LINE_INFO_CHANGED ] = false;
        changedValues [ PGSDefinitions.HIERARCHY_CHANGED ] = false;

        try {
            try {
                ug = m_userGroupHome.findByPrimaryKey(new Integer(userGroupID));
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                        collateErrorMessages("UC617",
                        "E1019",
                        new Object[]{userGroupID}),
                        ex);
            }

            Integer parentIDInteger = parentID == null ? null : Integer.valueOf( parentID );
            String existingName = ug.getName();

            if ( name != null && ( existingName == null || !existingName.equals(name) ) ) {
                checkSiblingUnique( parentIDInteger, name );
                ug.setName(name);
            }

            Integer existingParentID = ug.getParentID();

            if ( ( parentIDInteger != null &&
                    ( existingParentID == null || !existingParentID.equals( parentIDInteger )) ) ||
                    parentIDInteger == null && existingParentID != null ) {

                ///////////////////////////////////////////////////////////////////////////////
                //
                // We are moving the user group to another parent, we need to make sure that
                // the new parent group doesn't have an existing sibling with the same name.
                //
                ///////////////////////////////////////////////////////////////////////////////
                checkSiblingUnique( parentIDInteger, name );

                ug.setParentID( parentIDInteger );
                changedValues [ PGSDefinitions.HIERARCHY_CHANGED ] = true;
            }

            Integer existingRCSID = ug.getRefConfigSetID();

            if (refConfigSetID != null &&
                    ( existingRCSID == null || !existingRCSID.toString().equals( refConfigSetID ) ) ) {

                ug.setRefConfigSetID(new Integer(refConfigSetID));
            }

            return changedValues;
        }
        catch ( PDSException ex) {
            // We are catching PDS exceptions thrown from checkSiblingUnique
            m_ctx.setRollbackOnly();

            throw new PDSException( collateErrorMessages( "UC617"), ex );
        }
        catch ( FinderException ex) {
            logFatal ( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC617",
                                        "E4028",
                                        new Object[]{   userGroupID,
                                                        name,
                                                        parentID,
                                                        refConfigSetID}));
        }
        catch (RemoteException ex) {
            logFatal ( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC617",
                                        "E4028",
                                        new Object[]{   userGroupID,
                                                        name,
                                                        parentID,
                                                        refConfigSetID}));
        }
    }

    private void checkSiblingUnique( Integer parentID, String name )
            throws FinderException, RemoteException, PDSException {

        Collection peers = new ArrayList();

        if ( parentID != null ) {
            peers = m_userGroupHome.findByParentID( parentID );
        }
        else {
            Collection allGroups = m_userGroupHome.findAll( );
            for ( Iterator iAll = allGroups.iterator(); iAll.hasNext(); ) {
                UserGroup group = (UserGroup) iAll.next();

                if ( group.getParentID() == null ) {
                    peers.add( group );
                }
            }
        }

        for ( Iterator iPeer = peers.iterator(); iPeer.hasNext(); ) {
            UserGroup peer = (UserGroup) iPeer.next();

            if ( peer.getName().equalsIgnoreCase( name ) ) {
                m_ctx.setRollbackOnly();
                throw new PDSException ( collateErrorMessages ( "E2029", new Object [] { name } ) );
            }
        }
    }


    /**
     * deleteUserGroup implements the delete user group use case.
     * Various other dependant entities are deleted as part of
     * the method.
     *
     * @param userGroupID the PK of the user group to delete.
     * @param option DeviceGroups can be delete in three ways:
     * PGSDefinitions.DEL_DEEP_DELETE is the only valid value currently.
     * @param newUserGroupID not currently used.
     * @throws PDSException is thrown for application level errors.
     */
    public void deleteUserGroup(    String userGroupID,
                                    String option,
                                    String newUserGroupID)
        throws PDSException {

        UserGroup ug = null;

        try {
            ug = m_userGroupHome.findByPrimaryKey( new Integer ( userGroupID ) );
        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "E615",
                                        "E1019",
                                        new Object[]{ userGroupID }),
                ex);
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                collateErrorMessages(   "UC615",
                                        "E4044",
                                        new Object[]{ userGroupID } ) );
        }

        deleteUserGroup(    ug,
                            new Integer ( option ).intValue(),
                            newUserGroupID != null ?
                                new Integer ( newUserGroupID ) :
                                null );
    }


    /**
     * NOTE: this is the local implementation, the web UI should
     * use the other version of this method.
     *
     * deleteUserGroup implements the delete user group use case.
     * Various other dependant entities are deleted as part of
     * the method.
     *
     * @param ug the EJBObject of the user group to delete.
     * @param option DeviceGroups can be delete in three ways:
     * PGSDefinitions.DEL_DEEP_DELETE is the only valid value currently.
     * @param newUserGroupID not currently used.
     * @throws PDSException is thrown for application level errors.
     */
    public void deleteUserGroup(    UserGroup ug,
                                    int option,
                                    Integer newUserGroupID)
        throws PDSException {

        try {

            if ( option == PGSDefinitions.DEL_DEEP_DELETE) {

                Collection children = null;

                try {
                    children = m_userGroupHome.findByParentID( ug.getID() );
                }
                catch (FinderException ex) {
                    m_ctx.setRollbackOnly();

                    throw new PDSException(
                        collateErrorMessages(   "E615",
                                                "E1027",
                                                new Object[]{ ug.getExternalID() }),
                        ex);
                }

                for (Iterator iChild = children.iterator(); iChild.hasNext(); ) {
                    UserGroup ugChild = (UserGroup) iChild.next();

                    deleteUserGroup(    ugChild,
                                        option,
                                        null);
                }
            }
            // if

            deleteUserGroupConfigSets( ug );

            switch ( option ) {
                case PGSDefinitions.DEL_DEEP_DELETE:
                {
                    deepDelete( ug );
                    break;
                }
                case PGSDefinitions.DEL_NO_GROUP:
                {
                    // TBD
                    break;
                }
                case PGSDefinitions.DEL_REASSIGN:
                {
                    // TBD
                    break;
                }
            }
            // switch

            // New stuff to delete user's mini- application sets i.e. ones that
            // can and do only belong to the user as a holder for a single application
            // that is assigned to the user.
            m_applicationAdvocateEJBObject.unassignAllUserGroupsApplications( ug.getID().toString() );
            logDebug (  "deleted user group's application (sets)" );
            m_applicationGroupAdvocateEJBObject.unassignAllUserGroupsApplicationGroups( ug.getID() );
            logDebug (  "unassigned user group's application (sets)" );

            String externalUserGroupID = ug.getExternalID();

            try {
                ug.remove();
            }
            catch (RemoveException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "E615",
                                            "E6014",
                                            new Object[]{ ug.getExternalID() }),
                                            ex);

            }

            logTransaction ( m_ctx, "deleted user group " + externalUserGroupID );
        }
        catch (RemoteException ex) {
            logFatal( ex.toString(), ex );

            String ugExternal = null;

            try { ugExternal = ug.getExternalID(); } catch ( RemoteException rex ) {}

            throw new EJBException(
                collateErrorMessages(   "UC615",
                                        "E4044",
                                        new Object[]{ ugExternal } ) );
        }
    }



    private void deleteUserGroupConfigSets(UserGroup userGroup)
             throws PDSException, RemoteException {

        try {
            Collection ugConfigSets =
                    m_configurationSetHome.findByUserGroupID( userGroup.getID() );

            for (Iterator iCS = ugConfigSets.iterator(); iCS.hasNext(); ) {
                ((ConfigurationSet) iCS.next()).remove();
            }
        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "E615",
                                        "E1031",
                                        new Object[]{userGroup.getExternalID() } ),
                ex);
        }
        catch (RemoveException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                    collateErrorMessages("E615",
                    "E6015",
                    new Object[]{ userGroup.getExternalID() } ),
                    ex);
        }
    }



    private void deepDelete( UserGroup userGroup )
        throws PDSException, RemoteException {

        Collection existing = null;
        try {
            existing = m_userHome.findByUserGroupID( userGroup.getID() );
        }
        catch (FinderException ex) {
            m_ctx.setRollbackOnly();

            throw new PDSException(
                collateErrorMessages(   "UC615",
                                        "E1029",
                                        new Object[]{userGroup.getExternalID() } ),
                ex);
        }

        for (Iterator iUser = existing.iterator(); iUser.hasNext(); ) {
            m_userAdvocateEJBObject.deleteUser((User) iUser.next() );
        }
    }


    /**
     * copyUserGroup implements the copy user group use case.
     * @deprecated this method should not be used for now.
     * @param sourceUserGroupID the PK of the user group that
     * should be copied.
     * @return the EJBObject of the new (copy) user group.
     * @throws PDSException is thrown for application level errors.
     */
    public UserGroup copyUserGroup ( String sourceUserGroupID )
        throws PDSException {

        UserGroup source = null;
        UserGroup copy = null;
        Integer sourceID = new Integer ( sourceUserGroupID );

        try {
            try {
                source = m_userGroupHome.findByPrimaryKey( sourceID );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC618",
                                            "E1019",
                                            new Object[]{ sourceUserGroupID } ),
                    ex);
            }

            try {
                copy = createUserGroup( "Copy of " + source.getName(),
                                        source.getParentID(),
                                        source.getRefConfigSetID(),
                                        source.getOrganizationID() );
            }
            catch ( PDSException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException( collateErrorMessages( "UC618" ), ex );
            }

            /*ConfigurationSet sourceCS = null;

            try {
                Collection c = m_configurationSetHome.findByUserGroupID ( sourceID );

                for ( Iterator i = c.iterator(); i.hasNext(); ) {
                    sourceCS = (ConfigurationSet) i.next();

                    m_configurationSetHome.create(    copy.getRefConfigSetID(),
                                        sourceCS.getProfileType(),
                                        copy,
                                        sourceCS.getContent() );
                }
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC618",
                                            "E1031",
                                            new Object[]{ source.getExternalID() } ),
                    ex);
            }
            catch (CreateException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC618",
                                            "E2019",
                                            new Object[]{   copy.getID(),
                                                            copy.getRefConfigSetID(),
                                                            String.valueOf ( sourceCS.getProfileType() ),
                                                            sourceCS.getContent() } ),
                    ex );
            }*/

        }
        catch ( RemoteException ex ) {
            logFatal( ex.toString(), ex );

            String external = null;

            try { external = source.getExternalID(); } catch ( RemoteException rex ) {}

            throw new EJBException(
                    collateErrorMessages(   "UC618",
                                            "E4041",
                                            new Object[]{   external == null ?
                                                                sourceUserGroupID :
                                                                external } ) );
        }

        return copy;
    }


    /**
     * restarts all devices of all of the users who are in the given
     * user group.   This is also recursive, if the user group is the
     * parent of other user groups then they will be restarted.
     *
     * @param userGroupID the PK of the user group to restart.
     * @throws PDSException is thrown for application level errors.
     */
    public void restartDevices ( String userGroupID ) throws PDSException {

        String ugExternal = null;

        try {
            UserGroup ug = null;

            try {
                ug = m_userGroupHome.findByPrimaryKey( Integer.valueOf( userGroupID ) );
                ugExternal = ug.getExternalID();
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC466",
                                            "E1019",
                                            new Object[]{ userGroupID } ),
                    ex);
            }

            Collection users = null;

            try {
                users = m_userHome.findByUserGroupID( Integer.valueOf( userGroupID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException(
                    collateErrorMessages(   "UC466",
                                            "E1028",
                                            new Object[]{ ugExternal } ),
                    ex);
            }

            for ( Iterator iUser = users.iterator(); iUser.hasNext(); ) {
                User user = (User) iUser.next();
                m_userAdvocateEJBObject.restartDevices( user.getID() );
            }

            String externalUserGroupID =
                InternalToExternalIDTranslator.getInstance().translate( m_userGroupHome,
                                                                        Integer.valueOf( userGroupID) );

            logTransaction ( m_ctx, "restarted user group " + externalUserGroupID +
                " devices" );

            Collection children =
                m_userGroupHome.findByParentID( Integer.valueOf( userGroupID ) );

            for ( Iterator iChild = children.iterator(); iChild.hasNext(); ) {
                UserGroup child = (UserGroup) iChild.next();
                restartDevices ( child.getID().toString() );
            }

        }
        catch ( PDSException ex ) {
            throw new PDSException ( collateErrorMessages ( "UC466" ), ex );
        }
        catch ( RemoteException ex ) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                    collateErrorMessages(   "UC466",
                                            "E4073",
                                            new Object[]{ ugExternal } ) );
        }
        catch ( FinderException ex ) {
            logFatal( ex.toString(), ex );

            throw new EJBException(
                    collateErrorMessages(   "UC466",
                                            "E4073",
                                            new Object[]{ ugExternal } ) );
        }
    }



    /**
     * generateProfiles implements the project profile use case
     * for a user group.   This is also recursive, if the user group is the
     * parent of other user groups then they will be projected.
     *
     * @param userGroupID the PK of the user group whose profiles
     * should be generated.
     * @param profileTypes comma-separated string of the profile types
     * to be generated for this User Group.  For the permissible values are
     * in PGSDefinitions - definitions for these values are ( PROF_TYPE_PHONE,
     * PROF_TYPE_USER, PROF_TYPE_APPLICATION_REF).
     * @param projAlg is the name of the class to be used for the
     * projection part of the operation (optional if a null is passed then
     * the default projection class for the phone type and profile is
     * used instead).
     * @throws PDSException is thrown for application level errors.
     */
    public void generateProfiles(   String userGroupID,
                                    String profileTypes,
                                    String projAlg )
        throws PDSException {

        boolean[] profTypesToCreate = null;
        UserGroup ug = null;
        int jobID = -1;

        try {
            try {
                ug = m_userGroupHome.findByPrimaryKey( new Integer ( userGroupID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC675",
                                            "E1019",
                                            new Object [] { userGroupID } ),
                    ex );
            }

            try {
                profTypesToCreate = ProfileTypeStringParser.parse(profileTypes);
            }
            catch (PDSException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException( collateErrorMessages( "UC675" ), ex );
            }

            StringBuffer jobDetails = new StringBuffer ();
            jobDetails.append( "Projection for user group: ");
            jobDetails.append( ug.getExternalID());
            jobDetails.append ( " profile types: ");

            if ( profTypesToCreate[ PDSDefinitions.PROF_TYPE_USER ] ) {
                jobDetails.append ( "user ");
            }
            if ( profTypesToCreate[ PDSDefinitions.PROF_TYPE_PHONE ] ) {
                jobDetails.append ( "device ");
            }
            if ( profTypesToCreate[ PDSDefinitions.PROF_TYPE_APPLICATION_REF ] ) {
                jobDetails.append ( "application");
            }

            int numberOfUsersToProject = m_projectionHelperEJBObject.calculateTotalProfiles( ug );

            String initialProgress = "projected 0 of " + numberOfUsersToProject + " users.";

            jobID =
                m_jobManagerEJBObject.createJob( JobManager.PROJECTION, jobDetails.toString(), initialProgress );

            generateGroupProfiles ( ug, profTypesToCreate, projAlg, jobID, numberOfUsersToProject, 0 );

            m_jobManagerEJBObject.updateJobStatus( jobID, JobManager.COMPLETE, null );
        }
        catch ( PDSException ex ) {
            if ( jobID != -1 ) {
                try { m_jobManagerEJBObject.updateJobStatus( jobID, JobManager.FAILED, ex.toString() ); }
                catch (Exception e) {}
            }

            throw ex;
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            if ( jobID != -1 ) {
                try { m_jobManagerEJBObject.updateJobStatus( jobID, JobManager.FAILED, ex.toString() ); }
                catch (Exception e) {}
            }

            throw new EJBException (
                collateErrorMessages (  "UC675",
                                        "E4027",
                                        new Object [] { userGroupID } ) );

        }
    }


    ///////////////////////////////////////////////////////////////////////////////////////
    // This method, whilst not complex is subtle and suboptimal.   It is basic recursion,
    // doing a pre-order project of the given user group.   See comments below for
    // more detail.
    ///////////////////////////////////////////////////////////////////////////////////////
    private int generateGroupProfiles(  UserGroup ug,
                                        boolean[] profTypesToCreate,
                                        String projAlg,
                                        int jobID,
                                        int totalUsersToProject,
                                        int currentTotal )
        throws PDSException, RemoteException {


        try {

            ///////////////////////////////////////////////////////////////////////
            //
            // See if this UserGroup has any children - if so then project for
            // them too.
            //
            ///////////////////////////////////////////////////////////////////////
            Collection children = null;

            try {
                children = m_userGroupHome.findByParentID( ug.getID() );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC675",
                                            "E1027",
                                            new Object [] { ug.getExternalID() } ),
                    ex );
            }

            /*
             * The nastiness creeps in here because of the job status updates that have to be done.
             * For this to be done correctly we need the jobID, total number of users to project
             * and the 'currentTotal'.   The current total is passed as an argument to this
             * method for each child group.   Each group gets the sum of all of the users
             * in all of their child groups and all of the users in their own group.   In the
             * case of > 1 directy children groups the last child gets the total number of users
             * in all its previoulsy projected siblings.   Therefore it is safe to only update the
             * currentTotal parameter with the return value for that group.
             */
            for ( Iterator iChild = children.iterator(); iChild.hasNext(); ) {
                UserGroup child = (UserGroup) iChild.next();

                currentTotal =
                    generateGroupProfiles(  child,
                                            profTypesToCreate,
                                            projAlg,
                                            jobID,
                                            totalUsersToProject,
                                            currentTotal );
            }

            if (    profTypesToCreate[ PDSDefinitions.PROF_TYPE_USER ] ||
                    profTypesToCreate[ PDSDefinitions.PROF_TYPE_APPLICATION_REF ] ||
                    profTypesToCreate[ PDSDefinitions.PROF_TYPE_PHONE ] ) {

                currentTotal =
                    generateIndividualProfiles( ug,
                                                projAlg,
                                                profTypesToCreate,
                                                jobID,
                                                totalUsersToProject,
                                                currentTotal );
            }

        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            throw new EJBException (
                collateErrorMessages (  "UC675",
                                        "E4027",
                                        new Object [] { ug.getExternalID() } ) );

        }

        return currentTotal;
    }


    ///////////////////////////////////////////////////////////////////////
    //
    // generateIndividualProfiles produces User and/or Application type
    // profiles for a given User Group.   If not projection algorithm
    // class name is given then the default one for the Physical Device's
    // core software is used.
    //
    ///////////////////////////////////////////////////////////////////////
    private int generateIndividualProfiles( UserGroup ug,
                                            String projectionAlg,
                                            boolean [] profileTypes,
                                            int jobID,
                                            int totalUsersToProject,
                                            int currentTotal)
        throws PDSException, RemoteException {

        Integer userGroupID = null;
        MasterDetailsMap partProjectedProfiles = new MasterDetailsMap ();

        try {

            ///////////////////////////////////////////////////////////////////////
            //
            // generateIndividualProfiles is written in two main parts - the
            // setup and then the iteration.  In this part - setup - we fetch
            // the ProjectionInputs associated with this UserGroup and it's parents
            // and store them in an ArrayList.   This are then used to produce one
            // or more 'partial' projections during the iteration part.
            //
            ///////////////////////////////////////////////////////////////////////
            ArrayList userProfileInputs = new ArrayList();
            ArrayList appProfileInputs = new ArrayList();
            userGroupID = ug.getID();

            if ( profileTypes [ PDSDefinitions.PROF_TYPE_USER] )
                userProfileInputs.addAll(
                        m_projectionHelperEJBObject.addParentGroupConfigSets(
                                                            ug,
                                                            PDSDefinitions.PROF_TYPE_USER ) );
            if ( profileTypes [ PDSDefinitions.PROF_TYPE_APPLICATION_REF] )
                appProfileInputs.addAll(
                        m_projectionHelperEJBObject.addParentGroupConfigSets(
                                                            ug,
                                                            PDSDefinitions.PROF_TYPE_APPLICATION_REF ) );


            // Reverse the order
            if (userProfileInputs != null) {
                Collections.reverse(userProfileInputs);
            }
            if (appProfileInputs != null) {
                Collections.reverse(appProfileInputs);
            }

            Collection groupUsers = null;

            try {
                groupUsers = m_userHome.findByUserGroupID( userGroupID );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC675",
                                            "E1028",
                                            new Object [] { ug.getExternalID() } ),
                    ex );

            }


            ///////////////////////////////////////////////////////////////////////
            //
            // This is the iteration part of generateIndividualProfiles.   We
            // iterate over Users in this UserGroup and over those User's associated
            // Devices.
            //
            ///////////////////////////////////////////////////////////////////////
            for ( Iterator iUsers = groupUsers.iterator(); iUsers.hasNext(); ) {
                User user = (User) iUsers.next();

                Collection usersDevices = null;

                try {
                    usersDevices = m_deviceHome.findByUserID( user.getID() );
                }
                catch ( FinderException ex ) {
                    m_ctx.setRollbackOnly();

                    throw new PDSException (
                        collateErrorMessages (  "UC675",
                                                "E1025",
                                                new Object [] { user.getExternalID() } ),
                        ex );
                }

                for ( Iterator iDevices = usersDevices.iterator(); iDevices.hasNext(); ) {
                    Device device = (Device) iDevices.next();

                    ProjectionInput partProjectedUserProfile = null;

                    ///////////////////////////////////////////////////////////////////////
                    //
                    //  we simply delegate the generation of the Device profile
                    //  to the DeviceAdvocate bean.
                    //
                    ///////////////////////////////////////////////////////////////////////
                    if (profileTypes[PDSDefinitions.PROF_TYPE_PHONE]){
                        m_deviceAdvocateEJBObject.generateProfiles(
                                                    device,
                                                    new Integer ( PDSDefinitions.PROF_TYPE_PHONE).toString(),
                                                    projectionAlg,
                                                    false);
                    }

                    if ( profileTypes [ PDSDefinitions.PROF_TYPE_USER] ) {

                        ProjectionInput userInput = null;

                        partProjectedUserProfile =
                            getPartialProjection (  partProjectedProfiles,
                                                    userProfileInputs,
                                                    device,
                                                    PDSDefinitions.PROF_TYPE_USER );

                        userInput = m_projectionHelperEJBObject.getProjectionInput(  user,
                                                                PDSDefinitions.PROF_TYPE_USER );

                        ArrayList finalInputs = new ArrayList();

                        finalInputs.add( partProjectedUserProfile );
                        if ( userInput != null )
                            finalInputs.add( userInput );

                        String projectionClass = null;

                        if ( projectionAlg != null )
                            projectionClass = projectionAlg;

                        m_projectionHelperEJBObject.projectAndPersist(
                                                projectionClass,
                                                device,
                                                PDSDefinitions.PROF_TYPE_USER,
                                                finalInputs);
                    }

                    // Cisco devices have no concept of an Application profile.
                    if (!device.getModel().equals(PDSDefinitions.MODEL_HARDPHONE_CISCO_7940) &&
                        !device.getModel().equals(PDSDefinitions.MODEL_HARDPHONE_CISCO_7960) &&
                        profileTypes[PDSDefinitions.PROF_TYPE_APPLICATION_REF]) {

                        ProjectionInput userInput = null;

                        partProjectedUserProfile =
                            getPartialProjection (
                                    partProjectedProfiles,
                                    appProfileInputs,
                                    device,
                                    PDSDefinitions.PROF_TYPE_APPLICATION_REF );

                        userInput =
                                m_projectionHelperEJBObject.getProjectionInput(
                                        user,
                                        PDSDefinitions.PROF_TYPE_APPLICATION_REF);

                        ArrayList finalInputs = new ArrayList();

                        finalInputs.add( partProjectedUserProfile );

                        if ( userInput != null )
                            finalInputs.add( userInput );

                        String projectionClass = null;

                        if ( projectionAlg != null )
                            projectionClass = projectionAlg;

                        m_projectionHelperEJBObject.projectAndPersist(
                                projectionClass,
                                device,
                                PDSDefinitions.PROF_TYPE_APPLICATION_REF,
                                finalInputs);

                    } // if not Cisco and Application profile required
                }   // user's Devices loop

                currentTotal++;
                m_jobManagerEJBObject.updateJobProgress(
                        jobID,
                        "projected " + currentTotal + " of " +
                            totalUsersToProject + " users." );

            } // users for loop

        } // try
        catch ( PDSException ex ) {
            m_ctx.setRollbackOnly();

            throw new PDSException ( collateErrorMessages ( "UC675") , ex );
        }

        return currentTotal;
    }


    ///////////////////////////////////////////////////////////////////////
    //
    // getPartialProjection does most of the handling of cached partial
    // projections.   Rather than re-project the full list of group inputs
    // for each Device each user has in a user group we cache them.
    //
    // Cached 'partial inputs' are stored for device types * profile types -
    // the reason it is stored as this level is the default projection
    // algorithms may vary between device types.   Users and User Groups have
    // up to one configuration set for User type properties and up to one
    // application ref type settings hence the second level parameter in the
    // cache.
    //
    // getPartialProjection does not maintain state so we have to pass in
    // a MasterDetailsMap (store) whose state is maintained in the calling
    // method.   partialInputs is a Collection of ProjectionInputs gathered
    // in the calling method.   It contains the ProjectionInputs associated
    // with the heirarchy of UserGroups that this User Group belongs to directly
    // or indirectly
    //
    ///////////////////////////////////////////////////////////////////////
    private ProjectionInput getPartialProjection (  MasterDetailsMap store,
                                                    Collection partialInputs,
                                                    Device device,
                                                    int profileType )
        throws PDSException, RemoteException {

        ProjectionInput input = null;

        Integer deviceTypeID = device.getDeviceTypeID();

        Integer wrappedProfileType = new Integer ( profileType );

        if ( store.contains( deviceTypeID, wrappedProfileType ) ) {
            input = (ProjectionInput) store.getDetail(  deviceTypeID,
                                                        wrappedProfileType );
        }
        else {
            String projectionClass = null;

            projectionClass = m_projectionHelperEJBObject.getProjectionClassName( device, profileType );

            input =  m_projectionHelperEJBObject.project(   projectionClass,
                                                            partialInputs,
                                                            deviceTypeID,
                                                            profileType  );

            store.storeDetail(  deviceTypeID,
                                wrappedProfileType,
                                input );

        }

        return input;
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

            m_userHome = (UserHome) initial.lookup("User");
            m_userGroupHome = (UserGroupHome) initial.lookup("UserGroup");
            DeviceAdvocateHome deviceAdvocateHome = (DeviceAdvocateHome)
                    initial.lookup("DeviceAdvocate");

            m_deviceHome = (DeviceHome) initial.lookup("Device");

            ProjectionHelperHome projectionHelperHome = (ProjectionHelperHome)
                    initial.lookup("ProjectionHelper");

            m_configurationSetHome = (ConfigurationSetHome)
                    initial.lookup("ConfigurationSet");

            UserAdvocateHome userAdvocateHome = (UserAdvocateHome) initial.lookup("UserAdvocate");
            ApplicationAdvocateHome applicationAdvocateHome =
                    (ApplicationAdvocateHome) initial.lookup( "ApplicationAdvocate" );

            ApplicationGroupAdvocateHome applicationGroupAdvocateHome =
                    (ApplicationGroupAdvocateHome) initial.lookup( "ApplicationGroupAdvocate" );

            JobManagerHome jobManagerHome = (JobManagerHome) initial.lookup( "JobManager" );

            m_applicationAdvocateEJBObject = applicationAdvocateHome.create();
            m_applicationGroupAdvocateEJBObject = applicationGroupAdvocateHome.create();
            m_userAdvocateEJBObject = userAdvocateHome.create();
            m_deviceAdvocateEJBObject = deviceAdvocateHome.create();
            m_projectionHelperEJBObject = projectionHelperHome.create();
            m_jobManagerEJBObject = jobManagerHome.create();

        } catch (Exception ex) {
            logFatal( ex.toString(), ex );
            throw new EJBException(ex);
        }
    }


    /**
     *  Description of the Method
     */
    public void unsetSessionContext() {
        m_ctx = null;
    }
}
