/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/user/ApplicationBusiness.java#4 $
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


public interface ApplicationBusiness {

    public static final int MAX_NAME_LEN = 60;
    public static final int MAX_URL_LEN = 200;
    public static final int MAX_DESCRIPTION_LEN = 512;

    /**
     * getID returns the PK or Id of this Application.
     *
     * @return id of this Application
     */
    Integer getID() throws RemoteException;

    /**
     * getName returns the name of this Application.
     *
     * @return name of this Application.
     */
    String getName() throws RemoteException;

    /**
     * setName changes the name of the Application.   It
     * performs various validation checks on the argument such
     * as that it can not be null or exceed MAX_NAME_LEN
     *
     * @param name the new name to assign to this Application
     * @throws PDSException is thrown if the name arguement
     * breaks any of the validation rules.
     *
     */
    void setName (String name) throws PDSException, RemoteException;

    /**
     * getRefPropertyID returns the RefPropertyID of the Application.
     * Each (Pingtel) Application has a RefPropertyID so as to allow
     * use to do specialized projection for different types of applications.
     *
     * @return PK of the RefProperty which this Application corresponds to.
     */
    Integer getRefPropertyID () throws RemoteException;

    /**
     * getURL returns the URL of this Application
     * @return the URL of this Application.
     */
    String getURL () throws RemoteException;

    /**
     * setURL lets you modify the URL of this Application.
     * There are various validation rules for Application URLs
     * such as they can not be null or be longer than MAX_URL_LEN
     * or duplicates of existing URLs in the Config Server.
     *
     * @param url new URL for the Application.
     * @throws PDSException is thrown  if any of the validation
     * rules are broken.
     */
    void setURL (String url) throws PDSException, RemoteException;

    /**
     * getOrganizationID returns the PK of the Organization that this Application
     * belongs to.
     *
     * @return PK of the Application this Application belongs to.
     */
    Integer getOrganizationID () throws RemoteException;

    /**
     * getDeviceTypeID returns the PK of the DeviceType that this Application runs
     * on.  This is not really used as we currently only support Pingtel
     * applications.
     *
     * @return PK of the DeviceType that this Application can execute on.
     */
    Integer getDeviceTypeID () throws RemoteException;

    /**
     * getDescription returns a description of this Application if one
     * has be given to it.
     *
     * @return returns the description of the Application.
     */
    String getDescription () throws RemoteException;

    /**
     * setDescription allows you to change the description for this Application.
     *
     * @param description new description for the Application.
     * @throws PDSException is thrown if the description is too long
     * (MAX_DESCRIPTION_LEN) or null.
     */
    void setDescription (String description) throws PDSException, RemoteException;

    /**
     * getExternalID returns a friendly identifier for this Application.
     *
     * @return friendly identifier for this Application.
     */
    String getExternalID () throws RemoteException;

}