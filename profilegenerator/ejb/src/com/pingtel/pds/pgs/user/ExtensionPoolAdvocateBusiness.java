/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/user/ExtensionPoolAdvocateBusiness.java#4 $
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

import java.rmi.RemoteException;
import java.util.Map;

public interface ExtensionPoolAdvocateBusiness {

    public static final int MAX_NAME_LEN = 80;

    /**
     * createExtensionPool creates a extension pool (a named set of extensions
     * which can then be selected from by users when they want to assign an
     * extension to a user) for an organization.
     *
     * @param organizationID the PK of the organization that you are creating
     * pool for.
     * @param name the name by which the pool will be known (must be unique
     * within an organization).
     * @return the PK for the new pool.
     * @throws PDSException for validation errors and application errors
     * @throws RemoteException for system and remoting errors.
     */
    Integer createExtensionPool ( String organizationID, String name )
            throws PDSException, RemoteException;

    /**
     * deletes an existing Extension Pool.
     * To delete an extension pool you must unassign any extensions in
     * that pool from any users which may have them.   This is to remove
     * the RI constraint.   The rest is simple.
     *
     * @param extensionPoolID the (internal) PK of the pool to delete
     * @throws PDSException for application level errors.
     * @throws RemoteException for system and remoting errors.
     */
    void deleteExtensionPool ( String extensionPoolID )
            throws PDSException, RemoteException;


    /**
     * editExtensionPool allows administrators to change attributes for
     * a given extension pool (currently just the name).
     *
     * @param extensionPoolID the PK of the extension pool that you want
     * to modify
     * @param name the name that you want to change the pool to.
     * @throws PDSException is thrown for application level errors.
     * @throws RemoteException is thrown for RMI Remoting errors.
     */
    void editExtensionPool ( String extensionPoolID, String name )
            throws PDSException, RemoteException;

    /**
     * getNextExtension returns the next available extension number for a given
     * pool.
     *
     * @param extensionPoolID the extension pool ID for the pool that you want
     * to allocate the extension from.
     * @return next available extension.
     * @throws PDSException for application errors.
     * @throws RemoteException is thrown for RMI Remoting errors.
     */
    String getNextExtension ( String extensionPoolID ) throws PDSException, RemoteException;

    /**
     * allocateExtensionsToPool adds a contiguous range or integer extensions to
     * a given extension pool.
     *
     * @param extensionPoolID the ID/PK of the extension pool that the extensions
     * should be added to.
     * @param minExtension the lower end value for the extensions.
     * @param maxExtension the upper end value for the extensions.
     * @throws PDSException application errors.
     * @throws RemoteException is thrown for RMI Remoting errors
     */
    void allocateExtensionsToPool (  String extensionPoolID,
                                            String minExtension,
                                            String maxExtension ) throws PDSException, RemoteException;

    /**
     * deleteExtensionRange deletes the given extensions from the system.  This
     * can only be done if the extensions are not currently assigned to a
     * user if one is then a PDSException is thrown.   Extensions within
     * the range are only deleted if they belong to the extenions pool
     * whose ID is given.
     *
     * @param extensionPoolID is the extension pool's PK that you want to delete the
     * extensions from.
     * @param minExtension the min extension that you want to delete.
     * @param maxExtension the max extension that you want to delete.
     * @throws PDSException for application errors.
     * @throws RemoteException is thrown for RMI Remoting errors
     */
    void deleteExtensionRange (  String extensionPoolID,
                                        String minExtension,
                                        String maxExtension) throws PDSException, RemoteException;

    /**
     * getAllExtensionPoolsDetails returns a Map of ID to Name pairs for all
     * of the extension pools in the system.
     * @return Map of ID to Name pairs
     * @throws PDSException for applicatione errors.
     * @throws RemoteException is thrown for RMI Remoting errors
     */
    Map getAllExtensionPoolsDetails () throws PDSException, RemoteException;
}
