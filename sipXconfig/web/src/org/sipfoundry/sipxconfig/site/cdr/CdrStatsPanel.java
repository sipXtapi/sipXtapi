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

import java.io.IOException;
import java.io.PrintWriter;
import java.util.Date;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.apache.tapestry.web.WebResponse;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.cdr.CdrManager;
import org.sipfoundry.sipxconfig.cdr.CdrSearch;
import org.sipfoundry.sipxconfig.components.MillisDurationFormat;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class CdrStatsPanel extends BaseComponent {
    private static final Log LOG = LogFactory.getLog(CdrStatsPanel.class);

    @InjectObject(value = "spring:cdrManager")
    public abstract CdrManager getCdrManager();

    @InjectObject(value = "service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    @InjectObject(value = "service:tapestry.globals.WebResponse")
    public abstract WebResponse getResponse();

    @Parameter
    public abstract Date getStartTime();

    @Parameter
    public abstract Date getEndTime();

    @Parameter
    public abstract CdrSearch getCdrSearch();

    @Bean(initializer = "maxField=2, showZero=true")
    public abstract MillisDurationFormat getDurationFormat();

    public abstract Cdr getRow();

    public CdrTableModel getCdrTableModel() {
        CdrTableModel tableModel = new CdrTableModel(getCdrManager());
        tableModel.setFrom(getStartTime());
        tableModel.setTo(getEndTime());
        tableModel.setCdrSearch(getCdrSearch());
        return tableModel;
    }

    public ITableColumn getStartTimeColumn() {
        return TapestryUtils.createDateColumn("startTime", getMessages(),
                getExpressionEvaluator(), getPage().getLocale());
    }

    public void export() {
        try {
            PrintWriter writer = TapestryUtils.getCsvExportWriter(getResponse(), "cdrs.csv");
            getCdrManager().dumpCdrs(writer, getStartTime(), getEndTime(), getCdrSearch());
            writer.close();
        } catch (IOException e) {
            LOG.error("Error during CDR export", e);
        }
    }
}
