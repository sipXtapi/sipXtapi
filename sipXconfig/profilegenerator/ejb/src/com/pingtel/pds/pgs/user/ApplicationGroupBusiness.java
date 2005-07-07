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

import com.pingtel.pds.common.PDSException;

import java.rmi.RemoteException;


public interface ApplicationGroupBusiness {

    public static final int MAX_NAME_LEN = 60;

    /**
     * getID returns the PK or ID for this ApplicationGroup
     *
     * @return returns the PK or ID for this ApplicationGroup
     * @see ApplicationGroupBean#id
     */
    Integer getID() throws RemoteException;

    /**
     * getName returns the name of this ApplicationGroup.
     *
     * @return returns the name of this ApplicationGroup.
     * @see ApplicationGroupBean#name
     */
    String getName() throws RemoteException;

    /**
     * setName changes the name associated with the ApplicationGroup.  It
     * performs various validation checks, name can not be null or
     * exceed MAX_NAME_LEN.
     *
     * @param name new name for this ApplicationGroup
     * @throws PDSException is thrown if any of the validation rules are
     * broken.
     * @see ApplicationGroupBean#name
     * @see #MAX_NAME_LEN
     */
    void setName ( String name ) throws PDSException, RemoteException;

    /**
     * getExternalID returns a user-friendly idenitifier for this
     * ApplicationGroup.
     *
     * @return returns a user-friendly identifier.
     */
    String getExternalID () throws RemoteException;

    /**
     * getOrganizationID returns the PK of the Organization that
     * owns this ApplicationGroup.
     *
     * @return PK of the Organization that owns this ApplicationGroup.
     * @see ApplicationGroupBean#organizationID
     */
    Integer getOrganizationID () throws RemoteException;


}