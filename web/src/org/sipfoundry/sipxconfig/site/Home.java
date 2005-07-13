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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;

public abstract class Home extends BasePage implements PageRenderListener {
    public static final String PAGE = "Home"; 
    
    public void pageBeginRender(PageEvent event_) {
        Visit visit = (Visit) getVisit();
        visit.setNavigationVisible(true);
    }
}
