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

import com.pingtel.pds.common.LocalStrings;

/**
 * Business methods for Device type, Device type represents the physical
 * device down to the model number.
 */
public class DeviceTypeHelper 
{    
    private DeviceType deviceType;
    
    /**
     * @param deviceType EJB object to wrap
     */
    public DeviceTypeHelper(DeviceType deviceType)
    {
        this.deviceType = deviceType;
    }
    
    /**
     * Returns a label consumptionable by end user safe for changing, the model is
     * from database and cannot be altered w/o affecting logic
     * 
     * @param model
     * @param manufacturer
     */
    public static String getLabel(String model, String manufacturer)
    {
        LocalStrings resource = LocalStrings.getLocalStrings(DeviceTypeHelper.class);   
        String dataLabel = manufacturer + " - " + model;                  
        String resKey = "model." + manufacturer + "." + model;
        
        return resource.getStr(resKey, dataLabel);     
    }
}
