/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/soap/src/com/pingtel/pds/pgs/soap/SendProfilesService.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.soap;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.phone.DeviceGroup;
import com.pingtel.pds.pgs.phone.DeviceGroupHome;
import com.pingtel.pds.pgs.user.UserAdvocate;
import com.pingtel.pds.pgs.user.UserAdvocateHome;
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserHome;

import javax.naming.NamingException;
import javax.ejb.FinderException;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.rmi.RemoteException;

/**
 * SendProfilesService is an adapter class which contains methods
 * to allow you to get the Config Server to send profiles to
 * the various Users, Devices, User- and Device- Groups.
 */
public class SendProfilesService extends SoapService {

//////////////////////////////////////////////////////////////////////////
// Constants
////
    private static final String ENTITY_TYPE = "entitytype";
    private static final String ENTITY_ID = "entityid";
    private static final String PROFILE_TYPES = "profiletypes";
    private static final String DEVICE_GROUP_ENTITY_TYPE = "devicegroup";
    private static final String ALL_PROFILE_TYPES = "1,2,5";

//////////////////////////////////////////////////////////////////////////
// Attributes
////
    private JMSHelper mJmsHelper;
    private DeviceGroupHome mDeviceGroupHome;
    private UserHome mUserHome;
    private UserAdvocate mUserAdvocateEJBObject;


//////////////////////////////////////////////////////////////////////////
// Construction
////
    public SendProfilesService () {
        mJmsHelper = new JMSHelper();
        try {
            mDeviceGroupHome = (DeviceGroupHome)
                    EJBHomeFactory.getInstance().getHomeInterface(DeviceGroupHome.class, "DeviceGroup");

            mUserHome = (UserHome)
                    EJBHomeFactory.getInstance().getHomeInterface(UserHome.class, "User");

            UserAdvocateHome userAdvocateHome = (UserAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(UserAdvocateHome.class, "UserAdvocate");

            mUserAdvocateEJBObject = userAdvocateHome.create();
        }
        catch (Exception e) {
            throw new RuntimeException(e.getMessage());
        }
    }


//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * sendAllProfiles (re)sends all profiles for all of the Devices in
     * the Config Server.
     *
     * @throws PDSException is thrown for all application level errors.
     */
    public void sendAllProfiles() throws PDSException {
        try {
            Collection allGroups = mDeviceGroupHome.findTopGroups();
            for (Iterator iGroup = allGroups.iterator(); iGroup.hasNext(); ) {
                DeviceGroup deviceGroup = (DeviceGroup) iGroup.next();

                HashMap messageValues = new HashMap();
                messageValues.put(ENTITY_TYPE, DEVICE_GROUP_ENTITY_TYPE );
                messageValues.put(ENTITY_ID, deviceGroup.getID().toString() );
                messageValues.put(PROFILE_TYPES, ALL_PROFILE_TYPES );
                mJmsHelper.sendJMSMessage(mJmsHelper.PROFILES_QUEUE_NAME, messageValues);
            }
        }
        catch (Exception e) {
            throw new PDSException(e.getMessage());
        }
    }


    /**
     * sendProfilesForUser (re)sends all profiles for the given User.
     *
     * @param userId ID of the User whose profiles you want to send.
     * @throws PDSException is thrown for all errors.
     */
    public void sendProfilesForUser(String userId) throws PDSException {
        try {
            Collection c = mUserHome.findByDisplayID(userId);
            User user = (User) c.iterator().next();
            mUserAdvocateEJBObject.generateProfiles(user.getID(), ALL_PROFILE_TYPES, "com.pingtel.pds.pgs.plugins.projection.StandardTopDown");
        }
        catch (Exception e) {
            throw new PDSException(mSoapStrings.getString("E_SEND_USER_PROFILES_GEN"),e);
        }
    }


//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}
