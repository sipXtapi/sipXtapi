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

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.job.JobContext;

public abstract class JobStatusPage extends BasePage {
    public abstract JobContext getJobContext();

    public abstract ExpressionEvaluator getExpressionEvaluator();

    public void remove(IRequestCycle cycle_) {
        getJobContext().removeCompleted();
    }

    public void clear(IRequestCycle cycle_) {
        getJobContext().clear();
    }

    public ITableColumn getStartColumn() {
        return TapestryUtils.createDateColumn("start", getMessages(), getExpressionEvaluator());
    }

    public ITableColumn getStopColumn() {
        return TapestryUtils.createDateColumn("stop", getMessages(), getExpressionEvaluator());
    }
}
