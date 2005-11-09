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
import java.util.Set;

public class PhoneBuilder extends SimpleBeanBuilder {
    
    private static final String MODEL_ID_PROP = "modelId"; 
    
    private static final String[] IGNORE_LIST = { 
        MODEL_ID_PROP 
    };
    
    public PhoneBuilder() {
        getIgnoreList().addAll(Arrays.asList(IGNORE_LIST));        
    }
    
    public void toApiObject(Object apiObject, Object myObject, Set properties) {
        super.toApiObject(apiObject, myObject, properties);
        Phone phone = (Phone) apiObject;
        org.sipfoundry.sipxconfig.phone.Phone otherPhone = (org.sipfoundry.sipxconfig.phone.Phone) myObject;
        if (properties.contains(MODEL_ID_PROP)) {
            phone.setModelId(otherPhone.getModelId());
        }
    }

    public void toMyObject(Object myObject, Object apiObject, Set properties) {
        super.toMyObject(myObject, apiObject, properties);
    }    
}
