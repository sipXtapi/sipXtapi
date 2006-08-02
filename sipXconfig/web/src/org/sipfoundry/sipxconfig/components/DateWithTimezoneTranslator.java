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

import java.text.SimpleDateFormat;
import java.util.Locale;
import java.util.TimeZone;

import org.apache.tapestry.form.translator.DateTranslator;

/**
 * Default date tranlsator doesn't support setting timezones
 */
public class DateWithTimezoneTranslator extends DateTranslator {
    
    private TimeZone m_timeZone = TimeZone.getDefault();
    
    public TimeZone getTimeZone() {
        return m_timeZone;
    }

    public void setTimeZone(TimeZone zone) {
        m_timeZone = zone;
    }

    public SimpleDateFormat getDateFormat(Locale locale) {
        SimpleDateFormat format = super.getDateFormat(locale);
        format.setTimeZone(getTimeZone());
        return format;
    }
}
