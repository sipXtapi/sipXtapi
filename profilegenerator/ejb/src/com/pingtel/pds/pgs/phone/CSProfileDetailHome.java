/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/phone/CSProfileDetailHome.java#4 $
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

public interface CSProfileDetailHome extends EJBHome {

    public CSProfileDetail create ( Integer coreSoftwareDetailsID,
                                    Integer profileType,
                                    String fileName,
                                    String renderClassName,
                                    String xsltURL,
                                    String projectionClassName )
        throws CreateException, RemoteException;

    public CSProfileDetail findByPrimaryKey ( Integer id )
        throws FinderException, RemoteException;

    public Collection findByCoreSoftwareAndProfileType (
                                            Integer coreSoftwareDetailsID,
                                            Integer profileTypeID )
        throws FinderException, RemoteException;

}