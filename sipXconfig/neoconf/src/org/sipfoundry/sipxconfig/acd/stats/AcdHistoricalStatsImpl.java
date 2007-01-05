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
package org.sipfoundry.sipxconfig.acd.stats;

import java.io.File;
import java.util.Arrays;
import java.util.Date;
import java.util.List;
import java.util.Map;

import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.jdbc.core.ColumnMapRowMapper;
import org.springframework.jdbc.core.RowMapperResultReader;
import org.springframework.jdbc.core.support.JdbcDaoSupport;
import org.springframework.jdbc.support.rowset.SqlRowSet;
import org.springframework.jdbc.support.rowset.SqlRowSetMetaData;

public class AcdHistoricalStatsImpl extends JdbcDaoSupport implements AcdHistoricalStats,
        BeanFactoryAware {
    private ListableBeanFactory m_factory;
    private String m_reportScript;
    private Boolean m_enabled;    

    public List<String> getReports() {
        List<String> reports = Arrays.asList(m_factory
                .getBeanNamesForType(AcdHistoricalReport.class));
        return reports;
    }
    
    /**
     * Default value is determined is report cron script is installed locally.  You can enable
     * historic stats and configure it to talk to a remote database.
     * 
     * @return 
     */
    public boolean isEnabled() {
        if (m_enabled != null) {
            return m_enabled.booleanValue();
        }
        
        return isReportsInstalledLocally();
    }
    
    public void setEnabled(boolean enabled) {
        m_enabled = new Boolean(enabled);
    }
    
    boolean isReportsInstalledLocally() {
        return getReportScript() != null && new File(getReportScript()).exists();
    }

    public List<String> getReportFields(String reportName) {
        AcdHistoricalReport report = (AcdHistoricalReport) m_factory.getBean(reportName);
        Object[] sqlParameters = new Object[] {
            new Date(0), 
            new Date(0)
        };
        SqlRowSet emptySet = getJdbcTemplate().queryForRowSet(report.getQuery() + " limit 0", sqlParameters);
        SqlRowSetMetaData meta = emptySet.getMetaData();
        List<String> names = Arrays.asList(meta.getColumnNames());
        return names;
    }

    public List<Map<String, Object>> getReport(String reportName, Date startTime, Date endTime) {
        AcdHistoricalReport report = (AcdHistoricalReport) m_factory.getBean(reportName);
        ColumnMapRowMapper columnMapper = new ColumnMapRowMapper();
        RowMapperResultReader rowReader = new RowMapperResultReader(columnMapper);
        Object[] sqlParameters = new Object[] {
            startTime, 
            endTime
        };
        return getJdbcTemplate().query(report.getQuery(), sqlParameters, rowReader);
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_factory = (ListableBeanFactory) beanFactory;
    }

    public void setReportScript(String reportScript) {
        m_reportScript = reportScript;
    }

    public String getReportScript() {
        return m_reportScript;
    }
}
