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
    public abstract CallSequence getCallSequenceForUser(User user);

    public abstract CallSequence getCallSequence(Integer id);
}