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
import java.io.IOException;
import java.io.Writer;
import java.util.Arrays;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Map;

import org.apache.commons.beanutils.locale.LocaleConvertUtils;
import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
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
    private String m_exportDatePattern = "EEE, d MMM yyyy HH:mm:ss Z";

    public List<String> getReports() {
        List<String> reports = Arrays.asList(m_factory
                .getBeanNamesForType(AcdHistoricalReport.class));
        return reports;
    }
    
    public void dumpReport(Writer writer, List<Map<String, Object>> reportData, Locale locale) throws IOException {

        // nothing ACD specific here, could be reused
        // not a requirement to use locale, but it was easy and I thought it would be nice touch
        
        if (reportData.size() == 0) {
            return;
        }
        
        // column names
        CsvWriter csv = new CsvWriter(writer);
        Map<String, Object> row0 = reportData.get(0);
        csv.write(row0.keySet().toArray(new String[0]), false);
    
        // rows
        for (Map<String, Object> record : reportData) {
            Object[] recordData = record.values().toArray();
            String[] recordDataStrings = new String[recordData.length];
            for (int i = 0; i < recordData.length; i++) {
                recordDataStrings[i] = LocaleConvertUtils.convert(recordData[i], locale, m_exportDatePattern);
            }
            
            csv.write(recordDataStrings, false);                
        }
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

    /**
     * As defined http://java.sun.com/j2se/1.5.0/docs/api/java/text/SimpleDateFormat.html
     */
    public void setExportDatePattern(String exportDatePattern) {
        m_exportDatePattern = exportDatePattern;
    }
}
