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
package org.sipfoundry.sipxconfig.phone;

import java.io.Serializable;
import java.util.List;

/**
 * Aggregation of Phone, Endpoint and Lines objects, Mostly to pull together for display to end
 * user. All data here comes from properties of other objects. You'd never want to save this
 * to the database because it's not normalized. It's abstract so MVC systems can populate data
 * model with subclasses that have additional properies related to viewing system. e.g, is this
 * item selected from interface.
 */
public class PhoneSummary implements Serializable {

    private static final long serialVersionUID = 1L;

    private Phone m_phone;
    
    private List m_lines;
    
    public Phone getPhone() {
        return m_phone;
    }

    public void setPhone(Phone phone) {
        m_phone = phone;
    }

    public Endpoint getEndpoint() {
        return m_phone.getEndpoint();
    }
    
    public int getId() {
        return getEndpoint().getId();
    }

    public boolean hasLines() {
        return getLines() != null && getLines().size() > 0;
    }

    public List getLines() {
        return m_lines;
    }

    public void setLines(List endpointLines) {
        m_lines = endpointLines;
    }
}