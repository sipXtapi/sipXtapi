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
package org.sipfoundry.sipxconfig.site.cdr;

import java.util.Calendar;
import java.util.Date;

import org.apache.commons.lang.time.DateUtils;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;

public abstract class CdrPage extends BasePage implements PageBeginRenderListener {
    public abstract Date getStartTime();

    public abstract void setStartTime(Date startTime);

    public abstract Date getEndTime();

    public abstract void setEndTime(Date endTime);

    public void pageBeginRender(PageEvent event_) {
        if (getEndTime() == null) {
            setEndTime(getDefaultEndTime());
        }

        if (getStartTime() == null) {
            // if not set start a day before end time
            Calendar then = Calendar.getInstance();
            then.setTime(getEndTime());
            then.add(Calendar.DAY_OF_MONTH, -1);
            setStartTime(then.getTime());
        }
    }

    /**
     * By default set start at next midnight
     */
    public static Date getDefaultEndTime() {
        Calendar now = Calendar.getInstance();
        now.add(Calendar.DAY_OF_MONTH, 1);
        Calendar end = DateUtils.truncate(now, Calendar.DAY_OF_MONTH);
        return end.getTime();
    }
}
