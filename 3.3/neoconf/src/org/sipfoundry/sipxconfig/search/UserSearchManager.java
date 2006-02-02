/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.search;

import java.util.List;

import org.apache.commons.collections.Transformer;
import org.sipfoundry.sipxconfig.common.User;

public interface UserSearchManager {
    public static final String CONTEXT_BEAN_NAME = "userSearchManager";

    List search(User template, int firstResult, int pageSize, Transformer transformer);
}
