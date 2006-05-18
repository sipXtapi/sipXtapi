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
}
