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

import java.util.Collection;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.line.EditLine;


/**
 * Manage a phone's lines
 */
public abstract class PhoneLines extends BasePage implements PageRenderListener {

    public static final String PAGE = "PhoneLines";

    public abstract Phone getPhone();
    
    public abstract void setPhone(Phone phone);
    
    public abstract Integer getPhoneId();
    
    /** REQUIRED PROPERTY */
    public abstract void setPhoneId(Integer id);

    public Collection getLines() {
        return getPhone().getLines();
    }
    
    public abstract Line getCurrentRow();

    public abstract void setCurrentRow(Line line);
    
    public abstract SelectMap getSelections();
    
    public abstract void setSelections(SelectMap selections);
    
    public abstract PhoneContext getPhoneContext();

    public void pageBeginRender(PageEvent event_) {
        PhoneContext context = getPhoneContext();
        Phone phone = context.loadPhone(getPhoneId()); 
        setPhone(phone);
        
        // Generate the list of phone items
        if (getSelections() == null) {
            setSelections(new SelectMap());
        }
    }
    
    public void addUser(IRequestCycle cycle) {
        Object[] params = cycle.getServiceParameters();
        Integer phoneId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        AddPhoneUser page = (AddPhoneUser) cycle.getPage(AddPhoneUser.PAGE);
        page.setPhoneId(phoneId);
        cycle.activate(page);        
    }
    
    public void editLine(IRequestCycle cycle) {
        Object[] params = cycle.getServiceParameters();
        Integer lineId = (Integer) TapestryUtils.assertParameter(Integer.class, params, 0);
        EditLine page = (EditLine) cycle.getPage(EditLine.PAGE);
        page.setLineId(lineId);
        cycle.activate(page);                
    }
    
    public void deleteLine(IRequestCycle cycle_) {
        PhoneContext context = getPhoneContext();
        Phone phone = getPhone();
        Object[] lineIds = getSelections().getAllSelected().toArray();
        DataCollectionUtil.removeByPrimaryKey(phone.getLines(), lineIds);
        
        context.storePhone(phone);
    }
    
    public void moveLineUp(IRequestCycle cycle_) {
        moveLines(-1);
    }
    
    public void moveLineDown(IRequestCycle cycle_) {
        moveLines(1);
    }
    
    private void moveLines(int step) {
        PhoneContext context = getPhoneContext();
        Phone phone = getPhone();
        Object[] lineIds = getSelections().getAllSelected().toArray();
        DataCollectionUtil.moveByPrimaryKey(phone.getLines(), lineIds, step);        
        context.storePhone(phone);
    }

    public void ok(IRequestCycle cycle) {
        apply(cycle);
        cycle.activate(ManagePhones.PAGE);
    }

    public void apply(IRequestCycle cycle_) {
        PhoneContext dao = getPhoneContext();
        dao.storePhone(getPhone());
        dao.flush();
    }
    
    public void cancel(IRequestCycle cycle) {
        cycle.activate(ManagePhones.PAGE);
    }
}
