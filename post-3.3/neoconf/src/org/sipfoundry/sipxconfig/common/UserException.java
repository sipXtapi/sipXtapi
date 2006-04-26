/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.text.MessageFormat;

import org.apache.commons.lang.ArrayUtils;
import org.apache.commons.lang.StringUtils;

/**
 * Caught in application layer, this informs the user they've done something wrong. Despite being
 * an unchecked exception, this is not meant to be a fatal error and application layer should
 * handle it gracefully.
 */
public class UserException extends RuntimeException {
    private String m_message = StringUtils.EMPTY;
    private Object[] m_params = ArrayUtils.EMPTY_OBJECT_ARRAY;

    public UserException() {
    }

    public UserException(String message) {
        m_message = message;
    }

    public UserException(String message, Object param) {
        m_message = message;
        m_params = new Object[] {
            param
        };
    }

    public UserException(String message, Object param1, Object param2) {
        m_message = message;
        m_params = new Object[] {
            param1, param2
        };
    }

    public String getMessage() {
        return MessageFormat.format(m_message, m_params);
    }
}
