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
    private String m_message;
    private Object[] m_params = ArrayUtils.EMPTY_OBJECT_ARRAY;

    public UserException() {
    }

    public UserException(Throwable cause) {
        super(cause);
    }

    public UserException(String message, Object... params) {
        m_message = message;
        m_params = params;
    }

    public String getMessage() {
        if (m_message != null) {
            return MessageFormat.format(m_message, m_params);
        }
        if (getCause() != null) {
            return getCause().getLocalizedMessage();
        }
        return StringUtils.EMPTY;
    }
}
