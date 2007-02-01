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
package org.sipfoundry.sipxconfig.site.cdr;

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.cdr.Cdr;
import org.sipfoundry.sipxconfig.cdr.CdrManager;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class ActiveCallsPanel extends BaseComponent {
    @InjectObject(value = "spring:cdrManager")
    public abstract CdrManager getCdrManager();

    public abstract List<Cdr> getActiveCalls();

    public abstract void setActiveCalls(List<Cdr> cdrs);

    // FIXME: it would be much better if we can set activeCalls property in prepare for render,
    // but for some reason (Tapestry bug?) prepareForTender gets called to late during Ajax
    // triggered page interactions
    public List<Cdr> getSource() {
        List<Cdr> activeCalls = getActiveCalls();
        if (activeCalls == null) {
            activeCalls = getCdrManager().getActiveCalls();
            setActiveCalls(activeCalls);
        }
        return activeCalls;
    }
}
