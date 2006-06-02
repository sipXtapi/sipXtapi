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

public interface LdapManager {
    LdapConnectionParams getConnectionParams();

    void setConnectionParams(LdapConnectionParams params);

    AttrMap getAttrMap();

    void setAttrMap(AttrMap attrMap);

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
