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

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.bean.EvenOdd;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.job.Job;
import org.sipfoundry.sipxconfig.job.JobContext;

/**
 * Displays current staus of background jobs
 */
public abstract class JobStatusPage extends BasePage {
    public static final Object PAGE = "JobStatusPage";

    @InjectObject(value = "spring:jobContext")
    public abstract JobContext getJobContext();

    @InjectObject(value = "service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    @Bean
    public abstract EvenOdd getRowClass();

    public abstract Job getJob();

    public void remove() {
        getJobContext().removeCompleted();
    }

    public void clear() {
        getJobContext().clear();
    }

    public ITableColumn getStartColumn() {
        return TapestryUtils.createDateColumn("start", getMessages(), getExpressionEvaluator(),
                getPage().getLocale());
    }

    public ITableColumn getStopColumn() {
        return TapestryUtils.createDateColumn("stop", getMessages(), getExpressionEvaluator(),
                getPage().getLocale());
    }
}
