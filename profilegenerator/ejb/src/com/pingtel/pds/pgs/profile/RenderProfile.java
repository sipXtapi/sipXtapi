/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/profile/RenderProfile.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
 
package com.pingtel.pds.pgs.profile;


import javax.ejb.EJBObject;

/**
 *  Session Bean Facade remote interface to write send the profile objects to
 *  the profile writer.
 *
 *@author     ibutcher
 *@created    December 13, 2001
 */
public interface RenderProfile extends EJBObject, RenderProfileBusiness {}
