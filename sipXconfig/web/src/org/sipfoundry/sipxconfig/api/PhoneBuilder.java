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
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.SipxCollectionUtils;
import org.sipfoundry.sipxconfig.phone.LineSettings;

public class PhoneBuilder extends SimpleBeanBuilder {    
    private static final String MODEL_ID_PROP = "modelId"; 
    private static final String GROUPS_PROP = "groups"; 
    private static final String LINES_PROP = "lines"; 
    
    private static final String[] CUSTOM_FIELDS = { 
        MODEL_ID_PROP, GROUPS_PROP, LINES_PROP
    };
    
    public PhoneBuilder() {
        getCustomFields().addAll(Arrays.asList(CUSTOM_FIELDS));        
    }
    
    public void toApiObject(Object apiObject, Object myObject, Set properties) {
        super.toApiObject(apiObject, myObject, properties);
        Phone phone = (Phone) apiObject;
        org.sipfoundry.sipxconfig.phone.Phone otherPhone = (org.sipfoundry.sipxconfig.phone.Phone) myObject;
        if (properties.contains(MODEL_ID_PROP)) {
            phone.setModelId(otherPhone.getModelId());
        }
        if (properties.contains(GROUPS_PROP)) {
            String[] groups = SipxCollectionUtils.toStringArray(otherPhone.getGroupsAsList());
            phone.setGroups(groups);
        }
        if (properties.contains(LINES_PROP)) {
            List myLines = otherPhone.getLines();
            if (myLines.size() > 0) {
                Line[] apiLines = (Line[]) ApiBeanUtil.newArray(Line.class, myLines.size());            
                for (int i = 0; i < apiLines.length; i++) {
                    org.sipfoundry.sipxconfig.phone.Line myLine = (org.sipfoundry.sipxconfig.phone.Line) myLines.get(i);
                    LineSettings settings = (LineSettings) myLine.getAdapter(LineSettings.class);
                    apiLines[i].setUserId(settings.getUserId());
                    apiLines[i].setUri(myLine.getUri());
                }
                phone.setLines(apiLines);
            }
        }
    }

    public void toMyObject(Object myObject, Object apiObject, Set properties) {
        super.toMyObject(myObject, apiObject, properties);
    }    
}
