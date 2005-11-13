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
package org.sipfoundry.sipxconfig.site.search;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;

/**
 * Strategy for locating pages based on object class and id.
 */
public interface EditPageProvider {
    IPage getPage(IRequestCycle cycle, String klass, Object id);
}
