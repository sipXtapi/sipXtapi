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
public class ConfigSetPropertyPermission {

    private int m_refPropId;

    private int m_rcsId;

    private int m_isReadOnly;

    private int m_isFinal;

    public int getRefPropId() {
        return m_refPropId;
    }

    public void setRefPropId(int refPropId) {
        m_refPropId = refPropId;
    }

    public int getRcsId() {
        return m_rcsId;
    }

    public void setRcsId(int rcsId) {
        m_rcsId = rcsId;
    }

    public int getIsReadOnly() {
        return m_isReadOnly;
    }

    public void setIsReadOnly(int isReadOnly) {
        m_isReadOnly = isReadOnly;
    }

    public int getIsFinal() {
        return m_isFinal;
    }

    public void setIsFinal(int isFinal) {
        m_isFinal = isFinal;
    }

}
