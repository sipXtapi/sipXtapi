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
package org.sipfoundry.sipxconfig.site.phone;

import java.util.List;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.line.EditLine;


/**
 * Comments
 */
public abstract class PhoneLines extends BasePage implements PageRenderListener {

    public static final String PAGE = "PhoneLines";

    public abstract Endpoint getEndpoint();
    
    public abstract void setEndpoint(Endpoint endpoint);
    
    public abstract int getEndpointId();
    
    /** REQUIRED PROPERTY */
    public abstract void setEndpointId(int id);

    public List getLines() {
        return getEndpoint().getLines();
    }
    
    public abstract Line getCurrentRow();

    public abstract void setCurrentRow(Line line);
    
    public abstract SelectMap getSelections();
    
    public abstract void setSelections(SelectMap selections);
    
    public abstract PhoneContext getPhoneContext();

    public void pageBeginRender(PageEvent event_) {
        PhoneContext context = getPhoneContext();
        Endpoint endpoint = context.loadEndpoint(getEndpointId()); 
        setEndpoint(endpoint);
        
        // Generate the list of phone items
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
    }
    
    public void addUser(IRequestCycle cycle) {
        Object[] params = cycle.getServiceParameters();
        Integer endpointId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        AddPhoneUser page = (AddPhoneUser) cycle.getPage(AddPhoneUser.PAGE);
        page.setEndpointId(endpointId.intValue());
        cycle.activate(page);        
    }
    
    public void editLine(IRequestCycle cycle) {
        Object[] params = cycle.getServiceParameters();
        Integer lineId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        EditLine page = (EditLine) cycle.getPage(EditLine.PAGE);
        page.setLineId(lineId.intValue());
        cycle.activate(page);                
    }
    
    public void deleteLine(IRequestCycle cycle_) {
        PhoneContext context = getPhoneContext();
        Endpoint endpoint = getEndpoint();
        Object[] lineIds = getSelections().getAllSelected().toArray();
        DataCollectionUtil.removeByPrimaryKey(endpoint.getLines(), lineIds);
        context.storeEndpoint(endpoint);
    }
    
    public void moveLineUp(IRequestCycle cycle_) {
        moveLines(-1);
    }
    
    public void moveLineDown(IRequestCycle cycle_) {
        moveLines(1);
    }
    
    private void moveLines(int step) {
        PhoneContext context = getPhoneContext();
        Endpoint endpoint = getEndpoint();
        Object[] lineIds = getSelections().getAllSelected().toArray();
        DataCollectionUtil.moveByPrimaryKey(endpoint.getLines(), lineIds, step);        
        context.storeEndpoint(endpoint);
    }

    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);
    }

    public void apply(IRequestCycle cycle_) {
        PhoneContext dao = getPhoneContext();
        dao.storeEndpoint(getEndpoint());
        dao.flush();
    }
    
    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }
}
