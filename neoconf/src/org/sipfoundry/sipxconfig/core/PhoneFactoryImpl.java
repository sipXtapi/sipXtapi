/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.core;

import java.util.HashMap;
import java.util.Map;

import org.sipfoundry.sipxconfig.vendor.CiscoPhone;
import org.sipfoundry.sipxconfig.vendor.SipxPhone;
import org.springframework.beans.factory.xml.XmlBeanFactory;
import org.springframework.core.io.Resource;

/**
 * Comments
 */
public class PhoneFactoryImpl extends XmlBeanFactory 
        implements PhoneFactory {
    
    private Map m_models;

    public PhoneFactoryImpl(Resource phoneFactoryDefinitions) {
        super(phoneFactoryDefinitions);
        
        m_models = new HashMap();

        m_models.put("7960", CiscoPhone.MODEL_7960);
        m_models.put("7940", CiscoPhone.MODEL_7940);
        m_models.put("Cisco", CiscoPhone.MODEL_7960);
        m_models.put("Pingtel", SipxPhone.SOFTPHONE);        
    }
    
    public Phone getPhoneByModel(String model) {
        String map = (String) m_models.get(model);
        return (Phone) getBean(map != null ? map : model);
    }

    public Phone getPhoneByVendor(String vendor) {        
        // intentionally the same as by model
        return getPhoneByModel(vendor);
    }
}
