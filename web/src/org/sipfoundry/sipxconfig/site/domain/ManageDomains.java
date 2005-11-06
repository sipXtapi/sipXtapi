/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.domain;

import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.domain.Domain;

public abstract class ManageDomains extends BasePage implements PageRenderListener {
    
    private List m_domains;
    
    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);

    public List getDomains() {
        return m_domains; 
    }
    
    public void deleteDomains(IRequestCycle cycle_) {        
    }

    public void editDomain(IRequestCycle cycle_) {        
    }

    public void addDomain(IRequestCycle cycle_) {           
    }
   
    public void pageBeginRender(PageEvent event_) {
        if (m_domains == null) {
            m_domains = new ArrayList();
            Domain d = new Domain();
            d.setName("sipfoundry.org");
            m_domains.add(d);
        }
    
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
    }
}
