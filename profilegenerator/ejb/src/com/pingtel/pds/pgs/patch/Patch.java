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

package com.pingtel.pds.pgs.patch;

import java.util.Map;

public interface Patch {

    public static final String NOT_YET_APPLIED = "N";
    public static final String ALREADY_APPLIED = "A";
    public static final String FAILURE = "F";
    public static final String SUCCESS = "S";

    public abstract String update(Integer patchNumber);
    
    public void initialize(Map props);
}
