/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.util.Arrays;

import org.sipfoundry.sipxconfig.common.SipxCollectionUtils;

public class PhoneBuilder extends SimpleBeanBuilder {
    
    private static final String IGNORE_LIST = "modelId";
    
    public PhoneBuilder() {
        getIgnoreList().addAll(Arrays.asList(SipxCollectionUtils.splitString(IGNORE_LIST)));        
    }

    public void toApi(Object apiObject, Object otherObject) {
        super.toApi(apiObject, otherObject);  
        Phone phone = (Phone) apiObject;
        org.sipfoundry.sipxconfig.phone.Phone otherPhone = (org.sipfoundry.sipxconfig.phone.Phone) otherObject;
        phone.setModelId(otherPhone.getModelId());
    }

    public void fromApi(Object apiObject, Object otherObject) {
        super.fromApi(apiObject, otherObject);
    }    
}
