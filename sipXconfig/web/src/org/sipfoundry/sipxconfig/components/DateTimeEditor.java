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
package org.sipfoundry.sipxconfig.components;

import java.util.Calendar;
import java.util.Date;
import java.util.Locale;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;

public abstract class DateTimeEditor extends BaseComponent {
    private static final int[] TIME_FIELDS = {
        Calendar.HOUR_OF_DAY, Calendar.MINUTE, Calendar.SECOND
    };

    public abstract Date getTime();

    public abstract void setTime(Date time);

    public abstract Date getDatetime();

    public abstract void setDatetime(Date datetime);

    public abstract Date getDate();

    public abstract void setDate(Date date);

    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        Date datetime = getDatetime();
        setDate(new Date(datetime.getTime()));
        setTime(new Date(datetime.getTime()));

        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this) && TapestryUtils.isValid(this)) {
            datetime = toDateTime(getDate(), getTime(), getPage().getLocale());
            setDatetime(datetime);
        }
    }

    public static Date toDateTime(Date date, Date time, Locale locale) {
        Calendar calDate = Calendar.getInstance(locale);
        calDate.setTime(date);

        Calendar calTime = Calendar.getInstance(locale);
        calTime.setTime(time);

        for (int field : TIME_FIELDS) {
            calDate.set(field, calTime.get(field));
        }

        Date datetime2 = calDate.getTime();
        return datetime2;
    }
}
