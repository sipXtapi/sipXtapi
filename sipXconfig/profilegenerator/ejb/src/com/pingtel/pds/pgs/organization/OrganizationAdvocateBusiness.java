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

package com.pingtel.pds.pgs.organization;

import java.rmi.RemoteException;

import com.pingtel.pds.common.PDSException;

public interface OrganizationAdvocateBusiness {

    public Organization install (   String organizationName,
                                    String stereotype,
                                    String dnsDomain,
                                    String superAdminPassword )
        throws PDSException, RemoteException;
    /**
     *  Description of the Method
     *
     *@param  name                  Description of the Parameter
     *@param  parentOrganizationID  Description of the Parameter
     *@return                       Description of the Return Value
     *@exception  PDSException     Description of the Exception
     *@exception  RemoteException   Description of the Exception
     */
    public Organization createOrganization( String name,
                                            String parentOrganizationID,
                                            String stereotype,
                                            String dnsDomain)
             throws PDSException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  organizationID       Description of the Parameter
     *@exception  PDSException    Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public void deleteOrganization(String organizationID)
             throws PDSException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  organizationID        Description of the Parameter
     *@param  name                  Description of the Parameter
     *@param  parentOrganizationID  Description of the Parameter
     *@exception  PDSException     Description of the Exception
     *@exception  RemoteException   Description of the Exception
     */
    public void editOrganization(   String organizationID,
                                    String name,
                                    String parentOrganizationID,
                                    String dnsDomain )
             throws PDSException, RemoteException;
}