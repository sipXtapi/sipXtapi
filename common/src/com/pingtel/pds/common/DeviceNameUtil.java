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

package com.pingtel.pds.common;

import org.sipfoundry.sipxconfig.core.Phone;
import org.sipfoundry.sipxconfig.core.PhoneFactory;
import org.sipfoundry.sipxconfig.core.SipxConfig;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.beans.factory.access.SingletonBeanFactoryLocator;

public class DeviceNameUtil {
    private static DeviceNameUtil g_instance;
    private DeviceNameUtil() {}
    public static DeviceNameUtil getInstance() {
        if (g_instance == null)
            g_instance = new DeviceNameUtil ();
        return g_instance;
    }

    /** Device Specific Profile file offset information stored here */
    public String getDeviceProfileName ( int profileType,
                                         String vendor,
                                         String model,
                                         String macAddress ) {
        
        Phone phone = getPhoneFactory().getPhoneByModel(model);

        // subtract 1, neoconf is zero based
        return phone.getProfileFileName(profileType - 1, macAddress);
    }
    
    private PhoneFactory getPhoneFactory() {
        BeanFactoryLocator bfl = SingletonBeanFactoryLocator.getInstance();
        BeanFactoryReference bf = bfl.useBeanFactory("org.sipfoundry.sipxconfig.core");
        SipxConfig sipx = (SipxConfig) bf.getFactory().getBean("sipxconfig");
        
        return sipx.getPhoneFactory();    
    }

    public String getDeviceProfileToken(int profileType, String vendor) {
        
        Phone phone = getPhoneFactory().getPhoneByVendor(vendor);
        
        // subtract 1, neoconf is zero based
        return phone.getProfileSubscribeToken(profileType - 1);
    }   
}
