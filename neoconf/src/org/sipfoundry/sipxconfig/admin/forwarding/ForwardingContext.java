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
package org.sipfoundry.sipxconfig.admin.forwarding;

import org.sipfoundry.sipxconfig.phone.User;

/**
 * ForwardingContext
 */
public interface ForwardingContext {
    public static final String CONTEXT_BEAN_NAME = "forwardingContext";
    
    public CallSequence getCallSequenceForUser(User user);

    public void saveCallSequence(CallSequence callSequence);
    
    public void flush();
}