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

import java.rmi.RemoteException;

import org.jdom.Element;

import com.pingtel.pds.common.PDSException;

public interface ApplicationGroupAdvocateBusiness {
    /**
     * createApplicationGroup implements the create application set use
     * case.
     * @param name the name of the new application set.
     * @param organizationID the PK of the Organization (entity) that the
     * application set should be created in.
     * @return EJBObject of the new Application Set.
     * @throws PDSException is thrown for application level errors
     */
    ApplicationGroup createApplicationGroup (    String name,
                                                        String organizationID )
        throws PDSException, RemoteException;

    /**
     * deleteApplicationGroup implements the delete application set use
     * case.
     *
     * @param id the PK of the Application Set that you wish to delete.
     * @throws PDSException for application level errors.
     */
    void deleteApplicationGroup ( String id ) throws PDSException, RemoteException;

    /**
     * editApplicationGroup implements the edit Application Set use case.
     *
     * @param applicationsetid the PK for the Application Set that you wish
     * to edit.
     * @param name the name that you wish to change the Application Set's name
     * to.
     * @throws PDSException
     */
    void editApplicationGroup ( String applicationsetid,
                                    String name )
        throws PDSException, RemoteException;

    /**
     * assignUserAndApplicationGroup assigns a well-known (previously created)
     * Application Set to a User.
     *
     * @param userID the PK for the User that you wish to have the
     * Application Set assigned to.
     * @param applicationGroupID the PK of the Application Set that you wish
     * to assign to the User.
     * @throws PDSException is thrown for application level errors.
     */
    void assignUserAndApplicationGroup ( String userID,
                                                String applicationGroupID )
        throws PDSException, RemoteException;

    /**
     * assignUserGroupAndApplicationGroup assigns a well-known (previously created)
     * Application Set to a User Group.
     * @param userGroupID the PK of the User Group who you want to associate the
     * Application Set to.
     * @param applicationGroupID the PK of the Application Set that you wish
     * to assign to the User Group.
     * @throws PDSException is thrown for application level errors.
     */
    void assignUserGroupAndApplicationGroup ( String userGroupID,
                                                        String applicationGroupID )
        throws PDSException, RemoteException;

    /**
     * unassignUserAndApplicationGroup unassigns a User and an Application Set.
     *
     * @param userID the PK for the User that you wish to have the
     * Application Set unassigned from.
     * @param applicationGroupID the PK of the Application Set that you wish
     * to unassign from the User Group.
     * @throws PDSException is thrown for application level errors.
     */
    void unassignUserAndApplicationGroup (   String userID,
                                                    String applicationGroupID )
        throws PDSException, RemoteException;

    /**
     * unassignUserAndApplicationGroup unassigns a User Group and an Application Set.
     *
     * @param userGroupID the PK for the User Group that you wish to have the
     * Application Set unassigned from.
     * @param applicationGroupID the PK of the Application Set that you wish
     * to unassign from the User Group.
     * @throws PDSException is thrown for application level errors.
     */
    void unassignUserGroupAndApplicationGroup ( String userGroupID,
                                                        String applicationGroupID )
        throws PDSException, RemoteException;

    /**
     * assignApplicationToApplicationGroup assigns an Application to an
     * Application Set.
     * @param applicationID the PK of the Application that you wish to
     * assign to the Application Set.
     * @param applicationGroupID the PK of the Application Set that you wish
     * to add the Application to.
     * @throws PDSException is thrown for application level errors.
     */
    void assignApplicationToApplicationGroup (   String applicationID,
                                                        String applicationGroupID )
        throws PDSException, RemoteException;

    /**
     * unassignApplicationToApplicationGroup assigns an Application to an
     * Application Set.
     * @param applicationID the PK of the Application that you wish to
     * unassign to the Application Set from.
     * @param applicationGroupID the PK of the Application Set that you wish
     * to remove the Application from.
     * @throws PDSException is thrown for application level errors.
     */
    void unassignApplicationFromApplicationGroup (   String applicationID,
                                                            String applicationGroupID )
        throws PDSException, RemoteException;

    /**
     * unassignAllUsersApplicationGroups unassigns all Application Set
     * which are assigned to a given User.   This is a convienience method
     * used when deleting a User.
     *
     * @param userID the PK of the User who Application Sets you wish to
     * delete.
     * @throws PDSException is thrown for application level errors.
     */
    void unassignAllUsersApplicationGroups ( String userID ) throws PDSException, RemoteException;

    /**
     * unassignAllUserGroupsApplicationGroups unassigns all Application Sets
     * which are assigned to a given User Group.   This is a convienience method
     * used when deleting a User Group.
     *
     * @param userGroupID the PK of the User Group who Application Sets you wish to
     * delete.
     * @throws PDSException is thrown for application level errors.
     */
    void unassignAllUserGroupsApplicationGroups ( Integer userGroupID ) throws PDSException, RemoteException;

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
    Element evaluateApplicationSetContent ( Integer applicationSetID )
        throws PDSException, RemoteException;


}