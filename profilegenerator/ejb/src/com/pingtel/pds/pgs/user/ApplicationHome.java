/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/user/ApplicationHome.java#4 $
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
import java.util.Collection;

import javax.ejb.CreateException;
import javax.ejb.EJBHome;
import javax.ejb.FinderException;


public interface ApplicationHome extends EJBHome {

    /**
     * Standard EJB implementation of a create method.
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
     * @return PK of new Application.
     * @throws CreateException for application level errors.
     * @throws RemoteException for system/remoting errors.
     * @throws PDSException for validation errors
     */
    public Application create ( String name,
                                Integer organizationID,
                                Integer deviceTypeID,
                                Integer refPropertyID,
                                String url,
                                String description )
        throws CreateException, RemoteException, PDSException;

    public Collection findByOrganizationID ( Integer organizationID )
        throws FinderException, RemoteException;

    public Collection findAll ()
        throws FinderException, RemoteException;

    public Application findByPrimaryKey ( Integer pk )
        throws FinderException, RemoteException;

    public Collection findByApplicationGroupID ( Integer applicationGroupID )
        throws FinderException, RemoteException;

    /**
     * finds all Applications for a given URL.   Note Application URLs
     * should be unique in the Config Server so only one EJBObject should
     * be in the returned Collection.
     *
     * @param url URL that you want to serach with.
     * @return a Collection of EJBObjects for the Application(s).
     * @throws FinderException is thrown for non-system errors.
     * @throws RemoteException is thrown for system or remoting errors.
     */
    public Collection findByUrl(String url) throws FinderException, RemoteException;

}
