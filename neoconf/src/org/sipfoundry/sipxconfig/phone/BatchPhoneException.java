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
package org.sipfoundry.sipxconfig.phone;


public class BatchPhoneException extends RuntimeException {
    
    private Phone m_phone;
    
    public BatchPhoneException(Phone phone, Throwable cause) {
        super(cause);
        m_phone = phone;
    }
    
    public Phone getPhone() {
        return m_phone;
    }

}
