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
public class PhoneSummary implements PrimaryKeySource, Serializable {

    private static final long serialVersionUID = 1L;

    private Endpoint m_endpoint;
    
    /** 
     * transient because can't make assumption about custom code, not to mention peristing
     * this would be very helpful because there really only should be one per context.
     * (BTW: Tapestry pages need to persist PhoneSummaries)
     */
    private transient Phone m_phone;
    
    public void setEndpoint(Endpoint endpoint) {
        m_endpoint = endpoint;
    }

    public Endpoint getEndpoint() {
        return m_endpoint;
    }
    
    public void setPhone(Phone phone) {
        m_phone = phone;
    }
    
    public Phone getPhone() {
        return m_phone;
    }
    
    public int getId() {
        return getEndpoint().getId();
    }

    public boolean hasLines() {
        return getLines() != null && getLines().size() > 0;
    }

    public List getLines() {
        return m_endpoint.getLines();
    }
    
    public Object getPrimaryKey() {
        return new Integer(getId());
    }
}
