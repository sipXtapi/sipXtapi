/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/sipxchange/datasets/DataSetBuilderBusiness.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */


package com.pingtel.pds.pgs.sipxchange.datasets;


import java.rmi.RemoteException;
import java.util.Collection;

import com.pingtel.pds.common.PDSException;

/**
 * DataSetBuilderBusiness is the Business Interface (pattern) for the DataSetBuilder
 * EJ Bean.  It is used to create the XML databases which get sent to the various
 * servers in a sipxchange installation.
 */
public interface DataSetBuilderBusiness {

    /**
     * generateCredentials produces a complete version of the
     * credential.xml dataset.
     *
     * @throws com.pingtel.pds.common.PDSException is thrown for application (non-critical) errors.
     * @throws java.rmi.RemoteException is thrown for server (critical) errors.
     */
    public void generateCredentials () throws PDSException, RemoteException;

    /**
     * generateAliases produces the FULL version of the aliases database.
     *
     * @exception com.pingtel.pds.common.PDSException is thrown for application (non-critical) errors.
     * @exception java.rmi.RemoteException is thrown for server (critical) errors.
     */
    public void generateAliases () throws PDSException, RemoteException;

    /**
     * generatePermissions produces the FULL version of the permissions database.
     *
     * @exception com.pingtel.pds.common.PDSException is thrown for application (non-critical) errors.
     * @exception java.rmi.RemoteException is thrown for server (critical) errors.
     */
    //public void generatePermissions () throws PDSException, RemoteException;

    /**
     * generateExtensions produces the FULL version of the extensions database.
     *
     * @exception com.pingtel.pds.common.PDSException is thrown for application (non-critical) errors.
     * @exception java.rmi.RemoteException is thrown for server (critical) errors.
     */
    public void generateExtensions () throws PDSException, RemoteException;

    /**
     * generateAuthExceptions produces a complete version of the
     * authexception.xml dataset.
     *
     * @param changePoints are the location(s) in the User/UserGroup hierarchy.  If the
     * forwarding settings or permission change occurred for just one user then just pass
     * in that User.  If an entire UserGroup (and their subgroups) need changing then
     * pass that in.   Any number or combination of the above can be passed in.
     * @throws com.pingtel.pds.common.PDSException is thrown for applcation errors.
     * @throws RemoteException is thrown for server (critical) errors.
     */
    void generateAuthExceptions (Collection changePoints) throws PDSException, RemoteException;

    /**
     * generatePermissions builds the Permissions export dataset and sends it to
     * the other SIPxchange servers.
     *
     * @param changePoints are the location(s) in the User/UserGroup hierarchy.  If the
     * forwarding settings or permission change occurred for just one user then just pass
     * in that User.  If an entire UserGroup (and their subgroups) need changing then
     * pass that in.   Any number or combination of the above can be passed in.
     * @throws com.pingtel.pds.common.PDSException is thrown for applcation errors.
     * @throws RemoteException is thrown for server (critical) errors.
     */
    void generatePermissions (Collection changePoints) throws PDSException, RemoteException;

}