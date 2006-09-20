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
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.site.gateway.EditGateway;

/**
 * GatewayTable
 */
public abstract class GatewayTable extends BaseComponent implements PageBeginRenderListener {
    public abstract Gateway getCurrentRow();

    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selected);

    public void pageBeginRender(PageEvent event_) {
        SelectMap selections = getSelections();
        if (null == selections) {
            setSelections(new SelectMap());
        }
    }

    /**
     * When user clicks on link to edit a gateway
     */
    public IPage edit(IRequestCycle cycle, Integer id, Integer ruleId) {
        return EditGateway.getEditPage(cycle, id, getPage(), ruleId);
    }
}
