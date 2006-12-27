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
package org.sipfoundry.sipxconfig.site.common;

import java.text.DateFormat;
import java.text.Format;
import java.util.Date;
import java.util.Locale;

import org.apache.tapestry.IRender;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.contrib.table.model.ITableModelSource;
import org.apache.tapestry.contrib.table.model.ITableRendererSource;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumn;
import org.apache.tapestry.valid.RenderString;
import org.sipfoundry.sipxconfig.common.SqlInterval;
import org.sipfoundry.sipxconfig.components.MillisDurationFormat;

/**
 * Influence default display behaviour of data.  For more customized results
 * use ColumnValueBlocks.
 */
public class DefaultTableValueRendererSource  implements ITableRendererSource {    
    private static final MillisDurationFormat DURATION = new MillisDurationFormat();
    private ITableRendererSource m_default = SimpleTableColumn.DEFAULT_VALUE_RENDERER_SOURCE;
    static {
        DURATION.setMaxField(2);
    }

    public IRender getRenderer(IRequestCycle objCycle, ITableModelSource objSource, ITableColumn objColumn, 
            Object objRow) {
        Object value = ((SimpleTableColumn) objColumn).getColumnValue(objRow);
        IRender render = getRender(value, objCycle.getPage().getLocale());
        if (render == null) {
            render = m_default.getRenderer(objCycle, objSource, objColumn, objRow);
        }
        
        return render;
    }
    
    protected IRender getRender(Object value, Locale locale) {
        IRender render = null;
        if (value instanceof SqlInterval) {
            long millis = ((SqlInterval) value).getMillisecs();
            String sInterval = DURATION.format(millis);
            render = new RenderString(sInterval);
        } else if (value instanceof Date) {
            Format format = DateFormat.getDateTimeInstance(DateFormat.SHORT, DateFormat.SHORT, locale);
            String sValue = format.format(value);
            render = new RenderString(sValue);
        }
        
        return render;
    }
}


