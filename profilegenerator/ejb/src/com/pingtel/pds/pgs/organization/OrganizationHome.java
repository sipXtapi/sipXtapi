/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/organization/OrganizationHome.java#4 $
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
import java.util.Collection;

import javax.ejb.CreateException;
import javax.ejb.EJBHome;
import javax.ejb.FinderException;

public interface OrganizationHome extends EJBHome {

    /**
     *  Description of the Method
     *
     *@param  name                  Description of the Parameter
     *@param  parentOrganizationID  Description of the Parameter
     *@return                       Description of the Return Value
     *@exception  CreateException   Description of the Exception
     *@exception  RemoteException   Description of the Exception
     */
    public Organization create(  String name,
                                Integer parentID,
                                int stereotype,
                                String dnsDomain )
             throws CreateException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  pk                   Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public Organization findByPrimaryKey(Integer pk)
             throws FinderException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  parentOrganizationID  Description of the Parameter
     *@return                       Description of the Return Value
     *@exception  FinderException   Description of the Exception
     *@exception  RemoteException   Description of the Exception
     */
    public Collection findByParentID(Integer parentID)
             throws FinderException, RemoteException;


    /**
     *  Description of the Method
     *
     *@param  name                 Description of the Parameter
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public Collection findByName( String name )
             throws FinderException, RemoteException;


    /**
     *  Description of the Method
     *
     *@return                      Description of the Return Value
     *@exception  FinderException  Description of the Exception
     *@exception  RemoteException  Description of the Exception
     */
    public Collection findAll()
             throws FinderException, RemoteException;

}
