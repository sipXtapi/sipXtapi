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

package com.pingtel.pds.pgs.phone;

import java.rmi.RemoteException;
import java.util.Collection;

import javax.ejb.CreateException;
import javax.ejb.EJBHome;
import javax.ejb.FinderException;

/**
 */
public interface CoreSoftwareHome extends EJBHome {
    /**
     *
     * @param description
     * @param version
     * @param name
     *
     * @return
     * @exception CreateException
     * @exception RemoteException
     */
    public CoreSoftware create (    String description,
                                    String version,
                                    String name,
                                    String deviceTypeID )
        throws CreateException, RemoteException;

    /**
     *
     * @param pk
     *
     * @return
     * @exception FinderException
     * @exception RemoteException
     */
    public CoreSoftware findByPrimaryKey ( Integer pk )
        throws FinderException, RemoteException;

    /**
     *
     * @param pk
     *
     * @return
     * @exception FinderException
     * @exception RemoteException
     */
    public Collection findByDeviceType ( Integer phoneType )
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
