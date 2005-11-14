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
package org.sipfoundry.sipxconfig.search;

import java.util.List;

import org.apache.commons.collections.Transformer;

public interface SearchManager {
    String CONTEXT_BEAN_NAME = "searchManager";

    List search(String query, Transformer transformer);

    List search(Class entityClass, String query, Transformer transformer);
}
