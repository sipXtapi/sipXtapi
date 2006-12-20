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
package org.sipfoundry.sipxconfig.acd;

import java.util.Hashtable;

public interface Provisioning {
    public static final Integer SUCCESS = new Integer(1);

    Hashtable create(Hashtable params);

    Hashtable delete(Hashtable params);

    Hashtable set(Hashtable params);

    Hashtable get(Hashtable params);
}
