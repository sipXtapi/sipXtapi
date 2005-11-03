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

import java.util.Collection;

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
    
    /**
     * Return objects with the specified alias, or null if there are no such objects.
     * If the database is healthy then there should never be more than one such object.
     * But allow for the possibility that there are conflicts in the database by 
     * returning a Collection, particularly since alias uniqueness across tables is not
     * enforced by database constraints.
     */
    public Collection getObjectsWithAlias(String alias);
}
