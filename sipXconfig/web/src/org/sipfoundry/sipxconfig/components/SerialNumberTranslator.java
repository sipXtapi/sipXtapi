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

import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.ValidationMessages;
import org.apache.tapestry.form.translator.StringTranslator;
import org.sipfoundry.sipxconfig.device.DeviceDescriptor;

/**
 * Converts an input field to lowercase in tapestry
 */
public class SerialNumberTranslator extends StringTranslator {
    private DeviceDescriptor m_descriptor;

    public SerialNumberTranslator(DeviceDescriptor descriptor) {
        m_descriptor = descriptor;
    }

    protected Object parseText(IFormComponent field, ValidationMessages messages, String text) {
        return m_descriptor.cleanSerialNumber(text);
    }
}
