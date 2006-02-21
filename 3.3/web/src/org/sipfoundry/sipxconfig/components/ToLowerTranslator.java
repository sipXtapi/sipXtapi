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

import java.util.Locale;

import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.translator.StringTranslator;

/**
 * Converts an input field to lowercase in tapestry
 */
public class ToLowerTranslator extends StringTranslator {

    protected String formatObject(IFormComponent field, Locale locale, Object object) {
        String s = super.formatObject(field, locale, object);
        return s.toLowerCase(locale);
    }
}
