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
package org.sipfoundry.sipxconfig.site.admin.commserver;

import org.apache.commons.lang.time.DateUtils;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;

public abstract class Registrations extends BasePage implements PageRenderListener {
    public static final String PAGE = "Registrations";

    private long m_startRenderingTime;

    public abstract RegistrationItem getCurrentRow();

    public void pageBeginRender(PageEvent event_) {
        m_startRenderingTime = System.currentTimeMillis() / DateUtils.MILLIS_PER_SECOND;
    }

    public String getTimeToExpire() {
        RegistrationItem item = getCurrentRow();
        long l = item.timeToExpireAsSeconds(m_startRenderingTime);
        if (l < 0) {
            return getMessage("status.expired");
        }
        return Long.toString(l);
    }
}
