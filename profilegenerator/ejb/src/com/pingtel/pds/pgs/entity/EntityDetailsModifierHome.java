/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/entity/EntityDetailsModifierHome.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
package com.pingtel.pds.pgs.entity;

import javax.ejb.*;
import java.rmi.RemoteException;

public interface EntityDetailsModifierHome extends EJBHome {

    public EntityDetailsModifier create () throws CreateException, RemoteException;
}