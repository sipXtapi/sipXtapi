/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.core;

/**
 * Database object
 */
public class Alias {

    private String m_userId;

    private String m_alias;

    private int m_aliasOrder;

    public String getUserId() {
        return m_userId;
    }

    public void setUserId(String userId) {
        m_userId = userId;
    }

    public String getAlias() {
        return m_alias;
    }

    public void setAlias(String alias) {
        m_alias = alias;
    }

    public int getAliasOrder() {
        return m_aliasOrder;
    }

    public void setAliasOrder(int aliasOrder) {
        m_aliasOrder = aliasOrder;
    }

}
