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

import java.util.List;
import java.util.Map;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ITableColumnModel;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumn;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumnModel;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.acd.stats.AcdHistoricalStats;


public abstract class AcdHistoryPage extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "acd/AcdHistoryPage";
    
    @InjectObject(value = "spring:acdHistoricalStats")
    public abstract AcdHistoricalStats getAcdHistoricalStats();

    public abstract Map<String, Object> getRow();
    
    @Persist(value = "session")
    public abstract String getReportName();

    public abstract void setReportName(String reportName);
    
    public abstract Object getAvailableReportsIndexItem();
    
    public void showReport(String reportName) {
        setReportName(reportName);
    }
    
    public void pageBeginRender(PageEvent event) {
        String report = getReportName();
        if (report == null) {
            report = getAcdHistoricalStats().getReports().get(0);
            setReportName(report);
        }
    }

    public List<Map<String, Object>>getRows() {
        return getAcdHistoricalStats().getReport(getReportName());
    }
    
    public ITableColumnModel getColumns() {
        List<String> names = getAcdHistoricalStats().getReportFields(getReportName());
        ITableColumn[] columns = new ITableColumn[names.size()];
        for (int i = 0; i < columns.length; i++) {
            columns[i] = new MapTableColumn(names.get(i));
        }
        return new SimpleTableColumnModel(columns);
    }
   
}

class MapTableColumn extends SimpleTableColumn {
    public MapTableColumn(String columnName) {
        super(columnName);
    }
    
    public Object getColumnValue(Object objRow) {
        Object value = ((Map<String, Object>) objRow).get(getColumnName());
        return value;
    }    
}
