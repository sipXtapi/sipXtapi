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
package org.sipfoundry.sipxconfig.bulk.ldap;

import org.sipfoundry.sipxconfig.admin.CronSchedule;

public interface LdapManager {
    public static final String CONTEXT_BEAN_NAME = "ldapManager";

    LdapConnectionParams getConnectionParams();

    void setConnectionParams(LdapConnectionParams params);

    AttrMap getAttrMap();

    void setAttrMap(AttrMap attrMap);
    
    CronSchedule getSchedule();
    
    void setSchedule(CronSchedule schedule);

    /**
     * Check LDAP connection for the provided connection params
     * 
     * @throws UserException if connection is not possible for some reason
     */
    void verify(LdapConnectionParams params, AttrMap attrMap);
    
    /**
     * Retrieves LDAP schema.
     * 
     * Schema contains list of object classes and their attributes.
     * 
     * @throws UserException if connection is not possible for some reason
     */
    Schema getSchema();
}
