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
package org.sipfoundry.sipxconfig.alias;

/**
 * AliasOwner: represents "ownership" of a set of SIP aliases (including extensions).
 * For example, CallGroupContext owns the SIP aliases belonging to call groups and
 * park orbits.
 */
public interface AliasOwner {
    /**
     * Return true if the alias is in use by some entity managed by this class.
     * For example, CallGroupContext will return true if the alias belongs to a
     * call group or park orbit, ignoring the enabled/disabled state.
     */
    public boolean isAliasInUse(String alias);
}
