/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/user/ApplicationGroupAdvocateBean.java#5 $
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
import javax.naming.NamingException;

import java.io.ByteArrayInputStream;
import java.rmi.RemoteException;
import java.sql.SQLException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;

import org.jdom.CDATA;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.common.ejb.JDBCAwareEJB;
import com.pingtel.pds.pgs.common.ejb.InternalToExternalIDTranslator;
import com.pingtel.pds.pgs.profile.RefProperty;
import com.pingtel.pds.pgs.profile.RefPropertyHome;


public class ApplicationGroupAdvocateBean extends JDBCAwareEJB
    implements SessionBean, ApplicationGroupAdvocateBusiness {


    /**
     * createApplicationGroup implements the create application set use
     * case.
     * @param name the name of the new application set.
     * @param organizationID the PK of the Organization (entity) that the
     * application set should be created in.
     * @return EJBObject of the new Application Set.
     * @throws PDSException is thrown for application level errors
     */
    public ApplicationGroup createApplicationGroup (    String name,
                                                        String organizationID )
        throws PDSException {

        ApplicationGroup ag = null;

        try {
            ag = m_agHome.create( name, new Integer ( organizationID ) );
            logTransaction ( m_ctx, "created application set  " + ag.getExternalID() );
        }
        catch ( CreateException ex ) {
            m_ctx.setRollbackOnly();

            throw new PDSException (
                collateErrorMessages (  "UC320",
                                        "E220",
                                        new Object [] { name } ),
                ex );
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            throw new EJBException (
                collateErrorMessages (  "UC320",
                                        "E4047",
                                        new Object [] { name } ) );
        }

        return ag;
    }


    /**
     * deleteApplicationGroup implements the delete application set use
     * case.
     *
     * @param id the PK of the Application Set that you wish to delete.
     * @throws PDSException for application level errors.
     */
    public void deleteApplicationGroup ( String id ) throws PDSException {

        ApplicationGroup ag = null;
        String externalID = null;

        try {
            try {
                ag = m_agHome.findByPrimaryKey( new Integer ( id ) );
                externalID = ag.getExternalID();
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC330",
                                            "E1036",
                                            new Object [] { id } ),
                    ex );
            }

            try {
                executePreparedUpdate(  "   DELETE FROM USER_AG_ASSOC " +
                                        "   WHERE AG_ID = ? ",
                                        new Object [] { id } );

                executePreparedUpdate(  "   DELETE FROM USER_GROUP_AG_ASSOC " +
                                        "   WHERE AG_ID = ? ",
                                        new Object [] { id } );
            }
            catch ( SQLException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC330",
                                            "E5009",
                                            new Object [] { ag.getExternalID() } ),
                    ex );
            }

            try {
                executePreparedUpdate(  "   DELETE FROM APP_AG_ASSOC " +
                                        "   WHERE AG_ID = ? ",
                                        new Object [] { id } );
            }
            catch ( SQLException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC330",
                                            "E5010",
                                            new Object [] { ag.getExternalID() } ),
                    ex );
            }

            try {
                ag.remove();
            }
            catch ( RemoveException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC330",
                                            "E6016",
                                            new Object [] { ag.getExternalID() } ),
                    ex );
            }

            logTransaction ( m_ctx, "deleted application set  " + externalID );
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            String external = null;

            try { external = ag.getExternalID(); } catch ( RemoteException rex ) {}

            throw new EJBException (
                collateErrorMessages (  "UC320",
                                        "E4048",
                                        new Object [] { external == null ?
                                                            id :
                                                            external } ) );

        }
    }


    /**
     * editApplicationGroup implements the edit Application Set use case.
     *
     * @param applicationsetid the PK for the Application Set that you wish
     * to edit.
     * @param name the name that you wish to change the Application Set's name
     * to.
     * @throws PDSException
     */
    public void editApplicationGroup ( String applicationsetid, String name )
        throws PDSException {

        ApplicationGroup ag = null;
        try {
            try {
                ag = m_agHome.findByPrimaryKey( new Integer ( applicationsetid ) );
            }
            catch (FinderException ex) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC335",
                                            "E1036",
                                            new Object [] { applicationsetid } ),
                    ex );
            }
            if (name != null && !name.equals(ag.getName())) {
                ag.setName(name);
            }
        }
        catch (RemoteException ex) {
            logFatal ( ex.toString(), ex );

            throw new EJBException (
                collateErrorMessages (  "UC335",
                                        "E4070",
                                        new Object [] { applicationsetid } ) );
        }
        catch (PDSException e) {
            m_ctx.setRollbackOnly();

            throw new PDSException (collateErrorMessages ("UC335"), e);
        }

    }


    /**
     * assignUserAndApplicationGroup assigns an Application Set to a User.
     *
     * @param userID the PK for the User that you wish to have the
     * Application Set assigned to.
     * @param applicationGroupID the PK of the Application Set that you wish
     * to assign to the User.
     * @throws PDSException is thrown for application level errors.
     */
    public void assignUserAndApplicationGroup ( String userID,
                                                String applicationGroupID )
        throws PDSException {

        User user = null;
        ApplicationGroup ag = null;

        try {
            try {
                user = m_userHome.findByPrimaryKey( userID ) ;
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC690",
                                            "E1026",
                                            new Object [] { userID } ),
                    ex );
            }

            try {
                ag =
                    m_agHome.findByPrimaryKey( new Integer ( applicationGroupID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC690",
                                            "E1036",
                                            new Object [] { applicationGroupID } ),
                    ex );
            }

            try {
                executePreparedUpdate ( "INSERT INTO USER_AG_ASSOC ( USRS_ID, AG_ID ) " +
                                        "VALUES ( ?, ?)",
                                        new Object [] { userID, applicationGroupID } );

            }
            catch ( SQLException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC690",
                                            "E5011",
                                            new Object [] { ag.getExternalID(),
                                                            user.getExternalID() } ),
                    ex );
            }

            String externalUserID =
                InternalToExternalIDTranslator.getInstance().translate( m_userHome,
                                                                        userID );

            String externalAGID =
                InternalToExternalIDTranslator.getInstance().translate( m_agHome,
                                                                        Integer.valueOf( applicationGroupID ) );

            logTransaction ( m_ctx, " assigned application set " + externalAGID +
                " to user " + externalUserID );

        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            String userExternal = null;
            String agExternal = null;

            try {
                userExternal = user.getExternalID();
                agExternal = ag.getExternalID();
            } catch ( RemoteException rex ) {}

            throw new EJBException (
                collateErrorMessages (  "UC690",
                                        "E4049",
                                        new Object [] { agExternal == null ?
                                                            applicationGroupID :
                                                            agExternal,
                                                        userExternal == null ?
                                                            userID :
                                                            userExternal } ) );
        }
    }


    /**
     * assignUserGroupAndApplicationGroup assigns an Application Set to a User Group.
     * @param userGroupID the PK of the User Group who you want to associate the
     * Application Set to.
     * @param applicationGroupID the PK of the Application Set that you wish
     * to assign to the User Group.
     * @throws PDSException is thrown for application level errors.
     */
    public void assignUserGroupAndApplicationGroup ( String userGroupID,
                                                        String applicationGroupID )
        throws PDSException {

        UserGroup userGroup = null;
        ApplicationGroup ag = null;

        try {
            try {
                userGroup =
                    m_userGroupHome.findByPrimaryKey( new Integer ( userGroupID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC695",
                                            "E1019",
                                            new Object [] { userGroupID } ),
                    ex );
            }

            try {
                ag =
                    m_agHome.findByPrimaryKey( new Integer ( applicationGroupID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC695",
                                            "E1036",
                                            new Object [] { applicationGroupID } ),
                    ex );
            }

            try {
                executePreparedUpdate ( "INSERT INTO USER_GROUP_AG_ASSOC ( UG_ID, AG_ID ) " +
                                        "VALUES ( ?, ?)",
                                        new Object [] { userGroupID, applicationGroupID } );

            }
            catch ( SQLException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC695",
                                            "E5011",
                                            new Object [] { ag.getExternalID(),
                                                            userGroup.getExternalID() } ),
                    ex );
            }

            String externalUserGroupID =
                InternalToExternalIDTranslator.getInstance().translate( m_userGroupHome,
                                                                        Integer.valueOf( userGroupID ) );

            String externalAGID =
                InternalToExternalIDTranslator.getInstance().translate( m_agHome,
                                                                        Integer.valueOf( applicationGroupID ) );

            logTransaction ( m_ctx, " assigned application set " + externalAGID +
                " to user group " + externalUserGroupID );

        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            String ugExternal = null;
            String agExternal = null;

            try {
                ugExternal = userGroup.getExternalID();
                agExternal = ag.getExternalID();
            } catch ( RemoteException rex ) {}

            throw new EJBException (
                collateErrorMessages (  "UC695",
                                        "E4050",
                                        new Object [] { agExternal == null ?
                                                            applicationGroupID :
                                                            agExternal,
                                                        ugExternal == null ?
                                                            userGroupID :
                                                            ugExternal } ) );
        }
    }


    /**
     * unassignUserAndApplicationGroup unassigns a User and an Application Set.
     *
     * @param userID the PK for the User that you wish to have the
     * Application Set unassigned from.
     * @param applicationGroupID the PK of the Application Set that you wish
     * to unassign from the User.
     * @throws PDSException is thrown for application level errors.
     */
    public void unassignUserAndApplicationGroup (   String userID,
                                                    String applicationGroupID )
        throws PDSException {

        User user = null;
        ApplicationGroup ag = null;

        try {
            try {
                user = m_userHome.findByPrimaryKey( userID ) ;
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC698",
                                            "E1026",
                                            new Object [] { userID } ),
                    ex );
            }

            try {
                ag =
                    m_agHome.findByPrimaryKey( new Integer ( applicationGroupID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC698",
                                            "E1036",
                                            new Object [] { applicationGroupID } ),
                    ex );
            }

            try {
                executePreparedUpdate ( "DELETE FROM USER_AG_ASSOC " +
                                        "WHERE  usrs_id = ? " +
                                        "AND    ag_id = ? ",
                                        new Object [] { userID, applicationGroupID } );

            }
            catch ( SQLException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC698",
                                            "E5012",
                                            new Object [] { ag.getExternalID(),
                                                            user.getExternalID() } ),
                    ex );
            }

            String externalUserID =
                InternalToExternalIDTranslator.getInstance().translate( m_userHome,
                                                                        userID );

            String externalAGID =
                InternalToExternalIDTranslator.getInstance().translate( m_agHome,
                                                                        Integer.valueOf( applicationGroupID ) );

            logTransaction ( m_ctx, " unassigned application set " + externalAGID +
                " from user " + externalUserID );
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            String userExternal = null;
            String agExternal = null;

            try {
                userExternal = user.getExternalID();
                agExternal = ag.getExternalID();
            } catch ( RemoteException rex ) {}

            throw new EJBException (
                collateErrorMessages (  "UC698",
                                        "E4051",
                                        new Object [] { agExternal == null ?
                                                            applicationGroupID :
                                                            agExternal,
                                                        userExternal == null ?
                                                            userID :
                                                            userExternal } ) );
        }

    }


    /**
     * unassignUserAndApplicationGroup unassigns a User Group and an Application Set.
     *
     * @param userGroupID the PK for the User Group that you wish to have the
     * Application Set unassigned from.
     * @param applicationGroupID the PK of the Application Set that you wish
     * to unassign from the User Group.
     * @throws PDSException is thrown for application level errors.
     */
    public void unassignUserGroupAndApplicationGroup ( String userGroupID,
                                                        String applicationGroupID )
        throws PDSException {

        UserGroup userGroup = null;
        ApplicationGroup ag = null;

        try {
            try {
                userGroup =
                    m_userGroupHome.findByPrimaryKey( new Integer ( userGroupID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC699",
                                            "E1019",
                                            new Object [] { userGroupID } ),
                    ex );
            }

            try {
                ag =
                    m_agHome.findByPrimaryKey( new Integer ( applicationGroupID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC699",
                                            "E1036",
                                            new Object [] { applicationGroupID } ),
                    ex );
            }


            try {
                executePreparedUpdate ( "DELETE FROM USER_GROUP_AG_ASSOC " +
                                        "WHERE  ug_id = ? " +
                                        "AND    ag_id = ? ",
                                        new Object [] { userGroupID, applicationGroupID } );

            }
            catch ( SQLException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC698",
                                            "E5013",
                                            new Object [] { ag.getExternalID(),
                                                            userGroup.getExternalID() } ),
                    ex );
            }

            String externalUserGroupID =
                InternalToExternalIDTranslator.getInstance().translate( m_userGroupHome,
                                                                        Integer.valueOf( userGroupID ) );

            String externalAGID =
                InternalToExternalIDTranslator.getInstance().translate( m_agHome,
                                                                        Integer.valueOf( applicationGroupID ) );

            logTransaction ( m_ctx, " unassigned application set " + externalAGID +
                " from user group " + externalUserGroupID );
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            String ugExternal = null;
            String agExternal = null;

            try {
                ugExternal = userGroup.getExternalID();
                agExternal = ag.getExternalID();
            } catch ( RemoteException rex ) {}

            throw new EJBException (
                collateErrorMessages (  "UC695",
                                        "E4052",
                                        new Object [] { agExternal == null ?
                                                            applicationGroupID :
                                                            agExternal,
                                                        ugExternal == null ?
                                                            userGroupID :
                                                            ugExternal } ) );
        }
    }


    /**
     * assignApplicationToApplicationGroup assigns an Application to an
     * Application Set.
     * @param applicationID the PK of the Application that you wish to
     * assign to the Application Set.
     * @param applicationGroupID the PK of the Application Set that you wish
     * to add the Application to.
     * @throws PDSException is thrown for application level errors.
     */
    public void assignApplicationToApplicationGroup (   String applicationID,
                                                        String applicationGroupID )
        throws PDSException {

        Application application = null;
        ApplicationGroup ag = null;

        try {
            try {
                ag =
                    m_agHome.findByPrimaryKey( new Integer ( applicationGroupID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC340",
                                            "E1036",
                                            new Object [] { applicationGroupID } ),
                    ex );
            }

            try {
                application =
                    m_appHome.findByPrimaryKey ( new Integer ( applicationID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC340",
                                            "E1037",
                                            new Object [] { applicationID } ),
                    ex );
            }


            try {
                executePreparedUpdate ( "INSERT INTO APP_AG_ASSOC ( apps_id, ag_id ) " +
                                        "VALUES ( ?, ? )",
                                        new Object [] { applicationID,
                                                        applicationGroupID } );
            }
            catch ( SQLException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC340",
                                            "E5014",
                                            new Object [] { application.getExternalID(),
                                                            ag.getExternalID() } ),
                    ex );
            }

            String externalApplicationID =
                InternalToExternalIDTranslator.getInstance().translate( m_appHome,
                                                                        Integer.valueOf( applicationID ) );

            String externalAGID =
                InternalToExternalIDTranslator.getInstance().translate( m_agHome,
                                                                        Integer.valueOf( applicationGroupID ) );

            logTransaction ( m_ctx, " assigned application  " + externalApplicationID +
                " to application set " + externalAGID );

        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            String appExternal = null;
            String agExternal = null;

            try {
                appExternal = application.getExternalID();
                agExternal = ag.getExternalID();
            } catch ( Throwable rex ) {}

            throw new EJBException (
                collateErrorMessages (  "UC340",
                                        "E4053",
                                        new Object [] { appExternal == null ?
                                                            applicationID :
                                                            appExternal,
                                                        agExternal == null ?
                                                            applicationGroupID :
                                                            agExternal } ) );

        }

    }


    /**
     * unassignApplicationToApplicationGroup assigns an Application to an
     * Application Set.
     * @param applicationID the PK of the Application that you wish to
     * unassign to the Application Set from.
     * @param applicationGroupID the PK of the Application Set that you wish
     * to remove the Application from.
     * @throws PDSException is thrown for application level errors.
     */
    public void unassignApplicationFromApplicationGroup (   String applicationID,
                                                            String applicationGroupID )
        throws PDSException {

        Application application = null;
        ApplicationGroup ag = null;

        try {
            try {
                ag =
                    m_agHome.findByPrimaryKey( new Integer ( applicationGroupID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC350",
                                            "E1036",
                                            new Object [] { applicationGroupID } ),
                    ex );
            }

            try {
                application =
                    m_appHome.findByPrimaryKey ( new Integer ( applicationID ) );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC350",
                                            "E1037",
                                            new Object [] { applicationGroupID } ),
                    ex );
            }


            try {
                executePreparedUpdate ( "DELETE FROM APP_AG_ASSOC " +
                                        "WHERE  apps_id = ? " +
                                        "AND    ag_id = ? ",
                                        new Object [] { applicationID,
                                                        applicationGroupID } );
            }
            catch ( SQLException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "UC350",
                                            "E5015",
                                            new Object [] { application.getExternalID(),
                                                            ag.getExternalID() } ),
                    ex );
            }

            String externalApplicationID =
                InternalToExternalIDTranslator.getInstance().translate( m_appHome,
                                                                        Integer.valueOf( applicationID ) );

            String externalAGID =
                InternalToExternalIDTranslator.getInstance().translate( m_agHome,
                                                                        Integer.valueOf( applicationGroupID ) );

            logTransaction ( m_ctx, " unassigned application  " + externalApplicationID +
                " from application set " + externalAGID );
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            String appExternal = null;
            String agExternal = null;

            try {
                appExternal = application.getExternalID();
                agExternal = ag.getExternalID();
            } catch ( RemoteException rex ) {}

            throw new EJBException (
                collateErrorMessages (  "UC350",
                                        "E4053",
                                        new Object [] { appExternal == null ?
                                                            applicationID :
                                                            appExternal,
                                                        agExternal == null ?
                                                            applicationGroupID :
                                                            agExternal } ) );

        }

    }


    /**
     * unassignAllUsersApplicationGroups unassigns all Application Set
     * which are assigned to a given User.   This is a convienience method
     * used when deleting a User.
     *
     * @param userID the PK of the User who Application Sets you wish to
     * delete.
     * @throws PDSException is thrown for application level errors.
     */
    public void unassignAllUsersApplicationGroups ( String userID ) throws PDSException {

        String userExt = null;

        try {
            User user = null;

            try {
                user = m_userHome.findByPrimaryKey( userID );
                userExt = user.getExternalID();
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "E1026",
                                            new Object [] { userID } ),
                    ex );
            }

            try {
                executePreparedUpdate ( "DELETE FROM USER_AG_ASSOC " +
                                            "WHERE  usrs_id = ? ",
                                            new Object [] { userID } );
            }
            catch ( SQLException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "E6022",
                                            new Object [] { userExt } ),
                    ex );
            }
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            throw new EJBException (
                collateErrorMessages (  "E4074",
                                        new Object [] { userID } ) );
        }
    }


    /**
     * unassignAllUserGroupsApplicationGroups unassigns all Application Sets
     * which are assigned to a given User Group.   This is a convienience method
     * used when deleting a User Group.
     *
     * @param userGroupID the PK of the User Group who Application Sets you wish to
     * delete.
     * @throws PDSException is thrown for application level errors.
     */
    public void unassignAllUserGroupsApplicationGroups ( Integer userGroupID ) throws PDSException {

        String userGroupExt = null;

        try {
            UserGroup userGroup = null;

            try {
                userGroup = m_userGroupHome.findByPrimaryKey( userGroupID );
                userGroupExt = userGroup.getExternalID();
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "E1019",
                                            new Object [] { userGroupID } ),
                    ex );
            }

            try {
                executePreparedUpdate ( "DELETE FROM USER_GROUP_AG_ASSOC " +
                                            "WHERE  ug_id = ? ",
                                            new Object [] { userGroupID } );
            }
            catch ( SQLException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages (  "E6023",
                                            new Object [] { userGroupExt } ),
                    ex );
            }
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            throw new EJBException (
                collateErrorMessages (  "E4075",
                                        new Object [] { userGroupID } ) );
        }

    }


    /**
     * evaluateApplicationSetContent is used to produce the Configuration Set
     * type XML content for a given Application Set.  We do lazy evaluation
     * of Application Sets contents otherwise we would have to put in markup in
     * every entity's (User, User Group) Configuration Set who has that Application
     * Set assigned to them.   Apart from the obvious database bloat it would be
     * a update nighmare everytime a user assigned/unassigned an Application to
     * an entity or when they deleted the Application Set.
     *
     * @param applicationSetID the PK of the Application Set whose contents you
     * which to calculate.
     * @return JDOM Element containing the markup for the Application Set.  It is
     * used as part of projection.
     * @throws PDSException is thrown for application level errors.
     */
    public Element evaluateApplicationSetContent ( Integer applicationSetID )
        throws PDSException {

        HashMap cachedDefinitions = new HashMap ();
        Element set = new Element ( "applicationset" );

        try {
            Collection assignedApps = null;

            try {
                assignedApps = m_appHome.findByApplicationGroupID( applicationSetID );
            }
            catch ( FinderException ex ) {
                m_ctx.setRollbackOnly();

                throw new PDSException (
                    collateErrorMessages ( "E1044",
                                            null ),
                    ex );
            }

            for ( Iterator iApp = assignedApps.iterator(); iApp.hasNext(); ) {
                Application application = (Application) iApp.next();
                Integer rpID = application.getRefPropertyID();
                if ( !cachedDefinitions.containsKey( rpID ) ) {
                    try {
                        RefProperty rp = m_rpHome.findByPrimaryKey( rpID );
                        Document doc =
                            m_saxBuilder.build(
                                new ByteArrayInputStream (
                                    rp.getContent().getBytes() ) );

                        Element definition = doc.getRootElement();
                        cachedDefinitions.put( rpID, definition );
                        /*
                        <definition name="USER" mobility="false" cardinality="0..N" description="">
                           <value name="URL" type="string" index="true" description="" />
                        </definition>
                        */
                    }
                    catch ( FinderException ex ) {
                        m_ctx.setRollbackOnly();

                        throw new PDSException (
                            collateErrorMessages ( "E1029",
                                                    new Object [] { applicationSetID } ),
                            ex );

                    }
                    catch ( JDOMException ex ) {
                        throw new RemoteException ( ex.toString() );
                    }
                    catch ( java.io.IOException ex ) {
                        throw new RemoteException ( ex.toString() );
                    }
                } // if

                Element definition = (Element) cachedDefinitions.get( rpID );

                Element property =
                    new Element ( definition.getAttributeValue( "name" ));

                set.addContent( property );
                property.setAttribute( "ref_property_id", rpID.toString() );
                String cardinality = definition.getAttributeValue( "cardinality" );

                if ( cardinality.equals( "1..N" ) || cardinality.equals( "0..N" ) ) {
                    //property.setAttribute(  "id",
                    //                        Base64.encodeString( application.getURL() ) );
                    byte [] replace = application.getURL().getBytes();
                    StringBuffer target = new StringBuffer();
                    for ( int count = 0; count < replace.length; ++count )
                        target.append( Integer.toHexString( (int) replace[count] ) );

                    property.setAttribute(  "id", target.toString() );

                }

                Element propertyChild =
                    new Element ( definition.getAttributeValue( "name" ) );

                property.addContent( propertyChild );
                String valueName =
                    definition.getChild( "value" ).getAttributeValue( "name" );

                Element value = new Element ( valueName );
                propertyChild.addContent( value );
                value.addContent( new CDATA ( application.getURL() ) );
            }

            return set;
        }
        catch ( RemoteException ex ) {
            logFatal ( ex.toString(), ex );

            throw new EJBException (
                collateErrorMessages (  "E4057",
                                        null ),
                ex );
        }

    }



    /** Standard Boiler Plate Session Bean Method Implementation */
    public void ejbCreate () {}

    /** Standard Boiler Plate Session Bean Method Implementation */
    public void ejbRemove () {}

    /** Standard Boiler Plate Session Bean Method Implementation */
    public void ejbActivate () {}

    /** Standard Boiler Plate Session Bean Method Implementation */
    public void ejbPassivate () {}

    /** Standard Boiler Plate Session Bean Method Implementation */
    public void setSessionContext ( SessionContext ctx ) {

        m_ctx = ctx;

        try {
            Context initial = new InitialContext();

            m_agHome = (ApplicationGroupHome) initial.lookup( "ApplicationGroup" );
            m_userHome = (UserHome) initial.lookup( "User" );
            m_userGroupHome = (UserGroupHome) initial.lookup( "UserGroup" );
            m_appHome = (ApplicationHome) initial.lookup( "Application" );
            m_rpHome = (RefPropertyHome) initial.lookup( "RefProperty" );
        }
        catch ( NamingException ex ) {
            logFatal ( ex.toString(), ex );
            throw new EJBException ( ex.toString() );
        }
    }

    /** Standard Boiler Plate Implementation */
    public void unsetSessionContext () {
        m_ctx = null;
    }

    /** The Session Context object */
    private SessionContext m_ctx;

    // Home interface references


    private ApplicationGroupHome m_agHome;
    private ApplicationHome m_appHome;
    private UserHome m_userHome;
    private UserGroupHome m_userGroupHome;
    private RefPropertyHome m_rpHome;

    private SAXBuilder m_saxBuilder = new SAXBuilder();

}
