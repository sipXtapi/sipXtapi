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


package com.pingtel.pds.pgs.profile;

import java.io.Serializable;

import com.pingtel.pds.common.PDSDefinitions;

public class PDSPropertyMetaData implements Serializable, PDSDefinitions {

    public PDSPropertyMetaData (    String name,
                                    Integer refPropertyID,
                                    Integer aggregatable,
                                    Integer mobility,
                                    Integer isFinal,
                                    Integer readOnly,
                                    int type,
                                    Integer deviceTypeID ) {

        this.name = name;
        this.refPropertyID = refPropertyID;
        this.aggregatable = aggregatable;
        this.mobility = mobility;
        this.isFinal = isFinal;
        this.readOnly = readOnly;
        this.type = type;
        this.deviceTypeID = deviceTypeID;

    }

    public String getName() {
        return this.name;
    }

    public Integer getAggregatable () {
        return this.aggregatable;
    }

    public Integer getMobility () {
        return this.mobility;
    }

    public Integer getFinal () {
        return this.isFinal;
    }

    public int getType () {
        return this.type;
    }

    public Integer getRefPropertyID () {
        return this.refPropertyID;
    }

    public Integer getReadOnly () {
        return this.readOnly;
    }

    public Integer getDeviceTypeID() {
        return this.deviceTypeID;
    }


    private String name;
    private Integer refPropertyID;
    private Integer aggregatable;
    private Integer mobility;
    private Integer isFinal;
    private Integer readOnly;
    private int type;
    private Integer deviceTypeID;

}
