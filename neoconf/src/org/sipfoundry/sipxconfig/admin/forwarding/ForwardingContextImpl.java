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
 * ForwardingContextImpl
 * 
 * TODO: for testing purposes returns a single call sequence (for all users and for all ids)
 */
public class ForwardingContextImpl implements ForwardingContext {
    private CallSequence m_exampleCallSequence = new CallSequence();

    public CallSequence getCallSequenceForUser(User user_) {
        return m_exampleCallSequence;
    }

    public CallSequence getCallSequence(Integer id_) {
        return m_exampleCallSequence;
    }
}
