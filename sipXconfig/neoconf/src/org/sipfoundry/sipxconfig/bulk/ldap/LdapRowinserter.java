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

import javax.naming.directory.Attributes;

import org.sipfoundry.sipxconfig.bulk.RowInserter;

public class LdapRowinserter extends RowInserter<Attributes> {

    private AttrMap m_attrMap;

    /**
     * Initial implementation will just print all attributes...
     */
    protected String dataToString(Attributes attrs) {
        return attrs.toString();
    }

    protected void insertRow(Attributes attrs) {
        LOG.info(attrs.toString());
    }

    protected boolean checkRowData(Attributes attrs) {
        return (attrs.get("uid") != null);
    }

    public void setAttrMap(AttrMap attrMap) {
        m_attrMap = attrMap;
    }
}
