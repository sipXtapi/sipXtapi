/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
package com.pingtel.pds.pgs.profile;

import java.io.Serializable;
import java.util.Collection;

import org.jdom.Document;

public class ProjectionInput implements Serializable {
    /** 
     * Default constructor for serialization 
     */
    public ProjectionInput() {
    }

    public ProjectionInput ( Document properties, Collection finalRules ) {
        this.m_properties = properties;
        this.m_finalRules = finalRules;
    }

    public Document getDocument () {
        return this.m_properties;
    }

    public Collection getFinalRules () {
        return this.m_finalRules;
    }

    private Document m_properties;
    private Collection m_finalRules;
}