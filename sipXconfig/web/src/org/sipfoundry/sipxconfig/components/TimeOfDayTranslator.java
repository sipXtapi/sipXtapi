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

import java.text.Format;
import java.util.Locale;

import org.apache.tapestry.form.translator.FormatTranslator;
import org.apache.tapestry.valid.ValidationConstraint;
import org.apache.tapestry.valid.ValidationStrings;
import org.sipfoundry.sipxconfig.admin.TimeOfDay;
import org.sipfoundry.sipxconfig.admin.TimeOfDay.TimeOfDayFormat;

public class TimeOfDayTranslator extends FormatTranslator {
    protected String defaultPattern() {
        // unused
        return null;
    }

    protected ValidationConstraint getConstraint() {
        return ValidationConstraint.DATE_FORMAT;
    }

    protected Format getFormat(Locale locale) {
        return new TimeOfDay.TimeOfDayFormat(locale);
    }

    protected String getMessageKey() {
        return ValidationStrings.INVALID_DATE;
    }

    protected Object[] getMessageParameters(Locale locale, String label) {
        TimeOfDayFormat format = new TimeOfDay.TimeOfDayFormat(locale);

        return new Object[] {
            label, format.toLocalizedPattern()
        };
    }

}
