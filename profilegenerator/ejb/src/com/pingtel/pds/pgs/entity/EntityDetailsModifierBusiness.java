/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/entity/EntityDetailsModifierBusiness.java#4 $
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

import java.util.Collection;
import java.rmi.RemoteException;
import com.pingtel.pds.common.PDSException;

public interface EntityDetailsModifierBusiness {

    /**
     * updateEntityDetails takes the input fields names and values from the
     * details pages (User,Device,UserGroup and DeviceGroup), saves any
     * values to the that entities configuration set and takes any further
     * action needed.   This includes triggering the (re)building of our
     * sipxchange datasets and changing the actual attributes of the entities
     * themselves (changing user's names, extension, etc.)
     *
     * @param formStrings the form fields/inputs from the details pages
     * @throws PDSException for application errors
     * @throws RemoteException for RMI remoting errors
     */
     public void updateEntityDetails ( Collection formStrings )
            throws PDSException, RemoteException;
}