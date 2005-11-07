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

import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * AliasManager: manages all SIP aliases
 */
public interface AliasManager extends AliasOwner {
    public static final String CONTEXT_BEAN_NAME = "aliasManager";
    
    /**
     * Return true if the bean is allowed to use the specified alias.
     * If there are no existing database objects with that alias, then the
     * bean is allowed to use the alias.
     * If there are existing database objects with that alias, then the bean is
     * only allowed to use the alias if one of those objects is the bean itself.
     * (Ideally there should be at most one database object with a given alias, but
     * it is quite possible for duplication to occur across tables and the
     * system needs to continue to operate smoothly in this situation.)
     * If alias is null, then return true, since null aliases don't cause SIP collisions.
     */
    public boolean canObjectUseAlias(BeanWithId bean, String alias);
    
}
