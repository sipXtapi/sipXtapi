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

public class ConfigSetPropertyPermission {

    private int m_refPropId;

    private int m_rcsId;

    private int m_isReadOnly;

    private int m_isFinal;

    public int getRefPropId() {
        return m_refPropId;
    }

    public void setRefPropId(int ref_prop_id) {
        this.m_refPropId = ref_prop_id;
    }

    public int getRcsId() {
        return m_rcsId;
    }

    public void setRcsId(int rcs_id) {
        this.m_rcsId = rcs_id;
    }

    public int getIsReadOnly() {
        return m_isReadOnly;
    }

    public void setIsReadOnly(int is_read_only) {
        this.m_isReadOnly = is_read_only;
    }

    public int getIsFinal() {
        return m_isFinal;
    }

    public void setIsFinal(int is_final) {
        this.m_isFinal = is_final;
    }

}

