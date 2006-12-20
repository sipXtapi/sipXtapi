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
package org.sipfoundry.sipxconfig.site.acd;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;

public abstract class AcdStatsPanel extends BaseComponent implements PageBeginRenderListener {
//    public abstract String getAcdQueueUri();
//    public abstract AcdStatistics getAcdStatistics();
//    public abstract String getSelectedAcdQueueUri();
    
    public void pageBeginRender(PageEvent event) {
//        if (event.getRequestCycle().isRewinding()) {
//            AcdStatistics context = getAcdStatistics();
//            String queueUri = getAcdQueueUri();
//            if (queueUri == null) {
//                queueUri = getSelectedAcdQueueUri();
//            }
//            context.setQueueUri(queueUri);        
//        }
    }
}
