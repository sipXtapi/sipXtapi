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

import java.io.Serializable;

/**
 * Support common data object marshalling
 */
public interface DataObjectSource<T> {

    /** Read object from data source by class and object id */
    public T load(Class<T> c, Serializable serializable);
}
