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
package org.sipfoundry.sipxconfig.components;

import java.text.FieldPosition;
import java.text.Format;
import java.text.MessageFormat;
import java.text.ParsePosition;
import java.util.Arrays;

import org.apache.commons.lang.time.DateUtils;

public class MillisDurationFormat extends Format {

    public static final int DAYS = 0;
    public static final int HOURS = 1;
    public static final int MINUTES = 2;
    public static final int SECONDS = 3;

    private static final String SEPARATOR = ", ";
    private static final long[] INTERVALS = {
        DateUtils.MILLIS_PER_DAY, DateUtils.MILLIS_PER_HOUR, DateUtils.MILLIS_PER_MINUTE,
        DateUtils.MILLIS_PER_SECOND
    };

    // TODO: move to properties file
    private static final String[] LABELS = {
        "{0,choice,0#0 days|1#1 day|1<{0} days}", 
        "{0,choice,0#0 hours|1#1 hour|1<{0} hours}",
        "{0,choice,0#0 minutes|1#1 minute|1<{0} minutes}",
        "{0,choice,0#0 seconds|1#1 second|1<{0} seconds}"
    };

    private int m_maxField = DAYS;
    private String[] m_labels = LABELS;
    private String m_separator = SEPARATOR;
    private boolean m_showZero;

    public void setMaxField(int maxField) {
        m_maxField = maxField;
    }

    public void setUseLabels(boolean useLabels) {
        if (useLabels) {
            m_labels = LABELS;
        } else {
            m_labels = new String[LABELS.length];
            Arrays.fill(m_labels, "{0}");
        }
    }

    public void setSeparator(String separator) {
        m_separator = separator;
    }

    public void setShowZero(boolean showZero) {
        m_showZero = showZero;
    }

    public StringBuffer format(Object obj, StringBuffer toAppendTo, FieldPosition pos) {
        // only works for numbers - class exception otherwise
        Number millisNumber = (Number) obj;
        long millis = millisNumber.longValue();

        if (millis < DateUtils.MILLIS_PER_SECOND && !m_showZero) {
            return toAppendTo;
        }

        boolean bAdded = false;
        for (int i = m_maxField; i < INTERVALS.length; i++) {
            long interval = INTERVALS[i];
            long units = millis / interval;
            if (bAdded) {
                toAppendTo.append(m_separator);
            }
            if (units > 0) {
                millis = millis % interval;
                bAdded = true;
            }
            if (units > 0 || bAdded || i == INTERVALS.length - 1) {
                Object params = new Object[] {
                    new Long(units)
                };
                MessageFormat format = new MessageFormat(m_labels[i]);
                format.format(params, toAppendTo, pos);
                bAdded = true;
            }
        }

        return toAppendTo;
    }

    public Object parseObject(String source, ParsePosition pos) {
        throw new UnsupportedOperationException();
    }
}
