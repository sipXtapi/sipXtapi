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
package org.sipfoundry.sipxconfig.common;

/**
 * Support common data object marshalling
 */
public interface DataObjectSource {

    /** Read object from data source by class and object id */
    // FIXME: change second parameter to serializable
    public Object load(Class c, Integer serializable);
}
