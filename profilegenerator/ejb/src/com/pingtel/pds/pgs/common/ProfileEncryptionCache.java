/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/common/ProfileEncryptionCache.java#4 $
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package com.pingtel.pds.pgs.common;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.pgs.phone.Device;
import com.pingtel.pds.pgs.phone.DeviceGroup;
import com.pingtel.pds.pgs.phone.DeviceGroupHome;
import com.pingtel.pds.pgs.profile.ProjectionHelper;
import com.pingtel.pds.pgs.profile.ProjectionHelperHome;
import com.pingtel.pds.pgs.profile.ProjectionInput;
import org.jdom.Element;

import javax.ejb.EJBException;
import java.rmi.RemoteException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;

/**
 * <description of class including its purpose>
 * @author ibutcher
 * 
 */
public class ProfileEncryptionCache {

//////////////////////////////////////////////////////////////////////////
// Constants
////  
      

//////////////////////////////////////////////////////////////////////////
// Attributes
////
    // singleton instance
    private static ProfileEncryptionCache mInstance;

    // attributes
    private HashMap mCacheMap;

    // Session Bean EJBObject references
    private ProjectionHelper mProjectionHelperEJBObject;

    // home references
    private DeviceGroupHome mDeviceGroupHome;


//////////////////////////////////////////////////////////////////////////
// Construction
////
    private ProfileEncryptionCache() {
        mCacheMap = new HashMap();

        try {
            ProjectionHelperHome projectionHelperHome = (ProjectionHelperHome)
                    EJBHomeFactory.getInstance().getHomeInterface(
                            ProjectionHelperHome.class,
                            "ProjectionHelper");

            mProjectionHelperEJBObject = projectionHelperHome.create();

            mDeviceGroupHome = (DeviceGroupHome)
                    EJBHomeFactory.getInstance().getHomeInterface(
                            DeviceGroupHome.class,
                            "DeviceGroup");
        }
        catch (Exception e) {
            throw new EJBException(e.getMessage());
        }
    }
    
    
//////////////////////////////////////////////////////////////////////////
// Public Methods
////
    /**
     * getInstance returns a reference to a ProfileEncryptionCache object
     *
     * @return refence to ProfileEncryptionCache
     */
    public synchronized static ProfileEncryptionCache getInstance() {
        if (mInstance == null) {
            mInstance = new ProfileEncryptionCache();
        }
        return mInstance;
    }
    

//////////////////////////////////////////////////////////////////////////
// Implementation Methods
////
    public boolean shouldEncryptProfiles(Device device){
        boolean returnValue = false;

        try {
            Boolean cachedValue = Boolean.FALSE;

            synchronized (mCacheMap) {
                if(!mCacheMap.containsKey(device.getID())){
                    DeviceGroup deviceGroup =
                            mDeviceGroupHome.findByPrimaryKey(
                                    device.getDeviceGroupID());

                    ArrayList inputs = new ArrayList();
                    inputs.addAll(
                            mProjectionHelperEJBObject.addParentGroupConfigSets(
                                    deviceGroup));

                    ProjectionInput deviceInput =
                            mProjectionHelperEJBObject.getProjectionInput(device);
                    inputs.add(deviceInput);

                    ProjectionInput result =
                            mProjectionHelperEJBObject.project(
                                    "com.pingtel.pds.pgs.plugins.projection.StandardTopDown",
                                    inputs,
                                    device.getDeviceTypeID(),
                                    PDSDefinitions.PROF_TYPE_PHONE);

                    Element root = result.getDocument().getRootElement();

                    Element encryptProfiles = root.getChild("ENCRYPT_PROFILES");
                    if(encryptProfiles != null){
                        Element child =
                                encryptProfiles.getChild("ENCRYPT_PROFILES");
                        String value = child.getText();
                        if(value.equals("ENABLE")){
                            returnValue = true;
                        }
                    }

                    mCacheMap.put(device.getID(), new Boolean(returnValue));
                }

                cachedValue = (Boolean) mCacheMap.get(device.getID());
            } // synchronized

            return cachedValue.booleanValue();
        }
        catch (Exception e) {
            throw new EJBException(e.getMessage());
        }
    }


    public void flushEntry(Device device){
        try {
            Integer deviceID = device.getID();
            synchronized(mCacheMap){
                if(mCacheMap.containsKey(deviceID)){
                    mCacheMap.remove(deviceID);
                }
            }
        }
        catch (RemoteException e) {
            throw new EJBException(e.getMessage());
        }
    }


    public void flushAll() {
        synchronized(mCacheMap){
            mCacheMap.clear();
        }
    }


//////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////    


//////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////

}

