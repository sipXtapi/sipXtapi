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
package org.sipfoundry.sipxconfig.site.admin;

import java.text.DateFormat;
import java.util.Date;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.job.Job;
import org.sipfoundry.sipxconfig.job.JobContext;

public abstract class JobStatusPage extends BasePage {
    public static final DateFormat FORMAT = DateFormat.getDateTimeInstance(DateFormat.SHORT,
            DateFormat.SHORT);

    public abstract JobContext getJobContext();

    public abstract Job getCurrentRow();

    public void remove(IRequestCycle cycle_) {
        getJobContext().removeCompleted();
    }

    public void clear(IRequestCycle cycle_) {
        getJobContext().clear();
    }

    public String getCurrentStart() {
        Date start = getCurrentRow().getStart();
        return formatDate(start);
    }

    public String getCurrentStop() {
        Date stop = getCurrentRow().getStop();
        return formatDate(stop);
    }

    private String formatDate(Date date) {
        if (date == null) {
            return StringUtils.EMPTY;
        }
        return FORMAT.format(date);
    }
}
