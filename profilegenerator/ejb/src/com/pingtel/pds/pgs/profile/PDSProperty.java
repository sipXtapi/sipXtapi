/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/ejb/src/com/pingtel/pds/pgs/profile/PDSProperty.java#4 $
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

import java.io.Serializable;

public class PDSProperty implements Serializable {
    
        
    public PDSProperty (    Integer id, 
                            Object value, 
                            Integer configurationSetID, 
                            PDSPropertyMetaData meta ) {
                                
        this.id = id;
        this.value = value;
        this.configurationSetID = configurationSetID;
        this.metaData = meta;
    }
    
    public String getName () { // added for convienience
        return metaData.getName();
    }
    
    public Integer getID () {
        return this.id;
    }
    
    public Object getValue() {
        return this.value;
    }
    
    public Integer getConfigurationSetID () {
        return this.configurationSetID;
    }
    
    public  PDSPropertyMetaData getMetaData() {   
        return this.metaData;
    }
    
    private Integer id;
    private Object value;
    private Integer configurationSetID;
    private PDSPropertyMetaData metaData;
    
}
